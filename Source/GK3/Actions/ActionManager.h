//
// Clark Kromenaker
//
// Manages which actions are available for which nouns given current game state.
// 
// Also provides an API for showing the action bar and executing actions.
//
#pragma once
#include <functional>
#include <string>
#include <unordered_map>
#include <vector>

#include "NVC.h"
#include "StringUtil.h"
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
public:
    ~ActionManager();

	void Init();
	
	// Action Set Population
	void AddActionSet(const std::string& assetName);
	void AddActionSetIfForTimeblock(const std::string& assetName, const Timeblock& timeblock);
	void AddInventoryActionSets(const Timeblock& timeblock);
	void AddGlobalActionSets(const Timeblock& timeblock);
	void ClearActionSets();

    // Action Query
    const Action* GetAction(const std::string& noun, const std::string& verb) const;
    std::vector<const Action*> GetActions(const std::string& noun, VerbType verbType) const;
    bool HasTopicsLeft(const std::string& noun) const;
	
	// Action Execution
	bool ExecuteAction(const std::string& noun, const std::string& verb, std::function<void(const Action*)> finishCallback = nullptr);
	void ExecuteAction(const Action* action, std::function<void(const Action*)> finishCallback = nullptr, bool log = true);
    void ExecuteSheepAction(const std::string& sheepName, const std::string& functionName, std::function<void(const Action*)> finishCallback = nullptr);
    void ExecuteSheepAction(const std::string& sheepScriptText, std::function<void(const Action*)> finishCallback = nullptr);
    void ExecuteCustomAction(const std::string& noun, const std::string& verb, const std::string& caseLabel,
                             const std::string& sheepScriptText, std::function<void(const Action*)> finishCallback = nullptr);

    void QueueAction(const std::string& noun, const std::string& verb, std::function<void(const Action*)> finishCallback = nullptr);

	bool IsActionPlaying() const { return mCurrentAction != nullptr; }

    void SkipCurrentAction();
    bool IsSkippingCurrentAction() const { return mSkipInProgress; }
    
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
    // These "global" action sets are loaded for every scene during the appropriate timeblocks.
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

    // These inventory-related actions are loaded for every scene during the appropriate timeblocks.
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

    // Nested maps that map each unique valid Noun/Verb/Case combo to a specific Action.
    // Think of this like a lookup - map[noun] gives you all the actions for that noun, map[noun][verb] gives all actions for that noun/verb, etc.
    std::string_map_ci<std::string_map_ci<std::string_map_ci<Action*>>> mActions;
	
	// An action may specify a "case" under which it is valid.
	// A case label corresponds to a bit of SheepScript that evaluates to either true or false.
	// Cases must be stored here (rather than in Action Sets) because cases can be shared (especially global/inventory ones).
    std::string_map_ci<SheepScriptAndText> mCaseLogic;

    // Tracks all Noun/Verb/Case topic combos that have been played.
    // Comparing save files, the original game *seems* to use something like this (Member:DialogueMgr:0:mLinesPlayed) to decide if a topic is still available.
    std::string_map_ci<std::string_map_ci<std::string_set_ci>> mPlayedTopics;

	// Nouns and verbs that are currently active. Pulled out of action sets as they are loaded.
	// We do this to support the Sheep-eval feature of specifying n$ and v$ variables as wildcards for current noun/verb.
	// To use these, we must map each active noun/verb to an integer and back again.
	std::vector<std::string> mNouns;
	std::string_map_ci<int> mNounToEnum;
	std::vector<std::string> mVerbs;
	std::string_map_ci<int> mVerbToEnum;

    // An action that's used for executing custom "JIT" actions.
    // Used for arbitrary SheepScript execution via action system, as well as misc custom commands.
    Action mCustomAction;

    // The last action is cached in case we need to know what action was performed last.
    // This is most useful when re-showing the topic bar after dialogue cutscenes.
    const Action* mLastAction = nullptr;
    
	// The action that is currently playing/executing. Only one action may execute at a time.
	const Action* mCurrentAction = nullptr;
    
    // A callback to execute when the current action finishes executing.
    std::function<void(const Action*)> mCurrentActionFinishCallback = nullptr;
    
    // What frame the current action started on.
    uint32_t mCurrentActionStartFrame = 0;

    // In rare cases, we may need to queue actions to execute when the current one is finished.
    // Primary use case is executing actions on a timer - if timer expires DURING another action, we need to wait until that one finishes!
    struct ActionAndCallback
    {
        const Action* action = nullptr;
        std::function<void(const Action*)> callback = nullptr;
    };
    std::vector<ActionAndCallback> mActionQueue;

    // Are we skipping the current action? Mainly tracked to avoid recursive skips.
    bool mSkipInProgress = false;
	
	// An identifier for an executing action. Increment on each execution to uniquely identify actions.6
	// This mirrors what's output in GK3 when dumping actions.
    uint32_t mActionId = 0;
    
    // The field-of-view of the camera when action starts, so we can set it back after.
    // In the original game, I observed that this is stored and reverted after an action completes.
    float mActionStartCameraFov = 0.0f;
	
	// Action bar, which the player uses to perform actions on scene objects.
	ActionBar* mActionBar = nullptr;
    
	// Some assets should only be loaded for certain timeblocks. The asset name indicates this (e.g. GLB_12ALL.NVC or GLB_110A.NVC).
	// Checks asset name against current timeblock to see if the asset should be used.
	bool IsActionSetForTimeblock(const std::string& assetName, const Timeblock& timeblock);
	
	// Returns true if the case for an action is met. A case can be a global condition, or some user-defined script to evaluate.
	bool IsCaseMet(const std::string& noun, const std::string& verb, const std::string& caseLabel, VerbType verbType = VerbType::Normal) const;

    // Populates provided map with actions that are valid for the given noun.
    Action* GetHighestPriorityAction(const std::string& noun, const std::string& verb, VerbType verbType) const;
    void AddActionsToMap(const std::string& noun, VerbType verbType, std::unordered_map<std::string, const Action*>& map) const;
    
	// Called when action bar is canceled (press cancel button).
	void OnActionBarCanceled();
	
	// Called when an action finishes executing.
	void OnActionExecuteFinished();
};

extern ActionManager gActionManager;