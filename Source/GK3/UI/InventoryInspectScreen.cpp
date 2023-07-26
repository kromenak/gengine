#include "InventoryInspectScreen.h"

#include "ActionBar.h"
#include "ActionManager.h"
#include "AssetManager.h"
#include "GEngine.h"
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
	
	exitButton->SetUpTexture(gAssetManager.LoadTexture("EXITN.BMP"));
	exitButton->SetDownTexture(gAssetManager.LoadTexture("EXITD.BMP"));
	exitButton->SetHoverTexture(gAssetManager.LoadTexture("EXITHOV.BMP"));
	exitButton->SetDisabledTexture(gAssetManager.LoadTexture("EXITDIS.BMP"));
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
    gLayerManager.PushLayer(&mLayer);
    
	// Get closeup texture or die trying.
	//TODO: Placeholder here?
	Texture* closeupTexture = gInventoryManager.GetInventoryItemCloseupTexture(itemName);
	if(closeupTexture == nullptr)
	{
		std::cout << "Can't show inventory inspect for " << itemName << ": no closeup texture found.";
		return;
	}

	// Save item name.
	mInspectItemName = itemName;
	
	// Set closeup image.
	mCloseupImage->SetUpTexture(closeupTexture);
	mCloseupImage->GetRectTransform()->SetSizeDelta(static_cast<float>(closeupTexture->GetWidth()),
                                                    static_cast<float>(closeupTexture->GetHeight()));
	
	// Actually show the stuff!
	SetActive(true);
}

void InventoryInspectScreen::Hide()
{
    if(!IsActive()) { return; }
	SetActive(false);
    gLayerManager.PopLayer(&mLayer);
}

bool InventoryInspectScreen::IsShowing() const
{
    return gLayerManager.IsTopLayer(&mLayer);
}

void InventoryInspectScreen::OnClicked()
{
    // In demo mode, the inventory inspect screen is "misused" to show some placards to the player explaining what the game is about.
    // In this context, clicking each item just goes to the next one with the TURN_RIGHT action.
    if(GEngine::Instance()->IsDemoMode() && StringUtil::StartsWithIgnoreCase(mInspectItemName, "MS3I"))
    {
        gActionManager.ExecuteAction(mInspectItemName, "TURN_RIGHT");
        return;
    }

	// Show the action bar for this noun.
	gActionManager.ShowActionBar(mInspectItemName, [this](const Action* action) {

        // Perform the action.
		gActionManager.ExecuteAction(action, [this](const Action* action) {

            // After the action completes, check if we still have the inventory item shown.
            // In some rare cases (ex: eating a candy), the item no longer exists, so we should close this screen.
            bool isInInventory = gLayerManager.IsLayerInStack("InventoryLayer");
            if(isInInventory && !gInventoryManager.HasInventoryItem(mInspectItemName))
            {
                Hide();
            }
        });
	});
	
	// Since we're showing the close-up, add INSPECT_UNDO to back out of the close-up.
	ActionBar* actionBar = gActionManager.GetActionBar();
    if(!actionBar->HasVerb("INSPECT_UNDO"))
    {
        actionBar->AddVerbToFront("INSPECT_UNDO", [](){
            gInventoryManager.InventoryUninspect();
        });
    }
}
