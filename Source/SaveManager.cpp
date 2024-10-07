#include "SaveManager.h"

#include "ActionManager.h"
#include "FileSystem.h"
#include "GameProgress.h"
#include "GEngine.h"
#include "GK3UI.h"
#include "InventoryManager.h"
#include "LocationManager.h"
#include "Paths.h"
#include "SceneManager.h"
#include "Sidney.h"
#include "StringUtil.h"
#include "SystemUtil.h"

SaveManager gSaveManager;

SaveManager::SaveManager()
{
    std::string prefsPath = Paths::GetSaveDataPath("Prefs.ini");

    uint32_t bufferSize = 0;
    uint8_t* buffer = File::ReadIntoBuffer(prefsPath, bufferSize);
    mPrefs = new Config("Prefs.ini", AssetScope::Manual);
    mPrefs->Load(buffer, bufferSize);

    // Increment run count.
    int runCount = mPrefs->GetInt("App", "Run Count", 0);
    ++runCount;
    mPrefs->Set("App", "Run Count", runCount);

    // Save changes.
    mPrefs->Save(prefsPath);

    // Get initial list of saves from disk.
    GetSaves();
}

SaveManager::~SaveManager()
{
    SavePrefs();
    delete mPrefs;
}

void SaveManager::SavePrefs()
{
    mPrefs->Save(Paths::GetSaveDataPath("Prefs.ini"));
}

int SaveManager::GetRunCount() const
{
    return mPrefs->GetInt("App", "Run Count");
}

const std::vector<SaveSummary>& SaveManager::GetSaves()
{
    // If no saves, rescan the save directory to populate the list.
    if(mSaves.empty())
    {
        RescanSaveDirectory();
    }
    return mSaves;
}

void SaveManager::Save(const std::string& saveDescription, int saveIndex)
{
    mPendingSaveDescription = saveDescription;
    mPendingSaveIndex = overwriteSaveIndex;
}

void SaveManager::Load(const std::string& loadPath)
{
    mPendingLoadPath = loadPath;
}

void SaveManager::HandlePendingSavesAndLoads()
{
    if(!mPendingSaveDescription.empty())
    {
        SaveInternal(mPendingSaveDescription);
        mPendingSaveDescription.clear();
    }
    if(!mPendingLoadPath.empty())
    {
        LoadInternal(mPendingLoadPath);
        mPendingLoadPath.clear();
    }
}

void SaveManager::RescanSaveDirectory()
{
    // Clear any old saves, about to repopulate.
    mSaves.clear();

    // Reset next save number, we're about to recalculate that too.
    mNextSaveNumber = 1;

    // Get all files with "gk3" extension in the save data directory.
    std::vector<std::string> saveFileNames = Directory::List(Path::Combine({ Paths::GetSaveDataPath(), "Save Games" }), "gk3");
    for(std::string& saveFileName : saveFileNames)
    {
        // Load in the relevant data.
        std::string path = Path::Combine({ Paths::GetSaveDataPath(), "Save Games", saveFileName });
        PersistState ps(path.c_str(), PersistFormat::Binary, PersistMode::Load);

        // Read in save header.
        SaveHeader saveHeader;
        saveHeader.OnPersist(ps);

        //TODO: If we detect that this save file is not valid with the current version of the game (basd on SaveHeader data), skip it.

        //TODO: Saves are sorted by their date (in the SaveHeader).

        // Create save summary entry, also loading in the persist header data (which contains save description, location, and score).
        mSaves.emplace_back();
        mSaves.back().filePath = path;
        mSaves.back().saveInfo.OnPersist(ps);

        // We do allow saves that don't use the standard naming convention (saveXXXX.gk3).
        // However, only those with the standard naming convention are used to derive the next save number.
        std::string cropped = Path::RemoveExtension(saveFileName);
        if(!cropped.empty())
        {
            cropped = cropped.erase(0, 4);
            int num = atoi(cropped.c_str());
            if(num == mNextSaveNumber)
            {
                ++mNextSaveNumber;
            }
        }
    }
}

void SaveManager::SaveInternal(const std::string& saveDescription)
{
    // Save prefs any time the game is saved.
    SavePrefs();

    // Figure out save number to use.
    int saveNumber = mNextSaveNumber;
    if(mPendingSaveIndex >= 0)
    {
        saveNumber = mPendingSaveIndex + 1;
    }

    // Figure out the path to save to.
    std::string fileName = StringUtil::Format("save%04i.gk3", saveNumber);
    std::string savePath = Path::Combine({ Paths::GetSaveDataPath(), "Save Games", fileName });

    // Create the persistinator.
    PersistState ps(savePath.c_str(), PersistFormat::Binary, PersistMode::Save);

    // Create save header for the save.
    // I don't really see a reason/need to use non-default values for almost everything in there!
    // I'll fill in the save date/time for the hell of it I suppose.
    SaveHeader saveHeader;
    SystemUtil::GetTime(saveHeader.year, saveHeader.month, saveHeader.dayOfWeek, saveHeader.day,
                        saveHeader.hour, saveHeader.minute, saveHeader.second, saveHeader.milliseconds);

    // Save out the save header.
    saveHeader.OnPersist(ps);

    // Create the persist header.
    PersistHeader persistHeader;
    persistHeader.userDescription = saveDescription;
    persistHeader.location = gLocationManager.GetLocation();
    persistHeader.timeblock = gGameProgress.GetTimeblock().ToString();
    persistHeader.score = gGameProgress.GetScore();
    persistHeader.maxScore = gGameProgress.GetMaxScore();
    //TODO: Thumbnail
    persistHeader.OnPersist(ps);

    // Persist the ENTIRE game...
    OnPersist(ps);
    printf("Saved to file %s.\n", savePath.c_str());

    // Update entry in save list.
    if(mPendingSaveIndex >= 0 && mPendingSaveIndex < mSaves.size())
    {
        mSaves[mPendingSaveIndex].filePath = savePath;
        mSaves[mPendingSaveIndex].saveInfo = persistHeader;
    }
    else
    {
        mSaves.emplace_back();
        mSaves.back().filePath = savePath;
        mSaves.back().saveInfo = persistHeader;

        // Increment save number if this wasn't an overwrite of an existing slot.
        ++mNextSaveNumber;
    }
}

void SaveManager::LoadInternal(const std::string& loadPath)
{
    // If given a bum path, ignore it.
    if(!File::Exists(loadPath))
    {
        return;
    }

    //TODO: It might be valuable to create the PersistState *before* unloading the current scene (for verification its a valid save).
    //TODO: To do that, due to scope, we'd have to dynamically allocate though!
    
    // Ok, looks like we will actually load this save!
    // Let's unload the current scene to start with a clean slate.
    gSceneManager.UnloadScene([this, loadPath](){

        // Create the persistinator.
        PersistState ps(loadPath.c_str(), PersistFormat::Binary, PersistMode::Load);

        // Read past save header.
        SaveHeader saveHeader;
        saveHeader.OnPersist(ps);

        // Read past persist header.
        PersistHeader persistHeader;
        persistHeader.OnPersist(ps);

        //TODO: If we can detect that the file is corrupt or not the right type or other nefarious things, early out.

        // Load everything!
        OnPersist(ps);

        // Load the new scene.
        gSceneManager.LoadScene(gLocationManager.GetLocation(), [loadPath](){

            //TODO: Do we want to load any *scene* state (like positions or states of Actors)?
            //TODO: The above solution works pretty well, but Actors will not necessarily be in the same locations or states in the newly loaded scene.
            printf("Loaded save file %s.\n", loadPath.c_str());
        });
    });
}

void SaveManager::OnPersist(PersistState& ps)
{
    // OK, so the original GK3 had quite an impressive generalized save system. Via RTTI, it would save/load almost every class in the game!
    // This is very cool for a variety of reasons (being able to quick save/load at virtually any moment and get the correct load result).
    //
    // HOWEVER, for simplicities sake, I'm going to go with a less complex approach for now:
    // Just save important progress data and use that to reload the scene.
    GEngine::Instance()->OnPersist(ps);
    gLocationManager.OnPersist(ps);
    gInventoryManager.OnPersist(ps);
    gGameProgress.OnPersist(ps);
    gActionManager.OnPersist(ps);

    // The Sidney UI is somewhat unique in that it actually stores some important state data in there.
    // Maybe its a sign that a "SidneyManager" would make sense? Shrug.
    gGK3UI.GetSidney()->OnPersist(ps);
}
