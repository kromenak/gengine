#include "SidneyUtil.h"

#include "Actor.h"
#include "TextAsset.h"
#include "Texture.h"
#include "UIButton.h"
#include "UILabel.h"

UIButton* SidneyUtil::CreateTextButton(Actor* parent, const std::string& text, const std::string& font,
                                       const Vector2& pivotAndAnchor, const Vector2& position, const Vector2& size)
{
    // Create actor as child of parent.
    Actor* actor = new Actor(TransformType::RectTransform);
    actor->GetTransform()->SetParent(parent->GetTransform());

    // Add button to Actor.
    UIButton* button = actor->AddComponent<UIButton>();

    // Position and size appropriately.
    button->GetRectTransform()->SetPivot(pivotAndAnchor);
    button->GetRectTransform()->SetAnchor(pivotAndAnchor);
    button->GetRectTransform()->SetAnchoredPosition(position);
    button->GetRectTransform()->SetSizeDelta(size);

    // Set textures.
    //TODO: Placeholder for now.
    button->SetResizeBasedOnTexture(false);
    button->SetUpTexture(&Texture::Black);

    // Add button text.
    UILabel* buttonLabel = actor->AddComponent<UILabel>();

    // Set font and alignment.
    buttonLabel->SetFont(Services::GetAssets()->LoadFont(font));
    buttonLabel->SetHorizonalAlignment(HorizontalAlignment::Center);
    buttonLabel->SetVerticalAlignment(VerticalAlignment::Bottom);

    // Set text.
    buttonLabel->SetText(text);
    return button;
}