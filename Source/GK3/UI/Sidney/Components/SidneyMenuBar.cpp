#include "SidneyMenuBar.h"

#include <cassert>

#include "Actor.h"
#include "AssetManager.h"
#include "Random.h"
#include "SidneyButton.h"
#include "UIButton.h"
#include "UICanvas.h"
#include "UIImage.h"
#include "UILabel.h"
#include "UIUtil.h"

void SidneyMenuBar::Init(Actor* parent, const std::string& label, float labelWidth)
{
    // Cache frequently used assets.
    mDropdownFont = gAssetManager.LoadFont("SID_EMB_10.FON");
    mDropdownDisabledFont = gAssetManager.LoadFont("SID_NO_EMB_10.FON");

    mDropdownArrowTexture = gAssetManager.LoadTexture("S_DWNARW.BMP");
    mDropdownDisabledArrowTexture = gAssetManager.LoadTexture("S_DWNARW_NOEMB.BMP");

    Actor* menuBarCanvasActor = new Actor("Menu Bar", TransformType::RectTransform);
    menuBarCanvasActor->GetTransform()->SetParent(parent->GetTransform());
    UIUtil::AddCanvas(menuBarCanvasActor, 3);

    // Bar that stretches across entire screen.
    {
        Actor* menuBarActor = new Actor(TransformType::RectTransform);
        menuBarActor->GetTransform()->SetParent(menuBarCanvasActor->GetTransform());
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
        menuBarTopActor->GetTransform()->SetParent(menuBarCanvasActor->GetTransform());
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
        // Assume not showing options to start.
        bool showOptions = false;

        // Only if interactive, consider showing options.
        if(mInteractive)
        {
            // Show the dropdowns options if the root is hovered.
            showOptions = dropdown.enabled && dropdown.rootButton->IsHovered();

            // If the root is not hovered, BUT the dropdown options are active
            // and one of THEM is hovered, we want to show the options!
            if(!showOptions && !dropdown.options.empty() && dropdown.options.back()->IsActive())
            {
                for(SidneyButton* option : dropdown.options)
                {
                    // Hovered OR animating (after a button press).
                    if(option->GetButton()->IsHovered() || option->IsAnimating())
                    {
                        showOptions = true;
                        break;
                    }
                }

                if(!showOptions && dropdown.background->IsHovered())
                {
                    showOptions = true;
                }
            }
        }

        // Set all options active or inactive.
        for(SidneyButton* option : dropdown.options)
        {
            option->SetActive(showOptions);
        }
        dropdown.background->GetOwner()->SetActive(showOptions);
    }
}

void SidneyMenuBar::SetFirstDropdownPosition(float position)
{
    mNextDropdownPosition = position;
}

void SidneyMenuBar::SetDropdownSpacing(float spacing)
{
    mDropdownSpacing = spacing;
}

void SidneyMenuBar::AddDropdown(const std::string& label)
{
    Actor* dropdownActor = new Actor(TransformType::RectTransform);
    dropdownActor->GetTransform()->SetParent(mRoot->GetTransform());

    UILabel* dropdownLabel = dropdownActor->AddComponent<UILabel>();
    dropdownLabel->SetFont(mDropdownFont);
    dropdownLabel->SetText(label);
    dropdownLabel->SetHorizonalAlignment(HorizontalAlignment::Left);
    dropdownLabel->SetVerticalAlignment(VerticalAlignment::Center);

    dropdownLabel->GetRectTransform()->SetAnchor(AnchorPreset::TopLeft);
    dropdownLabel->GetRectTransform()->SetAnchoredPosition(mNextDropdownPosition, 0.0f);

    float labelWidth = dropdownLabel->GetTextWidth() + 16.0f;
    dropdownLabel->GetRectTransform()->SetSizeDelta(labelWidth, 13.0f);

    // Add dropdown nib.
    UIImage* downArrowImage = nullptr;
    {
        Actor* downArrowActor = new Actor(TransformType::RectTransform);
        downArrowActor->GetTransform()->SetParent(dropdownActor->GetTransform());

        downArrowImage = downArrowActor->AddComponent<UIImage>();
        downArrowImage->SetTexture(mDropdownArrowTexture, true);

        downArrowImage->GetRectTransform()->SetPivot(1.0f, 0.0f);
        downArrowImage->GetRectTransform()->SetAnchor(1.0f, 0.0f);
        downArrowImage->GetRectTransform()->SetAnchoredPosition(-6.0f, 3.0f);
        downArrowImage->GetRectTransform()->SetSizeDelta(7.0f, 6.0f);
    }

    // Add dropdown background image.
    UIImage* background = UIUtil::NewUIActorWithWidget<UIImage>(dropdownActor);
    background->SetColor(Color32(66, 65, 66, 255));
    background->GetRectTransform()->SetAnchor(AnchorPreset::TopLeft);
    background->GetRectTransform()->SetSizeDelta(0.0f, 0.0f);
    background->GetRectTransform()->SetAnchoredPosition(1.0f, -13.0f); // the +1 ensures that menu items hang off the background just a bit
    background->GetOwner()->SetActive(false);

    mDropdowns.emplace_back();
    mDropdowns.back().rootButton = dropdownActor->AddComponent<UIButton>();
    mDropdowns.back().rootLabel = dropdownLabel;
    mDropdowns.back().rootArrow = downArrowImage;
    mDropdowns.back().background = background->GetOwner()->AddComponent<UIButton>();
    mNextDropdownPosition += labelWidth + mDropdownSpacing;
}

void SidneyMenuBar::AddDropdownChoice(const std::string& label, const std::function<void()>& pressCallback)
{
    assert(!mDropdowns.empty());
    AddDropdownChoice(mDropdowns.size() - 1, label, pressCallback);
}

void SidneyMenuBar::AddDropdownChoice(size_t dropdownIndex, const std::string& label, const std::function<void()>& pressCallback)
{
    assert(dropdownIndex < mDropdowns.size());
    Dropdown& dropdown = mDropdowns[dropdownIndex];

    // Create button with desired text.
    SidneyButton* button = new SidneyButton(dropdown.rootButton->GetOwner());
    button->SetText(label);
    button->SetTextAlignment(HorizontalAlignment::Left);
    button->SetFont(gAssetManager.LoadFont("SID_PDN_10_L.FON"), gAssetManager.LoadFont("SID_PDN_10_UL.FON"));

    // Figure out desired width/height of this button.
    // It should have a minimum size, but be bigger to fit its text, but also match any bigger item defined previously.
    int labelWidth = Math::TruncateToInt(Math::Max(80.0f, button->GetLabel()->GetTextWidth() + 24.0f));

    // Perhaps due to pixel-perfect math, these buttons don't look correct unless they are even.
    if(labelWidth % 2 != 0)
    {
        ++labelWidth;
    }
    
    for(SidneyButton* option : dropdown.options)
    {
        if(option->GetWidth() > labelWidth)
        {
            labelWidth = option->GetWidth();
        }
    }
    button->SetWidth(labelWidth);
    button->SetHeight(button->GetHeight() + 2); // looks better with a little extra height

    // Set action on press.
    button->SetPressCallback([this, pressCallback](){
        if(pressCallback != nullptr)
        {
            pressCallback();
        }

        // After pressing a dropdown choice, the dropdown should close.
        // Easiest is just make sure all dropdowns are closed at this point.
        for(auto& dropdown : mDropdowns)
        {
            for(auto& choice : dropdown.options)
            {
                choice->SetActive(false);
            }
        }
    });

    // This warrants more investigation, but these dropdown buttons *appear* to use different SFX on each run of the game.
    // But the sound seems consistent for the entire duration of the game's run.
    // My best guess is they randomly choose at startup and stay that way?
    int randomSfxIndex = Random::Range(1, 6);
    button->SetPressAudio(gAssetManager.LoadAudio("SIDBUTTON" + std::to_string(randomSfxIndex) + ".WAV"));

    // Position button in the list.
    button->GetRectTransform()->SetAnchor(AnchorPreset::TopLeft);
    button->GetRectTransform()->SetAnchoredPosition(0.0f, -13.0f + dropdown.nextChoiceYPos);
    dropdown.nextChoiceYPos -= 14.0f;

    // Add to list of options.
    dropdown.options.emplace_back(button);

    // Backtrack and make sure any previously created option is as wide as this one.
    for(auto& option : dropdown.options)
    {
        if(option->GetWidth() < labelWidth)
        {
            option->SetWidth(labelWidth);
        }
    }

    // Resize the dropdown background to encompass all buttons in the dropdown.
    dropdown.background->GetRectTransform()->SetSizeDeltaX(labelWidth - 2);
    dropdown.background->GetRectTransform()->SetSizeDeltaY(Math::Abs(dropdown.nextChoiceYPos + 1));
}

void SidneyMenuBar::AddDropdownChoiceSeparator()
{
    assert(!mDropdowns.empty());
    mDropdowns.back().nextChoiceYPos -= 13.0f;
}

void SidneyMenuBar::SetDropdownEnabled(size_t index, bool enabled)
{
    if(index >= mDropdowns.size()) { return; }

    mDropdowns[index].enabled = enabled;
    if(enabled)
    {
        mDropdowns[index].rootLabel->SetFont(mDropdownFont);
        mDropdowns[index].rootArrow->SetTexture(mDropdownArrowTexture);
    }
    else
    {
        mDropdowns[index].rootLabel->SetFont(mDropdownDisabledFont);
        mDropdowns[index].rootArrow->SetTexture(mDropdownDisabledArrowTexture);
    }
}

void SidneyMenuBar::SetDropdownChoiceEnabled(size_t dropdownIndex, size_t choiceIndex, bool enabled)
{
    if(dropdownIndex >= mDropdowns.size()) { return; }
    if(choiceIndex > mDropdowns[dropdownIndex].options.size()) { return; }
    mDropdowns[dropdownIndex].options[choiceIndex]->GetButton()->SetCanInteract(enabled);
}

void SidneyMenuBar::ClearDropdownChoices(size_t dropdownIndex)
{
    if(dropdownIndex >= mDropdowns.size()) { return; }

    for(SidneyButton* button : mDropdowns[dropdownIndex].options)
    {
        button->PrepareToDestroy();
        button->Destroy();
    }
    mDropdowns[dropdownIndex].options.clear();
}
