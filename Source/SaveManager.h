//
// Clark Kromenaker
//
// Handles saving and loading save data and preferences.
//
#pragma once
#include <string>

#include "Config.h" // Including SaveManager.h usually means you also need Config.h

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

// Engine Preferences
#define PREFS_ENGINE "Engine"
    #define PREF_SCREEN_WIDTH "Screen Width"
    #define PREF_SCREEN_HEIGHT "Screen Height"
    #define PREF_FULLSCREEN "Full Screen"
    #define PREF_MONITOR "Monitor"
    #define PREF_WINDOW_X "Start X"
    #define PREF_WINDOW_Y "Start Y"

    #define PREF_CAMERA_GLIDE "Camera Glide"
    #define PREF_CAPTIONS "Captions"

// Hardware Renderer Preferences
#define PREFS_HARDWARE_RENDERER "Engine\\Hardware"
    #define PREFS_MIPMAPS "Mip Mapping"
    #define PREFS_TRILINEAR_FILTERING "Trilinear Filtering"

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