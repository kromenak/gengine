#include "InventoryInspectScreen.h"

#include "ActionBar.h"
#include "ActionManager.h"
#include "InventoryManager.h"
#include "Texture.h"
#include "UICanvas.h"
#include "UIButton.h"
#include "UIImage.h"

InventoryInspectScreen::InventoryInspectScreen() : Actor(TransformType::RectTransform),
    mLayer("CloseUpLayer")
{
    // Inventory overrides SFX/VO, but continues ambient audio from scene.
    mLayer.OverrideAudioState(true, true, false);

    // Add canvas to render UI elements.
	AddComponent<UICanvas>(1);
	
	// Add black background image that blocks out the scene entirely.
	UIImage* background = AddComponent<UIImage>();
	background->SetTexture(&Texture::Black);
	
	RectTransform* inventoryRectTransform = GetComponent<RectTransform>();
	inventoryRectTransform->SetSizeDelta(0.0f, 0.0f);
	inventoryRectTransform->SetAnchorMin(Vector2::Zero);
	inventoryRectTransform->SetAnchorMax(Vector2::One);
	
	// Add exit button to bottom-left corner of screen.
	Actor* exitButtonActor = new Actor(TransformType::RectTransform);
    exitButtonActor->GetTransform()->SetParent(GetTransform());
	UIButton* exitButton = exitButtonActor->AddComponent<UIButton>();
	
	exitButton->SetUpTexture(Services::GetAssets()->LoadTexture("EXITN.BMP"));
	exitButton->SetDownTexture(Services::GetAssets()->LoadTexture("EXITD.BMP"));
	exitButton->SetHoverTexture(Services::GetAssets()->LoadTexture("EXITHOV.BMP"));
	exitButton->SetDisabledTexture(Services::GetAssets()->LoadTexture("EXITDIS.BMP"));
    exitButton->SetPressCallback([this](UIButton* button) {
        Hide();
    });
	
	RectTransform* exitButtonRectTransform = exitButtonActor->GetComponent<RectTransform>();
	exitButtonRectTransform->SetParent(inventoryRectTransform);
	exitButtonRectTransform->SetSizeDelta(58.0f, 26.0f); // texture width/height
	exitButtonRectTransform->SetAnchor(Vector2::Zero);
	exitButtonRectTransform->SetAnchoredPosition(10.0f, 10.0f);
	exitButtonRectTransform->SetPivot(0.0f, 0.0f);
	
	// Create closeup image. It's just positioned at center of screen, which is default.
	Actor* closeupActor = new Actor(TransformType::RectTransform);
    closeupActor->GetTransform()->SetParent(GetTransform());
	mCloseupImage = closeupActor->AddComponent<UIButton>();
	mCloseupImage->GetRectTransform()->SetParent(inventoryRectTransform);
    mCloseupImage->SetPressCallback([this](UIButton* button) {
        OnClicked();
    });
	
	// Hide by default.
    SetActive(false);
}

void InventoryInspectScreen::Show(const std::string& itemName)
{
    Services::Get<LayerManager>()->PushLayer(&mLayer);
    
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
    if(!IsActive()) { return; }
	SetActive(false);
    Services::Get<LayerManager>()->PopLayer(&mLayer);
}

bool InventoryInspectScreen::IsShowing() const
{
    return Services::Get<LayerManager>()->IsTopLayer(&mLayer);
}

void InventoryInspectScreen::OnClicked()
{
	// Show the action bar for this noun.
	Services::Get<ActionManager>()->ShowActionBar(mInspectItemName, [this](const Action* action) {

        // Perform the action.
		Services::Get<ActionManager>()->ExecuteAction(action, [this](const Action* action) {

            // After the action completes, check if we still have the inventory item shown.
            // In some rare cases (ex: eating a candy), the item no longer exists, so we should close this screen.
            bool isInInventory = Services::Get<LayerManager>()->IsLayerInStack("InventoryLayer");
            if(isInInventory && !Services::Get<InventoryManager>()->HasInventoryItem(mInspectItemName))
            {
                Hide();
            }
        });
	});
	
	// Since we're showing the close-up, add INSPECT_UNDO to back out of the close-up.
	ActionBar* actionBar = Services::Get<ActionManager>()->GetActionBar();
    if(!actionBar->HasVerb("INSPECT_UNDO"))
    {
        actionBar->AddVerbToFront("INSPECT_UNDO", [](){
            Services::Get<InventoryManager>()->InventoryUninspect();
        });
    }
}
