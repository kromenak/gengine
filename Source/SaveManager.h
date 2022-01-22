//
// Clark Kromenaker
//
// Handles saving and loading save data and preferences.
//
#pragma once
#include <string>

// Sound Preferences
#define PREFS_SOUND "Sound"

#define PREFS_AUDIO_ENABLED "Sound Enabled"
#define PREFS_SFX_ENABLED "SFX Sound Enabled"
#define PREFS_VO_ENABLED "Dialogue Sound Enabled"
#define PREFS_AMBIENT_ENABLED "Ambient Sound Enabled"
#define PREFS_MUSIC_ENABLED "Music Sound Enabled"

#define PREFS_AUDIO_VOLUME "Volume"
#define PREFS_SFX_VOLUME "SFX Volume"
#define PREFS_VO_VOLUME "Dialogue Volume"
#define PREFS_AMBIENT_VOLUME "Ambient Volume"
#define PREFS_MUSIC_VOLUME "Music Volume"

// Hardware Renderer Preferences
#define PREFS_HARDWARE_RENDERER "Engine\\Hardware"

#define PREFS_MIPMAPS "Mip Mapping"
#define PREFS_TRILINEAR_FILTERING "Trilinear Filtering"

class Config;

class SaveManager
{
public:
    SaveManager();
    ~SaveManager();

    void Save(const std::string& saveName);
    void Load(const std::string& saveName);

    Config* GetPrefs() { return mPrefs; }
    void SavePrefs();

    int GetRunCount() const;

private:
    // The player's game preferences.
    // Things like audio settings, graphics settings, etc go here.
    // Basically anything that is not a per-save-game value.
    Config* mPrefs = nullptr;
};

extern SaveManager gSaveManager;