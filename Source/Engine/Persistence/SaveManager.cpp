#include "SaveManager.h"

#include "ActionManager.h"
#include "FileSystem.h"
#include "GameProgress.h"
#include "GameTimers.h"
#include "GEngine.h"
#include "GK3UI.h"
#include "InputManager.h"
#include "InventoryManager.h"
#include "LocationManager.h"
#include "Paths.h"
#include "ProgressBar.h"
#include "Renderer.h"
#include "SceneManager.h"
#include "SheepManager.h"
#include "Sidney.h"
#include "StringUtil.h"
#include "SystemUtil.h"
#include "Texture.h"

namespace
{
    struct SortSaves
    {
        bool operator()(const SaveSummary& a, const SaveSummary& b)
        {
            // Saves are sorted with older (earlier) saves at the top. Recent saves are at the bottom.
            // If year isn't equal, the one with the earlier year goes first.
            if(a.saveHeader.year != b.saveHeader.year)
            {
                return a.saveHeader.year < b.saveHeader.year;
            }

            // If year is equal, the one with the earlier month goes first.
            if(a.saveHeader.month != b.saveHeader.month)
            {
                return a.saveHeader.month < b.saveHeader.month;
            }

            // If month is equal, the one with the earlier day goes first.
            if(a.saveHeader.day != b.saveHeader.day)
            {
                return a.saveHeader.day < b.saveHeader.day;
            }

            // If day is equal...keep going, comparing hour/minute/second/millisecond.
            if(a.saveHeader.hour != b.saveHeader.hour)
            {
                return a.saveHeader.hour < b.saveHeader.hour;
            }
            if(a.saveHeader.minute != b.saveHeader.minute)
            {
                return a.saveHeader.minute < b.saveHeader.minute;
            }
            if(a.saveHeader.second != b.saveHeader.second)
            {
                return a.saveHeader.second < b.saveHeader.second;
            }
            if(a.saveHeader.milliseconds != b.saveHeader.milliseconds)
            {
                return a.saveHeader.milliseconds < b.saveHeader.milliseconds;
            }

            // These saves have identical timestamps...sort by filepath I guess.
            return a.filePath.compare(b.filePath) > 0;
        }
    };
}

SaveManager gSaveManager;

SaveManager::SaveManager()
{
    std::string prefsPath = Paths::GetUserDataPath("Prefs.ini");

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
    mPrefs->Save(Paths::GetUserDataPath("Prefs.ini"));
}

int SaveManager::GetRunCount() const
{
    return mPrefs->GetInt("App", "Run Count", 0);
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

void SaveManager::Save(const std::string& saveDescription, int saveIndex, bool quickSave)
{
    mPendingSaveDescription = saveDescription;
    mPendingSaveIndex = saveIndex;
    mPendingUseQuickSave = quickSave;
}

void SaveManager::Load(const std::string& loadPathOrDescription)
{
    // Allow specifying a save to load via user description.
    for(SaveSummary& save : mSaves)
    {
        if(StringUtil::EqualsIgnoreCase(save.persistHeader.userDescription, loadPathOrDescription))
        {
            mPendingLoadPath = save.filePath;
            return;
        }
    }

    // Worst case, assume the passed in string is the path of a save to load.
    mPendingLoadPath = loadPathOrDescription;
}

void SaveManager::HandleQuickSaveQuickLoad()
{
    // As in the original game, not allowed to quick save or quick load during cutscenes.
    // Or when the action bar is showing!
    if(gActionManager.IsActionPlaying() || gActionManager.IsActionBarShowing()) { return; }

    // F5 does a quick save, F6 does a quick load.
    // It shouldn't be possible to do both on one frame - quick save wins out if there's a tie.
    if(gInputManager.IsKeyLeadingEdge(SDL_SCANCODE_F5))
    {
        // To keep things sane, the game blocks quick saving in some circumstances.
        // The original game is a bit more lenient than this, but this is a good start for reasonable behavior - can only quick save when in the 3D scene.
        if(gLayerManager.IsTopLayer("SceneLayer"))
        {
            // See if we've already got a quick save to overwrite, or if this will be a new save.
            int quickSaveIndex = -1;
            for(int i = 0; i < mSaves.size(); ++i)
            {
                if(mSaves[i].isQuickSave)
                {
                    quickSaveIndex = i;
                    break;
                }
            }

            // Save to quick save slot.
            Save("(Quick Save)", quickSaveIndex, true);
        }
    }
    else if(gInputManager.IsKeyLeadingEdge(SDL_SCANCODE_F6))
    {
        // Find quick save index.
        int quickSaveIndex = -1;
        for(int i = 0; i < mSaves.size(); ++i)
        {
            if(mSaves[i].isQuickSave)
            {
                quickSaveIndex = i;
                break;
            }
        }

        // If we have a quick save, then load it!
        if(quickSaveIndex >= 0)
        {
            Load(mSaves[quickSaveIndex].filePath);
        }
    }
}

void SaveManager::HandlePendingSavesAndLoads()
{
    if(!mPendingSaveDescription.empty())
    {
        ProgressBar* progressBar = gGK3UI.ShowSaveProgressBar();
        progressBar->ShowFakeProgress(0.25f);

        SaveInternal(mPendingSaveDescription);
        mPendingSaveDescription.clear();
    }
    if(!mPendingLoadPath.empty())
    {
        // If given a bum path, ignore it.
        if(File::Exists(mPendingLoadPath))
        {
            // A loaded save state doesn't store what screens should/shouldn't be visible.
            // To simplify things, before loading a save state, hide all screens that appear above the game scene.
            gGK3UI.HideAllScreens();

            // Show loading progress bar.
            ProgressBar* progressBar = gGK3UI.ShowLoadProgressBar();
            progressBar->ShowFakeProgress(0.25f);

            // Actually do the load!
            LoadInternal(mPendingLoadPath);
            mPendingLoadPath.clear();
        }
    }
}

void SaveManager::RescanSaveDirectory()
{
    // Clear any old saves, about to repopulate.
    mSaves.clear();

    // Reset next save number, we're about to recalculate that too.
    mNextSaveNumber = 1;

    // Get all files with "gk3" extension in the save data directory.
    std::vector<std::string> saveFileNames = Directory::List(Path::Combine({ Paths::GetUserDataPath(), "Save Games" }), "gk3");
    for(std::string& saveFileName : saveFileNames)
    {
        // Load in the relevant data.
        std::string path = Path::Combine({ Paths::GetUserDataPath(), "Save Games", saveFileName });
        PersistState ps(path.c_str(), PersistFormat::Binary, PersistMode::Load);

        // Read in save header.
        SaveHeader saveHeader;
        saveHeader.OnPersist(ps);

        //TODO: If we detect that this save file is not valid with the current version of the game (based on SaveHeader data), skip it.

        // Create save summary entry, also loading in the persist header data (which contains save description, location, and score).
        mSaves.emplace_back();
        mSaves.back().filePath = path;
        mSaves.back().saveHeader = saveHeader;
        mSaves.back().persistHeader.OnPersist(ps);

        // We do allow saves that don't use the standard naming convention (saveXXXX.gk3).
        // However, only those with the standard naming convention are used to derive the next save number.
        std::string cropped = Path::RemoveExtension(saveFileName);
        if(!cropped.empty())
        {
            cropped = cropped.erase(0, 4);

            //TODO: Technically, this logic should detect and account for "gaps" in save game file names.
            //TODO: For example, if we have save0004.gk3 and save0006.gk3, it should try to use the missing save0005.gk3.
            int num = atoi(cropped.c_str());
            if(num >= mNextSaveNumber)
            {
                mNextSaveNumber = num + 1;
            }
        }

        // Remember if this is the quick save file.
        if(StringUtil::EqualsIgnoreCase(saveFileName, kQuickSaveFileName))
        {
            mSaves.back().isQuickSave = true;
        }
    }

    // Sort saves based on save date/time, putting earlier saves at the top of the list.
    std::sort(mSaves.begin(), mSaves.end(), SortSaves());
}

void SaveManager::SaveInternal(const std::string& saveDescription)
{
    // Save prefs any time the game is saved.
    SavePrefs();

    // Make sure save game folder exists.
    std::string saveFolderPath = Path::Combine({ Paths::GetUserDataPath(), "Save Games" });
    if(!Directory::CreateAll(saveFolderPath))
    {
        printf("Failed to save; could not create \"Save Games\" folder!\n");
        return;
    }

    // Figure out save file name.
    // This is usually sequential, but a specific filename is used for quicksaves.
    std::string fileName = StringUtil::Format("save%04i.gk3", mNextSaveNumber);
    if(mPendingUseQuickSave)
    {
        fileName = kQuickSaveFileName;
    }
    else if(mPendingSaveIndex >= 0 && mPendingSaveIndex < mSaves.size())
    {
        fileName = Path::GetFileName(mSaves[mPendingSaveIndex].filePath);
    }

    // Generate full save file path.
    std::string savePath = Path::Combine({ saveFolderPath, fileName });

    // Create the persistinator.
    PersistState ps(savePath.c_str(), PersistFormat::Binary, PersistMode::Save);

    // Create save header for the save.
    // I don't really see a reason/need to use non-default values for almost everything in there!
    // I'll fill in the save date/time for the hell of it I suppose.
    SaveHeader saveHeader;
    SystemUtil::GetTime(saveHeader.year, saveHeader.month, saveHeader.dayOfWeek, saveHeader.day,
                        saveHeader.hour, saveHeader.minute, saveHeader.second, saveHeader.milliseconds);

    // Write out the save header.
    saveHeader.OnPersist(ps);

    // Create the persist header.
    PersistHeader persistHeader;
    persistHeader.userDescription = saveDescription;
    persistHeader.location = gLocationManager.GetLocation();
    persistHeader.timeblock = gGameProgress.GetTimeblock().ToString();
    persistHeader.score = gGameProgress.GetScore();
    persistHeader.maxScore = gGameProgress.GetMaxScore();

    // Generate a thumbnail for the save game.
    {
        Texture* screenshot = gRenderer.TakeScreenshotToTexture();

        // We want the screenshot to be 160x120. For some resolutions, this is no problem.
        // But for wide resolutions, you end up with some stretching in the image.
        // Figure out what we could crop the width to in order to get roughly a 4:3 aspect ratio.
        uint32_t cropWidth = (screenshot->GetHeight() / 3) * 4;
        uint32_t cropHeight = (screenshot->GetHeight() / 3) * 3;
        screenshot->Crop(cropWidth, cropHeight, true);

        // After cropping, resize to thumbnail size.
        screenshot->Resize(160, 120);
        persistHeader.thumbnailTexture = std::unique_ptr<Texture>(screenshot);
    }

    // Write out the persist header.
    persistHeader.OnPersist(ps);

    // Set the save format version number to save with.
    ps.SetFormatVersionNumber(saveHeader.saveVersion);

    // Persist the ENTIRE game...
    OnPersist(ps);

    // And also persist scene data.
    Scene* scene = gSceneManager.GetScene();
    if(scene != nullptr)
    {
        scene->OnPersist(ps);

        // Save running sheep scripts.
        gSheepManager.OnPersist(ps);
    }
    printf("Saved to file %s.\n", savePath.c_str());

    // Update entry in save list.
    if(mPendingSaveIndex >= 0 && mPendingSaveIndex < mSaves.size())
    {
        mSaves[mPendingSaveIndex].filePath = savePath;
        mSaves[mPendingSaveIndex].saveHeader = std::move(saveHeader);
        mSaves[mPendingSaveIndex].persistHeader = std::move(persistHeader);
        // If you overwrite the quick save slot manually, it is still considered to be "the quick save."
    }
    else
    {
        mSaves.emplace_back();
        mSaves.back().filePath = savePath;
        mSaves.back().saveHeader = std::move(saveHeader);
        mSaves.back().persistHeader = std::move(persistHeader);
        mSaves.back().isQuickSave = mPendingUseQuickSave;

        // Increment save number if this wasn't an overwrite of an existing slot.
        if(!mPendingUseQuickSave)
        {
            ++mNextSaveNumber;
        }
    }

    // Sort saves based on save date/time, putting earlier saves at the top of the list.
    std::sort(mSaves.begin(), mSaves.end(), SortSaves());
}

void SaveManager::LoadInternal(const std::string& loadPath)
{
    mLoadPersistState = new PersistState(loadPath.c_str(), PersistFormat::Binary, PersistMode::Load);

    // Read past save header.
    SaveHeader saveHeader;
    saveHeader.OnPersist(*mLoadPersistState);

    // Store the save version number in the PersistState.
    // This allows load code to detect which version of the save file this is to try to stay compatible.
    mLoadPersistState->SetFormatVersionNumber(saveHeader.saveVersion);

    // Read past persist header.
    PersistHeader persistHeader;
    persistHeader.OnPersist(*mLoadPersistState);

    //TODO: If we can detect that the file is corrupt or not the right type or other nefarious things, early out.

    // Ok, looks like we will actually load this save!
    // Let's unload the current scene to start with a clean slate.
    gSceneManager.UnloadScene([this, loadPath](){

        // Load everything!
        OnPersist(*mLoadPersistState);

        // If this save was made while changing timeblocks, we need to show the timeblock screen instead of going directly to the gameplay scene.
        if(gGameProgress.IsChangingTimeblock())
        {
            gGameProgress.StartTimeblock(gGameProgress.GetTimeblock(), true, nullptr);
            printf("Loaded save file %s.\n", loadPath.c_str());
            delete mLoadPersistState;
        }
        else
        {
            // Load the new scene directly in this case.
            gSceneManager.LoadScene(gLocationManager.GetLocation(), [this, loadPath]() {

                // Restore the scene state using the save data.
                gSceneManager.GetScene()->OnPersist(*mLoadPersistState);

                // Load running sheep scripts.
                // Do this after scene load, since many running sheeps depend on a scene being loaded.
                gSheepManager.OnPersist(*mLoadPersistState);

                // Save versions 1-3 did not store any scene state, so actor positions weren't stored in save data.
                // These older save versions rely on Enter being called to ensure actors are positioned correctly.
                if(mLoadPersistState->GetFormatVersionNumber() < 4)
                {
                    gSceneManager.GetScene()->Enter();
                }

                // Done with persist state - delete it.
                delete mLoadPersistState;
                printf("Loaded save file %s.\n", loadPath.c_str());
            }, false);
        }
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

    // Game timers were added in save version 3 and higher.
    if(ps.GetFormatVersionNumber() >= 3)
    {
        GameTimers::OnPersist(ps);
    }

    // The Sidney UI is somewhat unique in that it actually stores some important state data in there.
    // Maybe its a sign that a "SidneyManager" would make sense? Shrug.
    gGK3UI.GetSidney()->OnPersist(ps);
}