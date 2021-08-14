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
#include <unordered_map>
#include <vector>

#include "NVC.h"
#include "Type.h"

class ActionBar;
class GKActor;
class SheepScript;
class Timeblock;

enum class VerbType
{
	Normal,
	Inventory,
	Topic
};

class ActionManager
{
	TYPE_DECL_BASE();
public:
	void Init();
	
	// Action Set Population
	void AddActionSet(const std::string& assetName);
	void AddActionSetIfForTimeblock(const std::string& assetName, const Timeblock& timeblock);
	void AddInventoryActionSets(const Timeblock& timeblock);
	void AddGlobalActionSets(const Timeblock& timeblock);
	void ClearActionSets();
	
	// Action Execution
	bool ExecuteAction(const std::string& noun, const std::string& verb, std::function<void(const Action*)> finishCallback = nullptr);
	void ExecuteAction(const Action* action, std::function<void(const Action*)> finishCallback = nullptr);
    void ExecuteSheepAction(const std::string& sheepName, const std::string& functionName, std::function<void(const Action*)> finishCallback = nullptr);
	bool IsActionPlaying() const { return mCurrentAction != nullptr; }

    void SkipCurrentAction();
    bool IsSkippingCurrentAction() const { return mSkipInProgress; }

	// Action Query
	const Action* GetAction(const std::string& noun, const std::string& verb) const;
	std::vector<const Action*> GetActions(const std::string& noun, VerbType verbType) const;
	bool HasTopicsLeft(const std::string& noun) const;
	
	// Int-Identifier to Noun/Verb
	std::string& GetNoun(int nounEnum);
	std::string& GetVerb(int verbEnum);
	
	// Action Bar
	void ShowActionBar(const std::string& noun, std::function<void(const Action*)> selectCallback);
	void ShowTopicBar(const std::string& noun);
	void ShowTopicBar();
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
	
	// Action sets that are currently active. Note that these change pretty frequently (i.e. on scene change).
	// When asked to show action bar, the game uses these to determine what valid actions are for a noun.
	std::vector<NVC*> mActionSets;
	
	// An action may specify a "case" under which it is valid.
	// A case label corresponds to a bit of sheepscript that evaluates to either true or false.
	// Cases must be stored here (rather than in Action Sets) because cases can be shared (especially global/inventory ones).
    std::unordered_map<std::string, SheepScript*> mCaseLogic;
	
	// Nouns and verbs that are currently active. Pulled out of action sets as they are loaded.
	// We do this to support the Sheep-eval feature of specifying n$ and v$ variables as wildcards for current noun/verb.
	// To use these, we must map each active noun/verb to an integer and back again.
	std::vector<std::string> mNouns;
	std::unordered_map<std::string, int> mNounToEnum;
	std::vector<std::string> mVerbs;
	std::unordered_map<std::string, int> mVerbToEnum;
	
	// An action that's used for "Sheep Commands."
	// When an arbitrary SheepScript needs to execute through the action system, we use this Action object.
	Action mSheepCommandAction;
	
	// When an action is playing, we save it here. Only one action can play at a time.
	// Also save the last action, in case we need it.
	const Action* mCurrentAction = nullptr;
	const Action* mLastAction = nullptr;

    // A callback to execute when the current action finishes executing.
    std::function<void(const Action*)> mCurrentActionFinishCallback = nullptr;

    // Are we skipping the current action? Mainly tracked to avoid recursive skips.
    bool mSkipInProgress = false;
	
	// An identifier for an executing action.
	// We just increment this value each time an action executes to uniquly identify each action.
	// This mirrors what's output in GK3 when dumping actions.
	int mActionId = 0;
	
	// Action bar, which the player uses to perform actions on scene objects.
	ActionBar* mActionBar = nullptr;
	
	// Some assets should only be loaded for certain timeblocks.
	// The asset name indicates this (e.g. GLB_12ALL.NVC or GLB_110A.NVC).
	// Checks asset name against current timeblock to see if the asset should be used.
	bool IsActionSetForTimeblock(const std::string& assetName, const Timeblock& timeblock);
	
	// Returns true if the case for an action is met.
	// A case can be a global condition, or some user-defined script to evaluate.
	bool IsCaseMet(const Action* item, VerbType verbType = VerbType::Normal) const;
	
	// Called when action bar is canceled (press cancel button).
	void OnActionBarCanceled();
	
	// Called when an action finishes executing.
	void OnActionExecuteFinished();
};
