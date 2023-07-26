//
// Clark Kromenaker
//
// UI that shows "closed-captioning", or subtitles, on-screen to match narration or voiceover.
//
#pragma once
#include "Actor.h"

#include <vector>

#include "StringUtil.h"

class Font;
class UIImage;
class UILabel;

class CaptionsOverlay : public Actor
{
public:
    static bool CaptionsEnabled();
    static void SetCaptionsEnabled(bool enabled);

    CaptionsOverlay();

    void AddCaption(const std::string& caption, const std::string& speaker);
    void AdvanceCaption(float delay);

    void HideAll();

protected:
    void OnUpdate(float deltaTime);

private:
    // Are captions enabled? This is an int b/c we need to represent more than 2 values (not loaded, disabled, enabled).
    static int sCaptionsEnabled;
    
    // Mapping from speaker NOUN to font.
    // Used to decide which font to use depending on who is speaking.
    std::string_map_ci<Font*> mSpeakerToFont;

    // A default font to use as a fallback.
    Font* mDefaultFont = nullptr;

    // A caption instance.
    // These are created and reused when a new caption is shown.
    struct Caption
    {
        Actor* actor = nullptr;
        UIImage* backing = nullptr;
        UILabel* label = nullptr;
    };

    // Captions that were created, but are now available for reuse.
    std::vector<Caption> mFreeCaptions;

    // Captions that are actively showing.
    std::vector<Caption> mActiveCaptions;

    // When the game tells us that the oldest caption should be hidden, it can also specify a short delay.
    float mAdvanceTimer = 0.0f;

    void RemoveOldestCaption();
    void RefreshLayout();
};