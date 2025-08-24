//
// Clark Kromenaker
//
// Handles saving and loading save data and preferences.
//
#pragma once
#include <string>
#include <vector>

#include "Config.h" // Including SaveManager.h usually means you also need Config.h
#include "PersistHeader.h"

class PersistState;

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
    #define PREF_CINEMATICS "Cinematics"

// Hardware Renderer Preferences
#define PREFS_HARDWARE_RENDERER "Engine\\Hardware"
    #define PREFS_MIPMAPS "Mip Mapping"
    #define PREFS_TRILINEAR_FILTERING "Trilinear Filtering"

#define PREFS_UI "UI"
    #define PREFS_USE_ORIGINAL_UI_SCALING_LOGIC "Use Original UI Scaling Logic"
    #define PREFS_SCALE_UI_AT_HIGH_RESOLUTIONS "Scale UIs at High Resolutions"
    #define PREFS_SCALE_UI_MINIMUM_HEIGHT "Scale UIs Minimum Resolution Height"
    #define PREFS_PIXEL_PERFECT_UI_SCALING "Pixel Perfect UI Scaling"

struct SaveSummary
{
    bool isQuickSave = false;
    std::string filePath;
    SaveHeader saveHeader;
    PersistHeader persistHeader;
};

class SaveManager
{
public:
    SaveManager();
    ~SaveManager();

    // Prefs
    Config* GetPrefs() { return mPrefs; }
    void SavePrefs();

    int GetRunCount() const;

    // Saves
    const std::vector<SaveSummary>& GetSaves();

    void Save(const std::string& saveDescription, int saveIndex = -1, bool quickSave = false);
    void Load(const std::string& loadPathOrDescription);

    void HandleQuickSaveQuickLoad();
    void HandlePendingSavesAndLoads();

private:
    const char* kQuickSaveFileName = "fastsave.gk3";

    // The player's game preferences.
    // Things like audio settings, graphics settings, etc go here.
    // Basically anything that is not a per-save-game value.
    Config* mPrefs = nullptr;

    // A path we should save the game to when it is safe to do so.
    std::string mPendingSaveDescription;

    // When saving the game, if zero or greater, will overwrite an existing save.
    int mPendingSaveIndex = -1;

    // Similarly, a pending load game path.
    std::string mPendingLoadPath;

    // If true, the pending save will use the quick save slot.
    bool mPendingUseQuickSave = false;

    // A list of saves found on disk, for displaying in save/load screens.
    std::vector<SaveSummary> mSaves;

    // Next number to use when making a save file.
    int mNextSaveNumber = 1;

    // Loading a save game takes place over several seconds (mainly due to scene load being async).
    // So we need to store the persist state until the scene loads and scene state can be restored.
    PersistState* mLoadPersistState = nullptr;

    void RescanSaveDirectory();

    void SaveInternal(const std::string& saveDescription);
    void LoadInternal(const std::string& loadPath);

    void OnPersist(PersistState& ps);
};

extern SaveManager gSaveManager;