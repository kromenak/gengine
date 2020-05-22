//
// InventoryInspectScreen.cpp
//
// Clark Kromenaker
//
#include "InventoryInspectScreen.h"

#include "ActionBar.h"
#include "ActionManager.h"
#include "InventoryManager.h"
#include "UICanvas.h"
#include "UIButton.h"
#include "UIImage.h"

InventoryInspectScreen::InventoryInspectScreen() : Actor(TransformType::RectTransform)
{
	mCanvas = AddComponent<UICanvas>();
	
	// Add black background image that blocks out the scene entirely.
	UIImage* background = AddComponent<UIImage>();
	mCanvas->AddWidget(background);
	background->SetTexture(Texture::Black);
	
	RectTransform* inventoryRectTransform = GetComponent<RectTransform>();
	inventoryRectTransform->SetSizeDelta(0.0f, 0.0f);
	inventoryRectTransform->SetAnchorMin(Vector2::Zero);
	inventoryRectTransform->SetAnchorMax(Vector2::One);
	
	// Add exit button to bottom-left corner of screen.
	Actor* exitButtonActor = new Actor(TransformType::RectTransform);
	UIButton* exitButton = exitButtonActor->AddComponent<UIButton>();
	mCanvas->AddWidget(exitButton);
	
	exitButton->SetUpTexture(Services::GetAssets()->LoadTexture("EXITN.BMP"));
	exitButton->SetDownTexture(Services::GetAssets()->LoadTexture("EXITD.BMP"));
	exitButton->SetHoverTexture(Services::GetAssets()->LoadTexture("EXITHOV.BMP"));
	exitButton->SetDisabledTexture(Services::GetAssets()->LoadTexture("EXITDIS.BMP"));
	exitButton->SetPressCallback(std::bind(&InventoryInspectScreen::Hide, this));
	
	RectTransform* exitButtonRectTransform = exitButtonActor->GetComponent<RectTransform>();
	exitButtonRectTransform->SetParent(inventoryRectTransform);
	exitButtonRectTransform->SetSizeDelta(58.0f, 26.0f); // texture width/height
	exitButtonRectTransform->SetAnchor(Vector2::Zero);
	exitButtonRectTransform->SetAnchoredPosition(10.0f, 10.0f);
	exitButtonRectTransform->SetPivot(0.0f, 0.0f);
	
	// Create closeup image. It's just positioned at center of screen, which is default.
	Actor* closeupActor = new Actor(TransformType::RectTransform);
	mCloseupImage = closeupActor->AddComponent<UIButton>();
	mCanvas->AddWidget(mCloseupImage);
	mCloseupImage->GetRectTransform()->SetParent(inventoryRectTransform);
	mCloseupImage->SetPressCallback(std::bind(&InventoryInspectScreen::OnClicked, this));
	
	// Hide by default.
	Hide();
}

void InventoryInspectScreen::Show(const std::string& itemName)
{
	// Get closeup texture or die trying.
	//TODO: Placeholder here?
	Texture* closeupTexture = Services::Get<InventoryManager>()->GetInventoryItemCloseupTexture(itemName);
	if(closeupTexture == nullptr)
	{
		std::cout << "Can't show inventory inspect for " << itemName << ": no closeup texture found.";
		return;
	}

	// Save item name.
	mInspectItemName = itemName;
	
	// Set closeup image.
	mCloseupImage->SetUpTexture(closeupTexture);
	mCloseupImage->GetRectTransform()->SetSizeDelta(closeupTexture->GetWidth(), closeupTexture->GetHeight());
	
	// Actually show the stuff!
	SetActive(true);
}

void InventoryInspectScreen::Hide()
{
	SetActive(false);
}

void InventoryInspectScreen::OnClicked()
{
	// Show the action bar for this noun.
	Services::Get<ActionManager>()->ShowActionBar(mInspectItemName, [](const Action* action) {
		Services::Get<ActionManager>()->ExecuteAction(action);
	});
	
	// We want to add an "inspect" verb, which means to show the close-up of the item.
	ActionBar* actionBar = Services::Get<ActionManager>()->GetActionBar();
	actionBar->AddVerbToFront("INSPECT_UNDO", []() {
		Services::Get<InventoryManager>()->InventoryUninspect();
	});
}
