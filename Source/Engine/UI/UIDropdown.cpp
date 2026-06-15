#include "UIDropdown.h"

#include "AssetManager.h"
#include "Font.h"
#include "InputManager.h"
#include "Log.h"
#include "Texture.h"
#include "UIButton.h"
#include "UIImage.h"
#include "UILabel.h"
#include "UIScrollbar.h"
#include "UIUtil.h"

UIDropdown::UIDropdown(const std::string& name, Actor* parent) : Actor(name, TransformType::RectTransform)
{
    GetTransform()->SetParent(parent->GetTransform());

    // Create expand button.
    {
        mExpandButton = UI::CreateWidgetActor<UIButton>("ExpandButton", this);

        // Position from top-right corner. Nudge 1 pixel to the left for correct look.
        mExpandButton->GetRectTransform()->SetAnchor(AnchorPreset::TopRight);
        mExpandButton->GetRectTransform()->SetAnchoredPosition(-1.0f, 0.0f);

        mExpandButton->SetUpTexture(gAssetManager.LoadAsset<Texture>("RC_ARW_R.BMP"));
        mExpandButton->SetDownTexture(gAssetManager.LoadAsset<Texture>("RC_ARW_DWN.BMP"));
        mExpandButton->SetHoverTexture(gAssetManager.LoadAsset<Texture>("RC_ARW_HI.BMP"));

        mExpandButton->SetPressCallback([this](UIButton* button){
            OnExpandButtonPressed();
        });
    }

    // Create current choice field.
    {
        mCurrentChoiceLabel = UI::CreateWidgetActor<UILabel>("CurrentChoiceLabel", this);

        // Anchor to fill size of parent, but put pivot to bottom-left.
        // Then, decrease horizontal size by width of expand button. Because pivot is on left, all size decrease occurs on the right!
        mCurrentChoiceLabel->GetRectTransform()->SetAnchor(AnchorPreset::CenterStretch);
        mCurrentChoiceLabel->GetRectTransform()->SetPivot(Vector2::Zero);
        mCurrentChoiceLabel->GetRectTransform()->SetSizeDelta(-mExpandButton->GetRectTransform()->GetRect().GetSize().x, 0.0f);

        // Show text centered in the space.
        // Font choice is not specified in UI spec file, so figured it out via trial and error.
        mCurrentChoiceLabel->SetHorizonalAlignment(HorizontalAlignment::Center);
        mCurrentChoiceLabel->SetVerticalAlignment(VerticalAlignment::Center);
        mCurrentChoiceLabel->SetFont(gAssetManager.LoadAsset<Font>("F_ARIAL_T8"));
        mCurrentChoiceLabel->SetText("");
    }

    // Create downdown choice box. This contains all the choices you can choose for the dropdown.
    {
        // This input blocker blocks the *entire screen* of everything behind the choice box.
        // Order is important here - UI input is processed front to back, so this should come before the actual choices so they can be interacted with.
        mChoiceBoxInputBlocker = UI::CreateWidgetActor<UIButton>("SceneBlocker", this);
        mChoiceBoxInputBlocker->GetRectTransform()->SetSizeDelta(Vector2::Zero);
        mChoiceBoxInputBlocker->SetInputMode(UIWidgetInputMode::ReceivesAllInput);
        mChoiceBoxInputBlocker->SetPressCallback([this](UIButton* button){
            mChoiceBoxInputBlocker->GetOwner()->SetActive(false);
            mChoiceBoxRT->GetOwner()->SetActive(false);
        });

        // Put a gray background inside the box.
        UIImage* background = UI::CreateWidgetActor<UIImage>("Background", this);
        background->SetColor(Color32::Gray);
        background->GetRectTransform()->SetAnchor(AnchorPreset::TopStretch);
        background->GetRectTransform()->SetSizeDelta(0.0f, 25.0f);
        mChoiceBoxRT = background->GetRectTransform();

        // Create border images for bottom/top/left/right.
        // I'm going to skip the corner images because...they don't seem necessary!
        for(int i = 0; i < 4; ++i)
        {
            UIImage* image = UI::CreateWidgetActor<UIImage>("Side", background);

            // Set texture.
            Texture* texture = nullptr;
            if(i < 2)
            {
                texture = gAssetManager.LoadAsset<Texture>("RC_BOX_TOP.BMP");
            }
            else
            {
                texture = gAssetManager.LoadAsset<Texture>("RC_BOX_SIDE.BMP");
            }
            image->SetRenderMode(UIImage::RenderMode::Tiled);
            image->SetTexture(texture);

            // Each side needs different anchoring/sizing properties.
            // Let's say 0 => bottom, 1 => top, 2 => left, 3 => right.
            if(i == 0)
            {
                image->GetRectTransform()->SetAnchorMin(Vector2(0.0f, 0.0f));
                image->GetRectTransform()->SetAnchorMax(Vector2(1.0f, 0.0f));
                image->GetRectTransform()->SetPivot(0.0f, 0.0f);
                image->GetRectTransform()->SetSizeDelta(0.0f, texture->GetHeight());
            }
            else if(i == 1)
            {
                image->GetRectTransform()->SetAnchorMin(Vector2(0.0f, 1.0f));
                image->GetRectTransform()->SetAnchorMax(Vector2(1.0f, 1.0f));
                image->GetRectTransform()->SetPivot(0.0f, 1.0f);
                image->GetRectTransform()->SetSizeDelta(0.0f, texture->GetHeight());
            }
            else if(i == 2)
            {
                image->GetRectTransform()->SetAnchorMin(Vector2::Zero);
                image->GetRectTransform()->SetAnchorMax(Vector2(0.0f, 1.0f));
                image->GetRectTransform()->SetPivot(0.0f, 0.0f);
                image->GetRectTransform()->SetSizeDelta(texture->GetWidth(), 0.0f);
            }
            else
            {
                image->GetRectTransform()->SetAnchorMin(Vector2(1.0f, 0.0f));
                image->GetRectTransform()->SetAnchorMax(Vector2(1.0f, 1.0f));
                image->GetRectTransform()->SetPivot(1.0f, 0.0f);
                image->GetRectTransform()->SetSizeDelta(texture->GetWidth(), 0.0f);
            }
        }

        // Create scrollbar to show if there are too many options.
        {
            Texture* downArrowUp = gAssetManager.LoadAsset<Texture>("RC_ARW_R.BMP");
            Texture* downArrowDown = gAssetManager.LoadAsset<Texture>("RC_ARW_DWN.BMP");

            // GK3 doesn't ship with a great up arrow to use in this context.
            // To HACK around this for now, we'll duplicate the dropdown arrow and flip it!
            Texture* upArrowUp = gAssetManager.LoadAsset<Texture>("RC_ARW_R_FLIP.BMP");
            if(upArrowUp == nullptr)
            {
                upArrowUp = downArrowUp->Duplicate();
                upArrowUp->SetName("RC_ARW_R_FLIP.BMP");
                upArrowUp->FlipVertically();
                gAssetManager.TrackAsset<Texture>(upArrowUp);
            }
            Texture* upArrowDown = gAssetManager.LoadAsset<Texture>("RC_ARW_DWN_FLIP.BMP");
            if(upArrowDown == nullptr)
            {
                upArrowDown = downArrowDown->Duplicate();
                upArrowDown->SetName("RC_ARW_DWN_FLIP.BMP");
                upArrowDown->FlipVertically();
                upArrowDown->FlipHorizontally();
                gAssetManager.TrackAsset<Texture>(upArrowDown);
            }

            // Create a scrollbar inside the choice box.
            UIScrollbarParams scrollbarParams;
            scrollbarParams.decreaseValueButtonUp = upArrowUp;
            scrollbarParams.decreaseValueButtonDown = upArrowDown;
            scrollbarParams.increaseValueButtonUp = downArrowUp;
            scrollbarParams.increaseValueButtonDown = downArrowDown;
            scrollbarParams.scrollbarBackingColor = Color32(123, 121, 123);
            scrollbarParams.handleParams.SetAllBorderColors(Color32(85, 84, 85));
            scrollbarParams.handleParams.centerColor = Color32(65, 64, 65);
            mScrollbar = UI::CreateWidgetActor<UIScrollbar>("Scrollbar", mChoiceBoxRT, scrollbarParams);

            // Hook up scrollbar to change the choice offset.
            mScrollbar->SetDecreaseValueCallback([this](){
                OnScrollbarUpArrowPressed();
            });
            mScrollbar->SetIncreaseValueCallback([this](){
                OnScrollbarDownArrowPressed();
            });
            mScrollbar->SetValueChangeCallback([this](float value){
                OnScrollbarValueChanged(value);
            });
        }

        // Hide choice box by default.
        HideChoiceBox();
    }
}

UIDropdown::UIDropdown(Actor* parent) : UIDropdown("Dropdown", parent)
{

}

void UIDropdown::SetChoices(const std::vector<std::string>& choices)
{
    // Save choices internally.
    mChoices = choices;

    // If scrolled, reset back to top.
    mChoicesOffset = 0;

    // Update UI.
    RefreshChoicesUI();
}

void UIDropdown::SetCurrentChoice(const std::string& choice)
{
    // The current choice can be set separately from the list of choices.
    // Sometimes the current choice is not one of the valid choice selections!
    // Ex: if you set resolution to something in Prefs.ini that isn't usually supported.
    mCurrentChoiceLabel->SetText(choice);
}

void UIDropdown::OnInactive()
{
    Log("Inactive dropdown");
}

void UIDropdown::OnUpdate(float deltaTime)
{
    // Check for changing visible choices based on mouse scroll wheel.
    if(mChoices.size() > mMaxVisibleChoices)
    {
        // See if mouse wheel has scrolled at all.
        Vector2 mouseWheelScrollDelta = gInputManager.GetMouseWheelScrollDelta();
        if(mouseWheelScrollDelta.y != 0)
        {
            // If so, increment or decrement choice offset up to min/max.
            if(mouseWheelScrollDelta.y > 0)
            {
                DecrementChoiceOffset();
            }
            else if(mouseWheelScrollDelta.y < 0)
            {
                IncrementChoiceOffset();
            }
        }
    }
}

void UIDropdown::ShowChoiceBox()
{
    mChoiceBoxInputBlocker->GetOwner()->SetActive(true);
    mChoiceBoxRT->GetOwner()->SetActive(true);
}

void UIDropdown::HideChoiceBox()
{
    mChoiceBoxInputBlocker->GetOwner()->SetActive(false);
    mChoiceBoxRT->GetOwner()->SetActive(false);
}

void UIDropdown::DecrementChoiceOffset()
{
    if(mChoicesOffset > 0)
    {
        --mChoicesOffset;
        RefreshChoicesUI();
        RefreshScrollbarPosition();
    }
}

void UIDropdown::IncrementChoiceOffset()
{
    if(mChoicesOffset < mChoices.size() - mMaxVisibleChoices)
    {
        ++mChoicesOffset;
        RefreshChoicesUI();
        RefreshScrollbarPosition();
    }
}

void UIDropdown::RefreshChoicesUI()
{
    // Hide all existing choices, in case choices were removed.
    for(auto& choiceUI : mChoiceUIs)
    {
        choiceUI.transform->GetOwner()->SetActive(false);
    }

    // Determine whether we will show a scrollbar or not.
    bool showingScrollbar = mMaxVisibleChoices > 0 && mChoices.size() > mMaxVisibleChoices;
    mScrollbar->GetOwner()->SetActive(showingScrollbar);

    // Generate dropdown choices.
    for(int i = mChoicesOffset; i < mChoices.size(); ++i)
    {
        // If max visible choices are limited, only spawn up to max items.
        int choiceUIIndex = i - mChoicesOffset;
        if(mMaxVisibleChoices > 0 && choiceUIIndex >= mMaxVisibleChoices)
        {
            break;
        }

        // We may need to create a new choice UI.
        if(choiceUIIndex >= mChoiceUIs.size())
        {
            // NOTE: Changing the button's texture currently updates the RectTransform's size. So do this before changing RT properties.
            UIButton* button = UI::CreateWidgetActor<UIButton>("Choice" + std::to_string(i), mChoiceBoxRT);
            button->SetUpTexture(nullptr, Color32::Gray);
            button->SetHoverTexture(nullptr, Color32(200, 200, 200, 255));
            button->SetDownTexture(nullptr, Color32(200, 200, 200, 255));
            button->SetPressCallback(std::bind(&UIDropdown::OnSelectionPressed, this, std::placeholders::_1));

            // Expand to fill width, anchor to top of box, and set pivot to top-left.
            button->GetRectTransform()->SetAnchor(AnchorPreset::TopLeft);

            // If showing a scrollbar, make some room for it on the right side.
            float width = mChoiceBoxRT->GetSize().x;
            if(showingScrollbar)
            {
                width -= mScrollbar->GetRectTransform()->GetSize().x;
            }

            // Height of each button is the same as the height of the dropdown itself.
            button->GetRectTransform()->SetSizeDelta(width, static_cast<RectTransform*>(GetTransform())->GetSize().y);

            // Create center-aligned label.
            UILabel* label = button->GetOwner()->AddComponent<UILabel>();
            label->SetFont(gAssetManager.LoadAsset<Font>("F_ARIAL_T8"));
            label->SetHorizonalAlignment(HorizontalAlignment::Center);
            label->SetVerticalAlignment(VerticalAlignment::Center);

            mChoiceUIs.emplace_back();
            mChoiceUIs.back().transform = button->GetRectTransform();
            mChoiceUIs.back().button = button;
            mChoiceUIs.back().label = label;
        }

        // Set active and set text.
        mChoiceUIs[choiceUIIndex].transform->GetOwner()->SetActive(true);
        mChoiceUIs[choiceUIIndex].label->SetText(mChoices[i]);
    }
}

void UIDropdown::OnExpandButtonPressed()
{
    // Toggle box active or inactive.
    bool isActive = mChoiceBoxRT->GetOwner()->IsActive();
    isActive = !isActive;
    if(isActive)
    {
        ShowChoiceBox();

        // Make sure the dropdown box appears below the current choice field.
        mChoiceBoxRT->SetAnchoredPosition(0.0f, -static_cast<RectTransform*>(GetTransform())->GetSize().y);

        // Position each choice one after the other going down.
        float yPos = 0.0f;
        for(auto& choice : mChoiceUIs)
        {
            choice.transform->SetAnchoredPosition(0.0f, yPos);
            yPos -= choice.transform->GetSize().y;
        }

        // Update the containing box to the size of the choices (so the scrollbar is sized right).
        mChoiceBoxRT->SetSizeDeltaY(-yPos);
    }
    else
    {
        HideChoiceBox();
    }
}

void UIDropdown::OnSelectionPressed(UIButton* button)
{
    // Figure out which selection was pressed.
    for(int i = 0; i < mChoiceUIs.size(); ++i)
    {
        if(mChoiceUIs[i].button == button)
        {
            // Update current selection text.
            mCurrentChoiceLabel->SetText(mChoiceUIs[i].label->GetText());

            // Let others know that the selection changed.
            if(mCallback != nullptr)
            {
                mCallback(mChoicesOffset + i);
            }
        }
    }

    // Hide the dropdown choice box.
    HideChoiceBox();
}

void UIDropdown::RefreshScrollbarPosition()
{
    mScrollbar->SetValueSilently(static_cast<float>(mChoicesOffset) / (mChoices.size() - mMaxVisibleChoices));
}

void UIDropdown::OnScrollbarUpArrowPressed()
{
    DecrementChoiceOffset();
}

void UIDropdown::OnScrollbarDownArrowPressed()
{
    IncrementChoiceOffset();
}

void UIDropdown::OnScrollbarValueChanged(float value)
{
    int oldRowOffset = mChoicesOffset;
    int maxOffset = mChoices.size() - mMaxVisibleChoices;
    for(int i = 0; i <= maxOffset; ++i)
    {
        float normalizedValue = static_cast<float>(i) / maxOffset;
        if(value >= normalizedValue)
        {
            // The scrollbar value is more than the value for this threshold, so we'll use this offset.
            mChoicesOffset = i;
        }
        else
        {
            // The scrollbar is less than this threshold, so we don't need to iterate anymore.
            break;
        }
    }

    // If the offset changed due to changing the scrollbar value, refresh the layout.
    if(mChoicesOffset != oldRowOffset)
    {
        RefreshChoicesUI();
    }
}