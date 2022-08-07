#include "SheepAPI_Inventory.h"

#include "InventoryManager.h"
#include "Scene.h"
#include "Services.h"
#include "StringUtil.h"

using namespace std;

shpvoid CombineInvItems(const std::string& firstItemName, const std::string& secondItemName, const std::string& combinedItemName)
{
    // All three items must be valid.
    // Output error messages for ALL invalid items before early out.
    bool firstItemValid = Services::Get<InventoryManager>()->IsValidInventoryItem(firstItemName);
    bool secondItemValid = Services::Get<InventoryManager>()->IsValidInventoryItem(secondItemName);
    bool combinedItemValid = Services::Get<InventoryManager>()->IsValidInventoryItem(combinedItemName);
    if(!firstItemValid)
    {
        Services::GetReports()->Log("Error", "'" + firstItemName + "' is not a valid inventory item name.");
    }
    if(!secondItemValid)
    {
        Services::GetReports()->Log("Error", "'" + secondItemName + "' is not a valid inventory item name.");
    }
    if(!combinedItemValid)
    {
        Services::GetReports()->Log("Error", "'" + combinedItemName + "' is not a valid inventory item name.");
    }
    if(!firstItemValid || !secondItemValid || !combinedItemValid)
    {
        return 0;
    }

    // This function doesn't actually check whether you HAVE any of the items involved in the combining or output.
    // It simply removes the first two (whether they exist or not) and adds the combined (whether you already have it or not).
    const std::string& egoName = Scene::GetEgoName();
    Services::Get<InventoryManager>()->RemoveInventoryItem(egoName, firstItemName);
    Services::Get<InventoryManager>()->RemoveInventoryItem(egoName, secondItemName);
    Services::Get<InventoryManager>()->AddInventoryItem(egoName, combinedItemName);
    return 0;
}
RegFunc3(CombineInvItems, void, string, string, string, IMMEDIATE, REL_FUNC);

int DoesEgoHaveInvItem(const std::string& itemName)
{
    // This function does work with invalid inventory item names.
    const std::string& egoName = Scene::GetEgoName();
    bool hasItem = Services::Get<InventoryManager>()->HasInventoryItem(egoName, itemName);
    return hasItem ? 1 : 0;
}
RegFunc1(DoesEgoHaveInvItem, int, string, IMMEDIATE, REL_FUNC);

int DoesGabeHaveInvItem(const std::string& itemName)
{
    // This function does work with invalid inventory item names.
    bool hasItem = Services::Get<InventoryManager>()->HasInventoryItem("Gabriel", itemName);
    return hasItem ? 1 : 0;
}
RegFunc1(DoesGabeHaveInvItem, int, string, IMMEDIATE, REL_FUNC);

int DoesGraceHaveInvItem(const std::string& itemName)
{
    // This function does work with invalid inventory item names.
    bool hasItem = Services::Get<InventoryManager>()->HasInventoryItem("Grace", itemName);
    return hasItem ? 1 : 0;
}
RegFunc1(DoesGraceHaveInvItem, int, string, IMMEDIATE, REL_FUNC);

shpvoid EgoTakeInvItem(const std::string& itemName)
{
    // It must be a valid inventory item.
    if(!Services::Get<InventoryManager>()->IsValidInventoryItem(itemName))
    {
        Services::GetReports()->Log("Error", "Error: '" + itemName + "' is not a valid inventory item name.");
        return 0;
    }

    // Add to inventory of Ego.
    const std::string& egoName = Scene::GetEgoName();
    Services::Get<InventoryManager>()->AddInventoryItem(egoName, itemName);
    return 0;
}
RegFunc1(EgoTakeInvItem, void, string, IMMEDIATE, REL_FUNC);

shpvoid DumpEgoActiveInvItem()
{
    const std::string& egoName = Scene::GetEgoName();
    std::string activeItem = Services::Get<InventoryManager>()->GetActiveInventoryItem(egoName);
    if(activeItem.empty())
    {
        Services::GetReports()->Log("Dump", "Ego active inventory item is 'NONE'.");
    }
    else
    {
        Services::GetReports()->Log("Dump", "Ego active inventory item is '" + activeItem + "'.");
    }
    return 0;
}
RegFunc0(DumpEgoActiveInvItem, void, IMMEDIATE, DEV_FUNC);

shpvoid SetEgoActiveInvItem(const std::string& itemName)
{
    // It must be a valid inventory item.
    if(!Services::Get<InventoryManager>()->IsValidInventoryItem(itemName))
    {
        Services::GetReports()->Log("Error", "Error: '" + itemName + "' is not a valid inventory item name.");
        return 0;
    }

    // If the item we are setting active is not in our inventory, output a warning (but let it go anyway).
    const std::string& egoName = Scene::GetEgoName();
    if(!Services::Get<InventoryManager>()->HasInventoryItem(egoName, itemName))
    {
        Services::GetReports()->Log("Warning", egoName + " does not have " + itemName + ".");
    }

    // Set the inventory item!
    Services::Get<InventoryManager>()->SetActiveInventoryItem(egoName, itemName);
    return 0;
}
RegFunc1(SetEgoActiveInvItem, void, string, IMMEDIATE, REL_FUNC);

shpvoid ShowInventory()
{
    Services::Get<InventoryManager>()->ShowInventory();
    return 0;
}
RegFunc0(ShowInventory, void, IMMEDIATE, REL_FUNC);

shpvoid HideInventory()
{
    Services::Get<InventoryManager>()->HideInventory();
    return 0;
}
RegFunc0(HideInventory, void, IMMEDIATE, REL_FUNC);

shpvoid InventoryInspect(const std::string& itemName)
{
    Services::Get<InventoryManager>()->InventoryInspect(itemName);
    return 0;
}
RegFunc1(InventoryInspect, void, string, IMMEDIATE, REL_FUNC);

shpvoid InventoryUninspect()
{
    Services::Get<InventoryManager>()->InventoryUninspect();
    return 0;
}
RegFunc0(InventoryUninspect, void, IMMEDIATE, REL_FUNC);

shpvoid SetInvItemStatus(const std::string& itemName, const std::string& status)
{
    // The item name must be valid.
    bool argumentError = false;
    if(!Services::Get<InventoryManager>()->IsValidInventoryItem(itemName))
    {
        Services::GetReports()->Log("Error", "Error: '" + itemName + "' is not a valid inventory item name.");
        argumentError = true;
    }

    // Make sure we're using a valid status.
    // NOTE: The reason I check inputs before doing the action is to emulate how error output works in the original game.
    bool validStatus = false;
    if(StringUtil::EqualsIgnoreCase(status, "NotPlaced")
       || StringUtil::EqualsIgnoreCase(status, "Used")
       || StringUtil::EqualsIgnoreCase(status, "Placed")
       || StringUtil::EqualsIgnoreCase(status, "GraceHas")
       || StringUtil::EqualsIgnoreCase(status, "GabeHas")
       || StringUtil::EqualsIgnoreCase(status, "BothHave")
       || StringUtil::EqualsIgnoreCase(status, "Used"))
    {
        validStatus = true;
    }
    if(!validStatus)
    {
        Services::GetReports()->Log("Error", "Error: '" + status + "' is not a valid inventory status.");
        argumentError = true;
    }

    // Early out if any argument is invalid.
    if(argumentError)
    {
        return 0;
    }

    // Status can be any of: NotPlaced, Placed, GraceHas, GabeHas, BothHave, Used.
    // It's unclear what the relevance of these different statuses are - there's no way to 'get' the status of an inventory item!
    // For now, I'll just boil these statuses down to the important states: have or don't have.
    if(StringUtil::EqualsIgnoreCase(status, "NotPlaced")
       || StringUtil::EqualsIgnoreCase(status, "Placed")
       || StringUtil::EqualsIgnoreCase(status, "Used"))
    {
        // NotPlaced = not in the game = make sure neither ego has it.
        // Placed = placed in the game, but not in anyone's inventory = make sure neither ego has it.
        // Used = item is used and no longer available = make sure neither ego has it.
        Services::Get<InventoryManager>()->RemoveInventoryItem("Gabriel", itemName);
        Services::Get<InventoryManager>()->RemoveInventoryItem("Grace", itemName);
    }
    else if(StringUtil::EqualsIgnoreCase(status, "GraceHas"))
    {
        // Grace has, but also implies that Gabriel DOES NOT have!
        Services::Get<InventoryManager>()->AddInventoryItem("Grace", itemName);
        Services::Get<InventoryManager>()->RemoveInventoryItem("Gabriel", itemName);
    }
    else if(StringUtil::EqualsIgnoreCase(status, "GabeHas"))
    {
        // Gabe has, but also implies that Grace DOES NOT have!
        Services::Get<InventoryManager>()->AddInventoryItem("Gabriel", itemName);
        Services::Get<InventoryManager>()->RemoveInventoryItem("Grace", itemName);
    }
    else if(StringUtil::EqualsIgnoreCase(status, "BothHave"))
    {
        Services::Get<InventoryManager>()->AddInventoryItem("Gabriel", itemName);
        Services::Get<InventoryManager>()->AddInventoryItem("Grace", itemName);
    }
    return 0;
}
RegFunc2(SetInvItemStatus, void, string, string, IMMEDIATE, REL_FUNC);

int IsTopLayerInventory()
{
    // This returns true if top layer is inventory OR inventory inspect screens.
    bool anyShowing = Services::Get<InventoryManager>()->IsInventoryShowing() ||
        Services::Get<InventoryManager>()->IsInventoryInspectShowing();
    return anyShowing ? 1 : 0;
}
RegFunc0(IsTopLayerInventory, int, IMMEDIATE, REL_FUNC);