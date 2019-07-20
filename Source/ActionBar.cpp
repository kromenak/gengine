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
	
	// Since we will set the action bar's position based on mouse position,
	// set the anchor to the lower-left corner.
	RectTransform* rectTransform = GetComponent<RectTransform>();
	rectTransform->SetAnchorMin(Vector2::Zero);
	
	/*
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
	button->SetPressCallback([]() { std::cout << "ahhh" << std::endl; });
	mCanvas->AddWidget(button);
	
	ButtonIconManager* buttonIconManager = Services::Get<ButtonIconManager>();
	ButtonIcon& buttonIcon = buttonIconManager->GetButtonIconForVerb("GLARB");
	button->SetUpTexture(buttonIcon.upTexture);
	button->SetDownTexture(buttonIcon.downTexture);
	button->SetHoverTexture(buttonIcon.hoverTexture);
	button->SetDisabledTexture(buttonIcon.disableTexture);
	
	std::cout << GetTransform()->GetPosition() << std::endl;
	*/
}

void ActionBar::Show(std::vector<const NVCItem*> actions, std::function<void(const NVCItem*)> executeCallback)
{
	Hide();
	
	// If we don't have any actions, don't need to do anything!
	if(actions.size() <= 0) { return; }
	
	// Iterate over all desired actions and show a button for it.
	ButtonIconManager* buttonIconManager = Services::Get<ButtonIconManager>();
	int buttonIndex = 0;
	float xPos = 0.0f;
	for(int i = 0; i < actions.size(); i++)
	{
		ButtonIcon& buttonIcon = buttonIconManager->GetButtonIconForVerb(actions[i]->verb);
		UIButton* actionButton = AddButton(buttonIndex, xPos, buttonIcon);
		
		const NVCItem* action = actions[i];
		actionButton->SetPressCallback([this, action, executeCallback]() {
			// Hide action bar on button press. Make sure to do this BEFORE executing action.
			// The execute might lead to a scene change, which deletes this object!
			this->Hide();
			
			// Execute the action, which will likely run some SheepScript.
			executeCallback(action);
		});
		
		buttonIndex++;
		xPos += buttonIcon.GetWidth();
	}
	
	//TODO: Deal with showing active inventory item after verbs.
	// (just gonna use a placeholder for now)
	ButtonIcon& invButtonIcon = buttonIconManager->GetButtonIconForNoun("CANDY");
	AddButton(buttonIndex, xPos, invButtonIcon);
	
	buttonIndex++;
	xPos += invButtonIcon.GetWidth();
	
	// Always put cancel button on the end.
	ButtonIcon& cancelButtonIcon = buttonIconManager->GetButtonIconForVerb("CANCEL");
	UIButton* cancelButton = AddButton(buttonIndex, xPos, cancelButtonIcon);
	
	// Just hide the bar when cancel is pressed.
	cancelButton->SetPressCallback(std::bind(&ActionBar::Hide, this));
	
	// Position action bar at mouse position.
	SetPosition(Services::GetInput()->GetMousePosition() - Vector2(xPos / 2.0f, 0.0f));
	
	//TODO: Make sure the bar doesn't go off screen.
	
	// It's showing now!
	mIsShowing = true;
}

void ActionBar::Hide()
{
	// Remove all widgets.
	mCanvas->RemoveAllWidgets();
	
	// Not showing anymore.
	mIsShowing = false;
}

void ActionBar::UpdateInternal(float deltaTime)
{
	if(mIsShowing && Services::GetInput()->IsKeyDown(SDL_SCANCODE_BACKSPACE))
	{
		Hide();
	}
}

UIButton* ActionBar::AddButton(int index, float xPos, const ButtonIcon& buttonIcon)
{
	// Either reuse a previously created button, or create a new one.
	UIButton* button = nullptr;
	if(index < mButtons.size())
	{
		button = mButtons[index];
	}
	else
	{
		Actor* buttonActor = new Actor(Actor::TransformType::RectTransform);
		buttonActor->GetTransform()->SetParent(GetTransform());
		
		button = buttonActor->AddComponent<UIButton>();
		mButtons.push_back(button);
	}
	
	// Add button as a widget (so it'll render).
	mCanvas->AddWidget(button);
	
	// Position correctly, relative to previous buttons.
	Transform* buttonTransform = button->GetOwner()->GetTransform();
	buttonTransform->SetPosition(Vector3(xPos, 0.0f, 0.0f));
	
	// Show correct icon on button.
	button->SetUpTexture(buttonIcon.upTexture);
	button->SetDownTexture(buttonIcon.downTexture);
	button->SetHoverTexture(buttonIcon.hoverTexture);
	button->SetDisabledTexture(buttonIcon.disableTexture);
	
	// Return button to caller, so they can do any additional stuff (like specify a callback).
	return button;
}
