#include "CaptionsOverlay.h"

#include "Font.h"
#include "IniParser.h"
#include "SaveManager.h"
#include "Scene.h"
#include "TextAsset.h"
#include "Texture.h"
#include "UICanvas.h"
#include "UIImage.h"
#include "UILabel.h"

/*static*/ int CaptionsOverlay::sCaptionsEnabled = -1;

/*static*/ bool CaptionsOverlay::CaptionsEnabled()
{
    if(sCaptionsEnabled == -1)
    {
        bool enableCaptions = gSaveManager.GetPrefs()->GetBool(PREFS_ENGINE, PREF_CAPTIONS, false);
        sCaptionsEnabled = enableCaptions ? 1 : 0;
    }
    return sCaptionsEnabled == 1;
}

/*static*/ void CaptionsOverlay::SetCaptionsEnabled(bool enabled)
{
    sCaptionsEnabled = enabled ? 1 : 0;
    gSaveManager.GetPrefs()->Set(PREFS_ENGINE, PREF_CAPTIONS, enabled);
}

CaptionsOverlay::CaptionsOverlay() : Actor(TransformType::RectTransform)
{
    // Draw order should be equal to the status overlay.
    AddComponent<UICanvas>(8);

    // Canvas takes up entire screen.
    RectTransform* rectTransform = GetComponent<RectTransform>();
    rectTransform->SetSizeDelta(0.0f, 0.0f);
    rectTransform->SetAnchorMin(Vector2::Zero);
    rectTransform->SetAnchorMax(Vector2::One);

    // Load font data.
    TextAsset* fontColors = Services::GetAssets()->LoadText("FONTCOLOR.TXT");
    {
        IniParser parser(fontColors->GetText(), fontColors->GetTextLength());
        parser.ParseAll();

        IniSection section = parser.GetSection("FONTCOLORS");
        for(auto& line : section.lines)
        {
            for(auto& entry : line.entries)
            {
                if(StringUtil::EqualsIgnoreCase(entry.key, "NOTLISTED"))
                {
                    mDefaultFont = Services::GetAssets()->LoadFont("F_" + entry.value);
                }
                else
                {
                    mSpeakerToFont[entry.key] = Services::GetAssets()->LoadFont("F_" + entry.value);
                }
            }
        }
    }
    Services::GetAssets()->UnloadText(fontColors);
}

void CaptionsOverlay::AddCaption(const std::string& captionText, const std::string& speaker)
{
    // Don't do anything if captions are turned off.
    if(!CaptionsEnabled()) { return; }

    Caption caption;
    if(mFreeCaptions.empty())
    {
        // Make a new one.
        caption.actor = new Actor(TransformType::RectTransform);
        caption.actor->GetTransform()->SetParent(GetTransform());

        // Add backing image. This is just a fully opaque black background.
        caption.backing = caption.actor->AddComponent<UIImage>();
        caption.backing->SetTexture(&Texture::Black);

        // Add label. This should fill space horizontally, with horizontal centering, top alignment.
        caption.label = caption.actor->AddComponent<UILabel>();
        caption.label->SetHorizonalAlignment(HorizontalAlignment::Center);
        caption.label->SetHorizontalOverflow(HorizontalOverflow::Wrap);
        caption.label->SetVerticalAlignment(VerticalAlignment::Top);

        // The caption should stretch to fill horizontal screen space.
        // But vertically, it is anchored to the bottom of the screen.
        RectTransform* rt = caption.backing->GetRectTransform();
        rt->SetAnchorMin(Vector2::Zero);
        rt->SetAnchorMax(Vector2(1.0f, 0.0f));
        rt->SetPivot(0.5f, 0.0f); // Pivot at bottom-center.
        rt->SetAnchoredPosition(0.0f, 0.0f);
    }
    else
    {
        // Reuse an existing one.
        caption = mFreeCaptions.back();
        mFreeCaptions.pop_back();
    }

    // Make sure the caption object is active, especially when recycling.
    caption.actor->SetActive(true);

    // Determine which font to use.
    // There's an initial default that's used if no other case matches.
    Font* font = mDefaultFont;

    // If "UNKNOWN" is specified, we assume the speaker is whoever is Ego.
    // Otherwise, grab from the font list.
    std::string_map_ci<Font*>::iterator fontIt;
    if(StringUtil::EqualsIgnoreCase(speaker, "UNKNOWN"))
    {
        fontIt = mSpeakerToFont.find(Scene::GetEgoName());
    }
    else
    {
        fontIt = mSpeakerToFont.find(speaker);
    }
    if(fontIt != mSpeakerToFont.end())
    {
        font = fontIt->second;
    }

    // Set font based on speaker.
    caption.label->SetFont(font);

    // Set caption rect height based on font used and amount of text lines needed.
    caption.backing->GetRectTransform()->SetSizeDeltaY(font->GetGlyphHeight());

    // Set text.
    caption.label->SetText(captionText);

    // Add to active captions.
    mActiveCaptions.push_back(caption);

    // New captions are added to the bottom of the UI.
    // So, each time we add a new caption, we must refresh the layout.
    RefreshLayout();
}

void CaptionsOverlay::AdvanceCaption(float delay)
{
    // Don't do anything if captions are turned off.
    if(!CaptionsEnabled()) { return; }

    // If there was a previous advance delay already in progress, go ahead and finish that one.
    if(mAdvanceTimer > 0.0f)
    {
        RemoveOldestCaption();
        mAdvanceTimer = 0.0f;
    }
    
    // Only bothet if there's actually stuff to remove!
    if(!mActiveCaptions.empty())
    {
        // If there's a delay, just save it.
        // If no delay, remove immediately.
        if(delay > 0.0f)
        {
            mAdvanceTimer = delay;
        }
        else
        {
            RemoveOldestCaption();
        }
    }
}

void CaptionsOverlay::HideAll()
{
    while(!mActiveCaptions.empty())
    {
        // Move all active captions to free.
        mFreeCaptions.push_back(mActiveCaptions.back());
        mActiveCaptions.pop_back();

        // Deactivate the freed caption so it's no longer visible.
        mFreeCaptions.back().actor->SetActive(false);
    }

    // Clear advance timer, since all are hidden.
    mAdvanceTimer = 0.0f;
}

void CaptionsOverlay::OnUpdate(float deltaTime)
{
    if(mAdvanceTimer > 0.0f)
    {
        mAdvanceTimer -= deltaTime;
        if(mAdvanceTimer <= 0.0f)
        {
            RemoveOldestCaption();
        }
    }
}

void CaptionsOverlay::RemoveOldestCaption()
{
    if(!mActiveCaptions.empty())
    {
        // Move caption from active to free list.
        mFreeCaptions.push_back(mActiveCaptions.front());
        mActiveCaptions.erase(mActiveCaptions.begin());

        // Deactivate the freed caption so it's no longer visible.
        mFreeCaptions.back().actor->SetActive(false);

        // Refresh the layout, since it just changed.
        RefreshLayout();
    }
}

void CaptionsOverlay::RefreshLayout()
{
    float yPos = 0.0f;
    for(int i = mActiveCaptions.size() - 1; i >= 0; --i)
    {
        mActiveCaptions[i].backing->GetRectTransform()->SetAnchoredPosition(0.0f, yPos);
        yPos += mActiveCaptions[i].backing->GetRectTransform()->GetRect().GetSize().y;
    }
}