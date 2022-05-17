#pragma once
#include "SheepSysFunc.h"

shpvoid CombineInvItems(const std::string& firstItemName, const std::string& secondItemName, const std::string& combinedItemName);
shpvoid SetInvItemStatus(const std::string& itemName, const std::string& status);

int DoesEgoHaveInvItem(const std::string& itemName);
shpvoid EgoTakeInvItem(const std::string& itemName);

shpvoid DumpEgoActiveInvItem(); // DEV
shpvoid SetEgoActiveInvItem(const std::string& itemName);

shpvoid ShowInventory();
shpvoid HideInventory();

shpvoid InventoryInspect(const std::string& itemName);
shpvoid InventoryUninspect();

int IsTopLayerInventory();

int DoesGabeHaveInvItem(const std::string& itemName);
int DoesGraceHaveInvItem(const std::string& itemName);