#include "SaveManager.h"

#include "FileSystem.h"
#include "GameProgress.h"
#include "LocationManager.h"
#include "Paths.h"
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

void SaveManager::Save(const std::string& saveDescription)
{
    mPendingSaveDescription = saveDescription;
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

    // Figure out the path to save to.
    std::string fileName = StringUtil::Format("save%04i.gk3", mNextSaveNumber);
    std::string savePath = Path::Combine({ Paths::GetSaveDataPath(), "Save Games", fileName});

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

    //TODO: SAVE ALL THE OTHER DATA...
    printf("Saved to file %s.\n", savePath.c_str());

    // Add to save list.
    mSaves.emplace_back();
    mSaves.back().filePath = savePath;
    mSaves.back().saveInfo = persistHeader;

    // We just saved to this file, so increment save number.
    ++mNextSaveNumber;
}

void SaveManager::LoadInternal(const std::string& loadPath)
{

}
