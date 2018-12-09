//
// ActionBar.cpp
//
// Clark Kromenaker
//
#include "ActionBar.h"

#include "ButtonIconManager.h"
#include "UIButton.h"
#include "UICanvas.h"
#include "UILabel.h"

ActionBar::ActionBar() : Actor(TransformType::RectTransform)
{
	// Create canvas, to contain the UI components.
	mCanvas = AddComponent<UICanvas>();
	
	Actor* labelActor = new Actor(Actor::TransformType::RectTransform);
	labelActor->GetTransform()->SetParent(GetTransform());
	RectTransform* rectTransform = labelActor->GetComponent<RectTransform>();
	rectTransform->SetSize(512, 384);
	
	UILabel* label = labelActor->AddComponent<UILabel>();
	label->SetFont(Services::GetAssets()->LoadFont("F_RYE"));
	label->SetText("Test Label");
	mCanvas->AddWidget(label);
	
	Actor* buttonActor = new Actor(Actor::TransformType::RectTransform);
	buttonActor->GetTransform()->SetParent(GetTransform());
	
	UIButton* button = buttonActor->AddComponent<UIButton>();
	//button->SetUpTexture(Services::GetAssets()->LoadTexture("TITLE_PLAY_U"));
	//button->SetDownTexture(Services::GetAssets()->LoadTexture("TITLE_PLAY_D"));
	//button->SetHoverTexture(Services::GetAssets()->LoadTexture("TITLE_PLAY_H"));
	//button->SetDisabledTexture(Services::GetAssets()->LoadTexture("TITLE_PLAY_X"));
	button->SetPressCallback([]() { std::cout << "ahhh" << std::endl; });
	mCanvas->AddWidget(button);
	
	ButtonIconManager* buttonIconManager = Services::Get<ButtonIconManager>();
	ButtonIcon& buttonIcon = buttonIconManager->GetButtonIconForVerb("GLARB");
	button->SetUpTexture(buttonIcon.upTexture);
	button->SetDownTexture(buttonIcon.downTexture);
	button->SetHoverTexture(buttonIcon.hoverTexture);
	button->SetDisabledTexture(buttonIcon.disableTexture);
	
	std::cout << GetTransform()->GetPosition() << std::endl;
}

void ActionBar::UpdateInternal(float deltaTime)
{
	
}
