//
// ActionBar.cpp
//
// Clark Kromenaker
//
#include "ActionBar.h"

#include "ButtonIconManager.h"
#include "InventoryManager.h"
#include "Scene.h"
#include "UIButton.h"
#include "UICanvas.h"
#include "UILabel.h"

ActionBar::ActionBar() : Actor(TransformType::RectTransform)
{
	// Create canvas, to contain the UI components.
	mCanvas = AddComponent<UICanvas>();
	
	// Since we will set the action bar's position based on mouse position, set the anchor to the lower-left corner.
	RectTransform* rectTransform = GetComponent<RectTransform>();
	rectTransform->SetSizeDelta(0.0f, 0.0f);
	rectTransform->SetAnchorMin(Vector2::Zero);
	rectTransform->SetAnchorMax(Vector2::One);
	
	// Create button holder - it holds the buttons and we move it around the screen.
	Actor* buttonHolderActor = new Actor(Actor::TransformType::RectTransform);
	mButtonHolder = buttonHolderActor->GetComponent<RectTransform>();
	mButtonHolder->SetParent(rectTransform);
	mButtonHolder->SetAnchorMin(Vector2::Zero);
	mButtonHolder->SetAnchorMax(Vector2::Zero);
	mButtonHolder->SetPivot(0.5f, 0.5f);
	
	// To have button holder appear in correct spot, we need the holder to be the right height.
	// So, just use one of the buttons to get a valid height.
	ButtonIcon& cancelButtonIcon = Services::Get<ButtonIconManager>()->GetButtonIconForVerb("CANCEL");
	mButtonHolder->SetSizeDelta(cancelButtonIcon.GetWidth(), cancelButtonIcon.GetWidth());
	
	// Hide by default.
	Hide();
}

void ActionBar::Show(std::vector<const Action*> actions, std::function<void(const Action*)> executeCallback)
{
	// Hide if not already hidden.
	Hide();
	
	// If we don't have any actions, don't need to do anything!
	if(actions.size() <= 0) { return; }
	
	// Iterate over all desired actions and show a button for it.
	ButtonIconManager* buttonIconManager = Services::Get<ButtonIconManager>();
	int buttonIndex = 0;
	for(int i = 0; i < actions.size(); ++i)
	{
		ButtonIcon& buttonIcon = buttonIconManager->GetButtonIconForVerb(actions[i]->verb);
		UIButton* actionButton = AddButton(buttonIndex, buttonIcon);
		
		const Action* action = actions[i];
		actionButton->SetPressCallback([this, action, executeCallback]() {
			// Hide action bar on button press.
			this->Hide();
			
			// Execute the action, which will likely run some SheepScript.
			executeCallback(action);
		});
		
		++buttonIndex;
	}
	
	// Get active inventory item for current ego.
	const std::string& egoName = GEngine::inst->GetScene()->GetEgoName();
	std::string activeItemName = Services::Get<InventoryManager>()->GetActiveInventoryItem(egoName);
	if(!activeItemName.empty())
	{
		ButtonIcon& invButtonIcon = buttonIconManager->GetButtonIconForNoun(activeItemName);
		UIButton* invButton = AddButton(buttonIndex, invButtonIcon);
		
		//TODO: Callback for INV button!
		invButton->SetPressCallback([]() {
			std::cout << "Pressed the inventory button." << std::endl;
		});
		
		++buttonIndex;
	}
	
	// Always put cancel button on the end.
	ButtonIcon& cancelButtonIcon = buttonIconManager->GetButtonIconForVerb("CANCEL");
	UIButton* cancelButton = AddButton(buttonIndex, cancelButtonIcon);
	
	// Just hide the bar when cancel is pressed.
	cancelButton->SetPressCallback(std::bind(&ActionBar::Hide, this));
	
	// Refresh layout after adding all buttons to position everything correctly.
	RefreshButtonLayout();
	
	// Position action bar at pointer.
	// This also makes sure it doesn't go offscreen.
	CenterOnPointer();
	
	// It's showing now!
	mButtonHolder->GetOwner()->SetActive(true);
}

void ActionBar::Hide()
{
	// Disable all buttons and put them on the free stack.
	for(auto& button : mButtons)
	{
		button->SetEnabled(false);
		mFreeButtons.push(button);
	}
	mButtons.clear();
	
	// Button holder inactive = no children show.
	mButtonHolder->GetOwner()->SetActive(false);
}

bool ActionBar::IsShowing() const
{
	return mButtonHolder->IsActiveAndEnabled();
}

void ActionBar::AddVerbToFront(const std::string& verb, std::function<void()> callback)
{
	ButtonIcon& icon = Services::Get<ButtonIconManager>()->GetButtonIconForVerb(verb);
	UIButton* button = AddButton(0, icon);
	button->SetPressCallback([this, callback]() {
		this->Hide();
		callback();
	});
	
	RefreshButtonLayout();
	CenterOnPointer();
}

void ActionBar::AddVerbToBack(const std::string& verb, std::function<void()> callback)
{
	ButtonIcon& icon = Services::Get<ButtonIconManager>()->GetButtonIconForVerb(verb);
	
	// Use "-1" to keep Cancel button at the back, no matter what.
	UIButton* button = AddButton(static_cast<int>(mButtons.size() - 1), icon);
	button->SetPressCallback([this, callback]() {
		this->Hide();
		callback();
	});
	
	RefreshButtonLayout();
	CenterOnPointer();
}

void ActionBar::OnUpdate(float deltaTime)
{
	if(mIsShowing && Services::GetInput()->IsKeyDown(SDL_SCANCODE_BACKSPACE))
	{
		Hide();
	}
}

UIButton* ActionBar::AddButton(int index, const ButtonIcon& buttonIcon)
{
	// Reuse a free button or create a new one.
	UIButton* button = nullptr;
	if(mFreeButtons.size() > 0)
	{
		button = mFreeButtons.top();
		mFreeButtons.pop();
		
		// Make sure button is enabled.
		button->SetEnabled(true);
		
		// Make sure any old callbacks are no longer set (since we recycle the buttons).
		button->SetPressCallback(nullptr);
	}
	else
	{
		Actor* buttonActor = new Actor(Actor::TransformType::RectTransform);
		buttonActor->GetTransform()->SetParent(mButtonHolder);
		
		button = buttonActor->AddComponent<UIButton>();
		
		// Add button as a widget.
		mCanvas->AddWidget(button);
	}
	
	// Put into buttons array at desired position.
	mButtons.insert(mButtons.begin() + index, button);
	
	// Set size correctly.
	button->GetRectTransform()->SetSizeDelta(buttonIcon.GetWidth(), buttonIcon.GetWidth());
	
	// Show correct icon on button.
	button->SetUpTexture(buttonIcon.upTexture);
	button->SetDownTexture(buttonIcon.downTexture);
	button->SetHoverTexture(buttonIcon.hoverTexture);
	button->SetDisabledTexture(buttonIcon.disableTexture);
	return button;
}

void ActionBar::RefreshButtonLayout()
{
	// Iterate over all buttons, positioning each one right after the previous one.
	float xPos = 0.0f;
	for(auto& button : mButtons)
	{		
		// Position correctly, relative to previous buttons.
		RectTransform* buttonRT = button->GetRectTransform();
		buttonRT->SetAnchor(Vector2::Zero);
		buttonRT->SetPivot(0.0f, 0.0f);
		buttonRT->SetAnchoredPosition(Vector2(xPos, 0.0f));
		
		xPos += buttonRT->GetSize().GetX();
	}
	
	// Update the button holder to match the size of all buttons.
	mButtonHolder->SetSizeDeltaX(xPos);
}

void ActionBar::CenterOnPointer()
{
	// Position action bar at mouse position.
	mButtonHolder->SetAnchoredPosition(Services::GetInput()->GetMousePosition());
	
	// Keep inside the screen.
	//TODO: Seems like this might be generally useful...perhaps a RectTransform "KeepInRect(Rect)" function?
	// Get min/max for rect of the holder.
	Rect screenRect = mCanvas->GetRectTransform()->GetWorldRect();
	//Vector2 screenRectMin = screenRect.GetMin();
	Vector2 screenRectMax = screenRect.GetMax();
	
	Rect buttonHolderRect = mButtonHolder->GetWorldRect();
	Vector2 min = buttonHolderRect.GetMin();
	Vector2 max = buttonHolderRect.GetMax();
	
	Vector2 anchoredPos = mButtonHolder->GetAnchoredPosition();
	if(min.GetX() < 0)
	{
		anchoredPos.SetX(anchoredPos.GetX() - min.GetX());
	}
	if(max.GetX() > screenRectMax.GetX())
	{
		anchoredPos.SetX(anchoredPos.GetX() - (max.GetX() - screenRectMax.GetX()));
	}
	if(min.GetY() < 0)
	{
		anchoredPos.SetY(anchoredPos.GetY() - min.GetY());
	}
	if(max.GetY() > screenRectMax.GetY())
	{
		anchoredPos.SetY(anchoredPos.GetY() - (max.GetY() - screenRectMax.GetY()));
	}
	mButtonHolder->SetAnchoredPosition(anchoredPos);
}
