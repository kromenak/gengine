#include "UIDropdown.h"

#include "Texture.h"
#include "UIButton.h"
#include "UICanvas.h"
#include "UIImage.h"
#include "UILabel.h"

UIDropdown::UIDropdown(UICanvas& canvas) : Actor(Actor::TransformType::RectTransform),
    mCanvas(canvas)
{
    // Create expand button.
    {
        Actor* expandButtonActor = new Actor(Actor::TransformType::RectTransform);
        expandButtonActor->GetTransform()->SetParent(GetTransform());

        mExpandButton = expandButtonActor->AddComponent<UIButton>();
        canvas.AddWidget(mExpandButton);

        // Position from top-right corner. Nudge 1 pixel to the left for correct look.
        mExpandButton->GetRectTransform()->SetAnchor(1.0f, 1.0f);
        mExpandButton->GetRectTransform()->SetPivot(1.0f, 1.0f);
        mExpandButton->GetRectTransform()->SetAnchoredPosition(-1.0f, 0.0f);

        mExpandButton->SetUpTexture(Services::GetAssets()->LoadTexture("RC_ARW_R"));
        mExpandButton->SetDownTexture(Services::GetAssets()->LoadTexture("RC_ARW_DWN"));
        mExpandButton->SetHoverTexture(Services::GetAssets()->LoadTexture("RC_ARW_HI"));

        mExpandButton->SetPressCallback(std::bind(&UIDropdown::OnExpandButtonPressed, this));
    }

    // Create current choice field.
    {
        Actor* currentChoiceActor = new Actor(Actor::TransformType::RectTransform);
        currentChoiceActor->GetTransform()->SetParent(GetTransform());

        mCurrentChoiceLabel = currentChoiceActor->AddComponent<UILabel>();
        canvas.AddWidget(mCurrentChoiceLabel);

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
        mCurrentChoiceLabel->SetFont(Services::GetAssets()->LoadFont("F_ARIAL_T8"));
        mCurrentChoiceLabel->SetText("800 x 600");
    }

    // Create downdown expand box.
    {
        Actor* boxActor = new Actor(Actor::TransformType::RectTransform);
        boxActor->GetTransform()->SetParent(GetTransform());

        // The expand box is anchored to the bottom edge of the dropdown's rect.
        mBoxRT = boxActor->GetComponent<RectTransform>();
        mBoxRT->SetAnchorMin(Vector2::Zero);
        mBoxRT->SetAnchorMax(Vector2(1.0f, 0.0f));
        mBoxRT->SetPivot(0.0f, 1.0f);
        mBoxRT->SetSizeDelta(0.0f, 50.0f);

        // Put a gray background inside the box.
        UIImage* background = boxActor->AddComponent<UIImage>();
        canvas.AddWidget(background);
        background->SetColor(Color32::Gray);

        // Create border images for bottom/top/left/right.
        // I'm going to skip the corner images because...they don't seem necessary!
        for(int i = 0; i < 4; ++i)
        {
            Actor* sideActor = new Actor(Actor::TransformType::RectTransform);
            sideActor->GetTransform()->SetParent(boxActor->GetTransform());

            UIImage* image = sideActor->AddComponent<UIImage>();
            canvas.AddWidget(image);

            // Set texture.
            Texture* texture = nullptr;
            if(i < 2)
            {
                texture = Services::GetAssets()->LoadTexture("RC_BOX_TOP");
            }
            else
            {
                texture = Services::GetAssets()->LoadTexture("RC_BOX_SIDE");
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

    // Generate dropdown selections.
    for(int i = 0; i < mChoices.size(); ++i)
    {
        // We may need to create a new selection.
        if(i >= mSelections.size())
        {
            Actor* buttonActor = new Actor(Actor::TransformType::RectTransform);
            buttonActor->GetTransform()->SetParent(mBoxRT);

            // NOTE: Changing the button's texture currently updates the RectTransform's size. So do this before changing RT properties.
            UIButton* button = buttonActor->AddComponent<UIButton>();
            //button->SetUpTexture(&Texture::White);
            //button->SetHoverTexture(&Texture::Black);

            // Expand to fill width, anchor to top of box, and set pivot to top-left.
            RectTransform* buttonRT = buttonActor->GetComponent<RectTransform>();
            buttonRT->SetAnchorMin(Vector2(0.0f, 1.0f));
            buttonRT->SetAnchorMax(Vector2::One);
            buttonRT->SetPivot(0.0f, 1.0f);

            // Height of each button is the same as the height of the dropdown itself.
            buttonRT->SetSizeDelta(0.0f, static_cast<RectTransform*>(GetTransform())->GetSize().y);

            // Create center-aligned label.
            UILabel* label = buttonActor->AddComponent<UILabel>();
            label->SetFont(Services::GetAssets()->LoadFont("F_ARIAL_T8"));
            label->SetHorizonalAlignment(HorizontalAlignment::Center);
            label->SetVerticalAlignment(VerticalAlignment::Center);

            mCanvas.AddWidget(button);
            mCanvas.AddWidget(label);

            mSelections.emplace_back();
            mSelections.back().transform = buttonRT;
            mSelections.back().button = button;
            mSelections.back().label = label;
        }

        mSelections[i].label->SetText(mChoices[i]);
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
        for(auto& selection : mSelections)
        {
            selection.transform->SetAnchoredPosition(0.0f, yPos);
            yPos -= selection.transform->GetSize().y;
        }
    }
}