#include "SidneyMenuBar.h"

#include <cassert>

#include "Actor.h"
#include "AssetManager.h"
#include "Random.h"
#include "SidneyButton.h"
#include "UIButton.h"
#include "UIImage.h"
#include "UILabel.h"

void SidneyMenuBar::Init(Actor* parent, const std::string& label, float labelWidth)
{
    // Bar that stretches across entire screen.
    {
        Actor* menuBarActor = new Actor(TransformType::RectTransform);
        menuBarActor->GetTransform()->SetParent(parent->GetTransform());
        mRoot = menuBarActor;

        UIImage* menuBarImage = menuBarActor->AddComponent<UIImage>();
        menuBarImage->SetTexture(gAssetManager.LoadTexture("S_BAR_STRETCH.BMP"), true);
        menuBarImage->SetRenderMode(UIImage::RenderMode::Tiled);

        menuBarImage->GetRectTransform()->SetPivot(1.0f, 1.0f); // Top-Right
        menuBarImage->GetRectTransform()->SetAnchorMin(0.0f, 1.0f); // Anchor to Top, Stretch Horizontally
        menuBarImage->GetRectTransform()->SetAnchorMax(1.0f, 1.0f);
        menuBarImage->GetRectTransform()->SetAnchoredPosition(0.0f, -25.0f);
        menuBarImage->GetRectTransform()->SetSizeDeltaX(0.0f);
    }

    // Bar that extends from top-right, used to give enough height for the screen name label.
    {
        Actor* menuBarTopActor = new Actor(TransformType::RectTransform);
        menuBarTopActor->GetTransform()->SetParent(parent->GetTransform());
        UIImage* menuBarTopImage = menuBarTopActor->AddComponent<UIImage>();

        menuBarTopImage->SetTexture(gAssetManager.LoadTexture("S_BAR_TOPSTRIP_LR.BMP"), true);
        menuBarTopImage->SetRenderMode(UIImage::RenderMode::Tiled);

        menuBarTopImage->GetRectTransform()->SetPivot(1.0f, 1.0f); // Top-Right
        menuBarTopImage->GetRectTransform()->SetAnchor(1.0f, 1.0f); // Anchor to Top-Right
        menuBarTopImage->GetRectTransform()->SetAnchoredPosition(0.0f, -16.0f);
        menuBarTopImage->GetRectTransform()->SetSizeDeltaX(labelWidth);

        // Triangle bit that slopes downward.
        {
            Actor* menuBarAngleActor = new Actor(TransformType::RectTransform);
            menuBarAngleActor->GetTransform()->SetParent(menuBarTopActor->GetTransform());
            UIImage* menuBarAngleImage = menuBarAngleActor->AddComponent<UIImage>();

            menuBarAngleImage->SetTexture(gAssetManager.LoadTexture("S_BAR_TOPANGLE_LR.BMP"), true);

            menuBarAngleImage->GetRectTransform()->SetPivot(1.0f, 1.0f); // Top-Right
            menuBarAngleImage->GetRectTransform()->SetAnchor(0.0f, 1.0f); // Anchor to Top-Left
            menuBarAngleImage->GetRectTransform()->SetAnchoredPosition(0.0f, 0.0f);
        }

        // Screen name label.
        {
            Actor* screenNameActor = new Actor(TransformType::RectTransform);
            screenNameActor->GetTransform()->SetParent(menuBarTopActor->GetTransform());
            UILabel* screenNameLabel = screenNameActor->AddComponent<UILabel>();

            screenNameLabel->SetFont(gAssetManager.LoadFont("SID_EMB_18.FON"));
            screenNameLabel->SetText(label);
            screenNameLabel->SetHorizonalAlignment(HorizontalAlignment::Right);
            screenNameLabel->SetVerticalAlignment(VerticalAlignment::Top);
            screenNameLabel->SetMasked(true);

            screenNameLabel->GetRectTransform()->SetPivot(1.0f, 1.0f); // Top-Right
            screenNameLabel->GetRectTransform()->SetAnchor(1.0f, 1.0f); // Top-Right
            screenNameLabel->GetRectTransform()->SetAnchoredPosition(-4.0f, -1.0f); // Nudge a bit to get right positioning
            screenNameLabel->GetRectTransform()->SetSizeDelta(labelWidth, 18.0f);
        }
    }
}

void SidneyMenuBar::Update()
{
    // Iterate and update each dropdown.
    for(Dropdown& dropdown : mDropdowns)
    {
        // Show the dropdowns options if the root is hovered.
        bool showOptions = dropdown.root->IsHovered();

        // If the root is not hovered, BUT the dropdown options are active
        // and one of THEM is hovered, we want to show the options!
        if(!showOptions && dropdown.options.back()->IsActive())
        {
            for(SidneyButton* option : dropdown.options)
            {
                if(option->GetButton()->IsHovered())
                {
                    showOptions = true;
                    break;
                }
            }
        }

        // Set all options active or inactive.
        for(SidneyButton* option : dropdown.options)
        {
            option->SetActive(showOptions);
        }
    }
}

void SidneyMenuBar::AddDropdown(const std::string& label)
{
    Actor* dropdownActor = new Actor(TransformType::RectTransform);
    dropdownActor->GetTransform()->SetParent(mRoot->GetTransform());

    UILabel* dropdownLabel = dropdownActor->AddComponent<UILabel>();
    dropdownLabel->SetFont(gAssetManager.LoadFont("SID_EMB_10.FON"));
    dropdownLabel->SetText(label);
    dropdownLabel->SetHorizonalAlignment(HorizontalAlignment::Left);
    dropdownLabel->SetVerticalAlignment(VerticalAlignment::Center);
    
    dropdownLabel->GetRectTransform()->SetPivot(0.0f, 0.0f);
    dropdownLabel->GetRectTransform()->SetAnchor(0.0f, 0.0f);
    dropdownLabel->GetRectTransform()->SetAnchoredPosition(mNextMenuItemPos, 1.0f);

    float labelWidth = dropdownLabel->GetTextWidth() + 16.0f;
    dropdownLabel->GetRectTransform()->SetSizeDelta(labelWidth, 13.0f);

    // Add dropdown nib.
    {
        Actor* downArrowActor = new Actor(TransformType::RectTransform);
        downArrowActor->GetTransform()->SetParent(dropdownActor->GetTransform());

        UIImage* downArrowImage = downArrowActor->AddComponent<UIImage>();
        downArrowImage->SetTexture(gAssetManager.LoadTexture("S_DWNARW.BMP"), true);

        downArrowImage->GetRectTransform()->SetPivot(1.0f, 0.0f);
        downArrowImage->GetRectTransform()->SetAnchor(1.0f, 0.0f);
        downArrowImage->GetRectTransform()->SetAnchoredPosition(-6.0f, 3.0f);
        downArrowImage->GetRectTransform()->SetSizeDelta(7.0f, 6.0f);
    }

    mDropdowns.emplace_back();
    mDropdowns.back().root = dropdownActor->AddComponent<UIButton>();
    mNextMenuItemPos += labelWidth + kMenuItemSpacing;
}

void SidneyMenuBar::AddDropdownChoice(const std::string& label, std::function<void()> pressCallback)
{
    assert(!mDropdowns.empty());

    SidneyButton* button = new SidneyButton(mDropdowns.back().root->GetOwner());
    button->SetText(label);
    button->SetTextAlignment(HorizontalAlignment::Left);
    button->SetFont(gAssetManager.LoadFont("SID_PDN_10_L.FON"));
    button->SetWidth(100.0f);
    button->SetPressCallback(pressCallback);

    // This warrants more investigation, but these dropdown buttons *appear* to use different SFX on each run of the game.
    // But the sound seems consistent for the entire duration of the game's run.
    // My best guess is they randomly choose at startup and stay that way?
    int randomSfxIndex = Random::Range(1, 6);
    button->SetPressAudio(gAssetManager.LoadAudio("SIDBUTTON" + std::to_string(randomSfxIndex) + ".WAV"));

    button->GetRectTransform()->SetAnchor(AnchorPreset::TopLeft);
    button->GetRectTransform()->SetAnchoredPosition(0.0f, -13.0f + mDropdowns.back().options.size() * -13.0f);
    mDropdowns.back().options.emplace_back(button);
}