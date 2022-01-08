#include "SaveManager.h"

#include "Config.h"
#include "FileSystem.h"
#include "Paths.h"

SaveManager gSaveManager;

SaveManager::SaveManager()
{
    std::string prefsPath = Paths::GetSaveDataPath("Prefs.ini");

    uint32 bufferSize = 0;
    char* buffer = File::ReadIntoBuffer(prefsPath, bufferSize);
    mPrefs = new Config("Prefs.ini", buffer, bufferSize);

    // Increment run count.
    int runCount = mPrefs->GetInt("App", "Run Count", 0);
    ++runCount;
    mPrefs->Set("App", "Run Count", runCount);

    // Save changes.
    mPrefs->Save(prefsPath);
}

SaveManager::~SaveManager()
{
    SavePrefs();
    delete mPrefs;
}

void SaveManager::Save(const std::string& saveName)
{
    // Save prefs any time the game is saved.
    SavePrefs();
}

void SaveManager::Load(const std::string& saveName)
{

}

void SaveManager::SavePrefs()
{
    mPrefs->Save(Paths::GetSaveDataPath("Prefs.ini"));
}

int SaveManager::GetRunCount() const
{
    return mPrefs->GetInt("App", "Run Count");
}