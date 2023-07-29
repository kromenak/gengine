#include "UIDropdown.h"

#include "AssetManager.h"
#include "Texture.h"
#include "UIButton.h"
#include "UIImage.h"
#include "UILabel.h"

UIDropdown::UIDropdown(Actor* parent) : Actor(TransformType::RectTransform)
{
    GetTransform()->SetParent(parent->GetTransform());

    // Create expand button.
    {
        Actor* expandButtonActor = new Actor(TransformType::RectTransform);
        expandButtonActor->GetTransform()->SetParent(GetTransform());

        mExpandButton = expandButtonActor->AddComponent<UIButton>();

        // Position from top-right corner. Nudge 1 pixel to the left for correct look.
        mExpandButton->GetRectTransform()->SetAnchor(1.0f, 1.0f);
        mExpandButton->GetRectTransform()->SetPivot(1.0f, 1.0f);
        mExpandButton->GetRectTransform()->SetAnchoredPosition(-1.0f, 0.0f);

        mExpandButton->SetUpTexture(gAssetManager.LoadTexture("RC_ARW_R.BMP"));
        mExpandButton->SetDownTexture(gAssetManager.LoadTexture("RC_ARW_DWN.BMP"));
        mExpandButton->SetHoverTexture(gAssetManager.LoadTexture("RC_ARW_HI.BMP"));

        mExpandButton->SetPressCallback([this](UIButton* button) {
            OnExpandButtonPressed();
        });
    }

    // Create current choice field.
    {
        Actor* currentChoiceActor = new Actor(TransformType::RectTransform);
        currentChoiceActor->GetTransform()->SetParent(GetTransform());

        mCurrentChoiceLabel = currentChoiceActor->AddComponent<UILabel>();

        // Anchor to fill size of parent, but put pivot to bottom-left.
        // Then, decrease horizontal size by width of expand button. Because pivot is on left, all size decrease occurs on the right!
        mCurrentChoiceLabel->GetRectTransform()->SetAnchorMin(Vector2::Zero);
        mCurrentChoiceLabel->GetRectTransform()->SetAnchorMax(Vector2::One);
        mCurrentChoiceLabel->GetRectTransform()->SetPivot(Vector2::Zero);
        mCurrentChoiceLabel->GetRectTransform()->SetSizeDelta(-mExpandButton->GetRectTransform()->GetRect().GetSize().x, 0.0f);

        // Show text centered in the space.
        // Font choice is not specified in UI spec file, so figured it out via trial and error.
        mCurrentChoiceLabel->SetHorizonalAlignment(HorizontalAlignment::Center);
        mCurrentChoiceLabel->SetVerticalAlignment(VerticalAlignment::Center);
        mCurrentChoiceLabel->SetFont(gAssetManager.LoadFont("F_ARIAL_T8"));
        mCurrentChoiceLabel->SetText("800 x 600");
    }

    // Create downdown expand box.
    {
        Actor* boxActor = new Actor(TransformType::RectTransform);
        boxActor->GetTransform()->SetParent(GetTransform());

        // The expand box is anchored to the bottom edge of the dropdown's rect.
        mBoxRT = boxActor->GetComponent<RectTransform>();
        mBoxRT->SetAnchorMin(Vector2::Zero);
        mBoxRT->SetAnchorMax(Vector2(1.0f, 0.0f));
        mBoxRT->SetPivot(0.0f, 1.0f);
        mBoxRT->SetSizeDelta(0.0f, 50.0f);

        // Put a gray background inside the box.
        UIImage* background = boxActor->AddComponent<UIImage>();
        background->SetColor(Color32::Gray);

        // Create border images for bottom/top/left/right.
        // I'm going to skip the corner images because...they don't seem necessary!
        for(int i = 0; i < 4; ++i)
        {
            Actor* sideActor = new Actor(TransformType::RectTransform);
            sideActor->GetTransform()->SetParent(boxActor->GetTransform());

            UIImage* image = sideActor->AddComponent<UIImage>();

            // Set texture.
            Texture* texture = nullptr;
            if(i < 2)
            {
                texture = gAssetManager.LoadTexture("RC_BOX_TOP.BMP");
            }
            else
            {
                texture = gAssetManager.LoadTexture("RC_BOX_SIDE.BMP");
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

        // Hide box by default.
        boxActor->SetActive(false);
    }
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

void UIDropdown::OnUpdate(float deltaTime)
{
    // Check for changing visible choices based on mouse scroll wheel.
    if(mChoices.size() > mMaxVisibleChoices)
    {
        // See if mouse wheel has scrolled atall.
        Vector2 mouseWheelScrollDelta = gInputManager.GetMouseWheelScrollDelta();
        if(mouseWheelScrollDelta.y != 0)
        {
            // If so, increment or decrement choice offset up to min/max.
            int oldChoiceOffset = mChoicesOffset;
            if(mouseWheelScrollDelta.y > 0 && mChoicesOffset > 0)
            {
                --mChoicesOffset;
            }
            else if(mouseWheelScrollDelta.y < 0 && mChoicesOffset < mChoices.size() - mMaxVisibleChoices)
            {
                ++mChoicesOffset;
            }

            // If offset changed, refresh displayed choices.
            if(mChoicesOffset != oldChoiceOffset)
            {
                RefreshChoicesUI();
            }
        }
    }
}

void UIDropdown::RefreshChoicesUI()
{
    // Hide all existing choices, in case choices were removed.
    for(auto& choiceUI : mChoiceUIs)
    {
        choiceUI.transform->GetOwner()->SetActive(false);
    }

    // Generate dropdown choices.
    for(int i = mChoicesOffset; i < mChoices.size(); ++i)
    {
        // If max visible choices are limited, only spawn up to max items.
        int choiceUIIndex = i - mChoicesOffset;
        if(mMaxVisibleChoices > 0 && choiceUIIndex >= mMaxVisibleChoices)
        {
            break;
        }
        
        // We may need to create a new selection.
        if(choiceUIIndex >= mChoiceUIs.size())
        {
            Actor* buttonActor = new Actor(TransformType::RectTransform);
            buttonActor->GetTransform()->SetParent(mBoxRT);

            // NOTE: Changing the button's texture currently updates the RectTransform's size. So do this before changing RT properties.
            UIButton* button = buttonActor->AddComponent<UIButton>();
            button->SetUpTexture(nullptr, Color32::Gray);
            button->SetHoverTexture(nullptr, Color32(200, 200, 200, 255));
            button->SetDownTexture(nullptr, Color32(200, 200, 200, 255));
            button->SetPressCallback(std::bind(&UIDropdown::OnSelectionPressed, this, std::placeholders::_1));

            // Expand to fill width, anchor to top of box, and set pivot to top-left.
            RectTransform* buttonRT = buttonActor->GetComponent<RectTransform>();
            buttonRT->SetAnchorMin(Vector2(0.0f, 1.0f));
            buttonRT->SetAnchorMax(Vector2::One);
            buttonRT->SetPivot(0.0f, 1.0f);

            // Height of each button is the same as the height of the dropdown itself.
            buttonRT->SetSizeDelta(0.0f, static_cast<RectTransform*>(GetTransform())->GetSize().y);

            // Create center-aligned label.
            UILabel* label = buttonActor->AddComponent<UILabel>();
            label->SetFont(gAssetManager.LoadFont("F_ARIAL_T8"));
            label->SetHorizonalAlignment(HorizontalAlignment::Center);
            label->SetVerticalAlignment(VerticalAlignment::Center);
            
            mChoiceUIs.emplace_back();
            mChoiceUIs.back().transform = buttonRT;
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
    bool isActive = mBoxRT->GetOwner()->IsActive();
    isActive = !isActive;
    mBoxRT->GetOwner()->SetActive(isActive);

    // If active, populate the box with options!
    if(isActive)
    {
        int yPos = 0.0f;
        for(auto& selection : mChoiceUIs)
        {
            selection.transform->SetAnchoredPosition(0.0f, yPos);
            yPos -= selection.transform->GetSize().y;
        }
    }
}

void UIDropdown::OnSelectionPressed(UIButton* button)
{
    // Figure out which selection was pressed.
    if(mCallback != nullptr)
    {
        for(int i = 0; i < mChoiceUIs.size(); ++i)
        {
            if(mChoiceUIs[i].button == button)
            {
                // Update current selection text.
                mCurrentChoiceLabel->SetText(mChoiceUIs[i].label->GetText());

                // Let others know that the selection changed.
                mCallback(mChoicesOffset + i);
            }
        }
    }

    // Hide the dropdown selection box.
    mBoxRT->GetOwner()->SetActive(false);
}