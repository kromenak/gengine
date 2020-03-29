//
// ActionManager.h
//
// Clark Kromenaker
//
// Manages showing/executing action bar.
//
#pragma once
#include <functional>
#include <string>
#include <vector>

#include "Type.h"

class Action;
class ActionBar;
class GKActor;
class NVC;
class Timeblock;

class ActionManager
{
	TYPE_DECL_BASE();
public:
	ActionManager();
	
	// Action set population.
	void ClearActionSets() { mActionSets.clear(); }
	void AddActionSet(const std::string& assetName);
	void AddActionSetIfForTimeblock(const std::string& assetName, const Timeblock& timeblock);
	void AddGlobalAndInventoryActionSets(const Timeblock& timeblock);
	
	// Action queries.
	const std::vector<NVC*> GetActionSets() const { return mActionSets; }
	std::vector<const Action*> GetActions(const std::string& noun, GKActor* ego) const;
	const Action* GetAction(const std::string& noun, const std::string& verb, GKActor* ego) const;
	
	// Action bar UI.
	void ShowActionBar(const std::string& noun, std::function<void(const Action*)> selectCallback);
	bool IsActionBarShowing() const;
	ActionBar* GetActionBar() const { return mActionBar; }
	
private:
	const std::string kGlobalActionSets[11] {
		"GLB_ALL.NVC",
		"GLB_23ALL.NVC",
		"GLB102P.NVC",
		"GLB202A.NVC",
		"GLB210A.NVC",
		"GLB212P.NVC",
		"GLB205P.NVC",
		"GLB307A.NVC",
		"GLB310A.NVC",
		"GLB312P.NVC",
		"GLB306P.NVC"
	};
	
	const std::string kInventoryActionSets[16] {
		"INV_ALL.NVC",
		"INV_1ALL.NVC",
		"INV_23ALL.NVC",
		"INV_3ALL.NVC",
		"INV110A.NVC",
		"INV102P.NVC",
		"INV104P.NVC",
		"INV202A.NVC",
		"INV207A.NVC",
		"INV210A.NVC",
		"INV212P.NVC",
		"INV202P.NVC",
		"INV205P.NVC",
		"INV307A.NVC",
		"INV312P.NVC",
		"INV303P.NVC"
	};
	
	// Action sets that are currently active. Note that these change pretty frequenrly (i.e. on scene change).
	// When asked to show action bar, the game uses these to determine what valid actions are for a noun.
	std::vector<NVC*> mActionSets;
	
	// Action bar, which the player uses to perform actions on scene objects.
	ActionBar* mActionBar = nullptr;
	
	// Some assets should only be loaded for certain timeblocks.
	// The asset name indicates this (e.g. GLB_12ALL.NVC or GLB_110A.NVC).
	// Checks asset name against current timeblock to see if the asset should be used.
	bool IsActionSetForTimeblock(const std::string& assetName, const Timeblock& timeblock);
};
