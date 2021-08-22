#include "ActionManager.h"

#include <cassert>

#include "ActionBar.h"
#include "VerbManager.h"
#include "GameProgress.h"
#include "GKActor.h"
#include "IniParser.h"
#include "Scene.h"
#include "Services.h"
#include "StringUtil.h"
#include "Timeblock.h"

TYPE_DEF_BASE(ActionManager);

void ActionManager::Init()
{
	// Pre-populate the Sheep Command action.
	mSheepCommandAction.noun = "SHEEP_COMMAND";
	mSheepCommandAction.verb = "NONE";
	mSheepCommandAction.caseLabel = "NONE";
	
	// Create action bar, which will be used to choose nouns/verbs by the player.
	mActionBar = new ActionBar();
	mActionBar->SetIsDestroyOnLoad(false);
}

void ActionManager::AddActionSet(const std::string& assetName)
{
	NVC* actionSet = Services::GetAssets()->LoadNVC(assetName);
	if(actionSet != nullptr)
	{
		Services::GetReports()->Log("Generic", StringUtil::Format("Reading NVC file: %s", assetName.c_str()));
		mActionSets.push_back(actionSet);
		
		// Also build case logic map.
		auto caseLogic = actionSet->GetCases();
		mCaseLogic.insert(caseLogic.begin(), caseLogic.end());
		
		// Iterate actions and add nouns/verbs to lists and maps.
		// This allows us to convert a noun/verb to a unique integer-based ID, and back again.
		// Doing this primarily to support n$ and v$ requirement in Sheep eval logic...
		const std::vector<Action*> actions = actionSet->GetActions();
		for(auto& action : actions)
		{
			auto nounIt = mNounToEnum.find(action->noun);
			if(nounIt == mNounToEnum.end())
			{
				mNounToEnum[action->noun] = (int)mNouns.size();
				mNouns.push_back(action->noun);
			}
			
			auto verbIt = mVerbToEnum.find(action->verb);
			if(verbIt == mVerbToEnum.end())
			{
				mVerbToEnum[action->verb] = (int)mVerbs.size();
				mVerbs.push_back(action->verb);
			}
		}
	}
}

void ActionManager::AddActionSetIfForTimeblock(const std::string& assetName, const Timeblock& timeblock)
{
	if(IsActionSetForTimeblock(assetName, timeblock))
	{
		AddActionSet(assetName);
	}
}

void ActionManager::AddGlobalActionSets(const Timeblock& timeblock)
{
	for(auto& actionSet : kGlobalActionSets)
	{
		AddActionSetIfForTimeblock(actionSet, timeblock);
	}
}

void ActionManager::AddInventoryActionSets(const Timeblock& timeblock)
{
	for(auto& actionSet : kInventoryActionSets)
	{
		AddActionSetIfForTimeblock(actionSet, timeblock);
	}
}

void ActionManager::ClearActionSets()
{
	mActionSets.clear();
	mCaseLogic.clear();
	mNounToEnum.clear();
	mNouns.clear();
	mVerbToEnum.clear();
	mVerbs.clear();
}

bool ActionManager::ExecuteAction(const std::string& noun, const std::string& verb, std::function<void(const Action*)> finishCallback)
{
	// Iterate action sets and find a valid candidate for this noun/verb combo.
	// Action sets are loaded such that the LAST valid candidate we find is the one we should use.
	const Action* candidate = nullptr;
	for(auto& actionSet : mActionSets)
	{
		const Action* action = actionSet->GetAction(noun, verb);
		if(action != nullptr && IsCaseMet(action))
		{
			candidate = action;
			continue;
		}
	}
	
	// Execute action if we found a valid one.
	if(candidate != nullptr)
	{
		ExecuteAction(candidate, finishCallback);
		return true;
	}

    // Well...we did technically finish I suppose!
    if(finishCallback != nullptr)
    {
        finishCallback(nullptr);
    }
	return false;
}

void ActionManager::ExecuteAction(const Action* action, std::function<void(const Action*)> finishCallback)
{
	if(action == nullptr)
	{
		//TODO: Log
		return;
	}
	
	// We should only execute one action at a time.
	if(mCurrentAction != nullptr)
	{
		//TODO: Log?
		return;
	}
	mCurrentAction = action;
    mCurrentActionFinishCallback = finishCallback;
	
	// Log it!
	Services::GetReports()->Log("Actions", StringUtil::Format("Playing NVC %s", action->ToString().c_str()));
	
	// Increment action ID.
	mActionId++;
	
	// If this is a topic, automatically increment topic counts.
	if(Services::Get<VerbManager>()->IsTopic(action->verb))
	{
		Services::Get<GameProgress>()->IncTopicCount(action->noun, action->verb);
	}
	
	// If no script is associated with the action, that might be an error...
	// But for now, we'll just treat it as action is immediately over.
	if(action->script != nullptr)
	{
		// Execute action in Sheep system, call finished function when done.
		Services::GetSheep()->Execute(action->script, std::bind(&ActionManager::OnActionExecuteFinished, this));
	}
	else
	{
		//TODO: Log?
		OnActionExecuteFinished();
	}
}

void ActionManager::ExecuteSheepAction(const std::string& sheepName, const std::string& functionName, std::function<void(const Action*)> finishCallback)
{
    // We should only execute one action at a time.
    if(mCurrentAction != nullptr)
    {
        //TODO: Log?
        return;
    }
    mCurrentAction = &mSheepCommandAction;
    mCurrentActionFinishCallback = finishCallback;

    // Log it!
    mSheepCommandAction.scriptText = "wait CallSheep(\"" + sheepName + "\", \"" + functionName + "\")";
    Services::GetReports()->Log("Actions", StringUtil::Format("Playing NVC %s", mSheepCommandAction.ToString().c_str()));

    // Increment action ID.
    mActionId++;

    //TODO: We mayyyy want to actually compile a sheep script snippet and execute it here.
    //TODO: For example, the end conversation does a `SHEEP_COMMAND:NONE:NONE`: wait CallSheep("Name", "Function") in the original game.
    //TODO: But for now, let's just use the sheep command action as a placeholder and execute the function call.
    Services::GetSheep()->Execute(sheepName, functionName, std::bind(&ActionManager::OnActionExecuteFinished, this));
}

void ActionManager::SkipCurrentAction()
{
    // Avoid recursive calls.
    if(mSkipInProgress) { return; }
    mSkipInProgress = true;

    // The idea here is that the game's execution should immediately "skip" to the end of the current action.
    // The most "global" and unintrusive way I can think to do that is...just run update in a loop until the action is done!
    // So, the game is essentially running in fast-forward, in the background, and not rendering anything until the action has resolved.
    std::cout << "Attempt skip current action..." << std::endl;
    int skipCount = 0;
    while(IsActionPlaying())
    {
        GEngine::Instance()->ForceUpdate();
        ++skipCount;
    }
    std::cout << "Skipped " << skipCount << " times" << std::endl;
    mSkipInProgress = false;
}

const Action* ActionManager::GetAction(const std::string& noun, const std::string& verb) const
{
	// For any noun/verb pair, there is only ONE possible action that can be performed at any given time.
	// Keep track of the candidate as we iterate from most general/broad to most specific.
	// The most specific valid action will be our candidate.
	const Action* candidate = nullptr;
	
	// If the verb is an inventory item, handle ANY_OBJECT/ANY_INV_ITEM wildcards for noun/verb.
	bool verbIsInventoryItem = Services::Get<VerbManager>()->IsInventoryItem(verb);
	if(verbIsInventoryItem)
	{
		for(auto& nvc : mActionSets)
		{
			std::vector<const Action*> actionsForAnyObject = nvc->GetActions("ANY_OBJECT", "ANY_INV_ITEM");
			for(auto& action : actionsForAnyObject)
			{
				if(IsCaseMet(action))
				{
					std::cout << "Candidate action for " << noun << "/" << verb << " matches ANY_OBJECT/ANY_INV_ITEM" << std::endl;
					candidate = action;
				}
			}
		}
	}
	
	// Find any matches for "ANY_OBJECT" and this verb next.
	for(auto& nvc : mActionSets)
	{
		std::vector<const Action*> actionsForAnyObject = nvc->GetActions("ANY_OBJECT", verb);
		for(auto& action : actionsForAnyObject)
		{
			if(IsCaseMet(action))
			{
				std::cout << "Candidate action for " << noun << "/" << verb << " matches ANY_OBJECT/" << verb << std::endl;
				candidate = action;
			}
		}
	}
	
	// If the verb is an inventory item, handle noun/ANY_INV_ITEM combo.
	// Find any matches for "ANY_OBJECT" and this verb next.
	if(verbIsInventoryItem)
	{
		for(auto& nvc : mActionSets)
		{
			std::vector<const Action*> actionsForAnyObject = nvc->GetActions(noun, "ANY_INV_ITEM");
			for(auto& action : actionsForAnyObject)
			{
				if(IsCaseMet(action))
				{
					std::cout << "Candidate action for " << noun << "/" << verb << " matches " << noun << "/ANY_INV_ITEM" << std::endl;
					candidate = action;
				}
			}
		}
	}
	
	// Finally, check for any exact noun/verb matches.
	for(auto& nvc : mActionSets)
	{
		std::vector<const Action*> actionsForAnyObject = nvc->GetActions(noun, verb);
		for(auto& action : actionsForAnyObject)
		{
			if(IsCaseMet(action))
			{
				std::cout << "Candidate action for " << noun << "/" << verb << " matches " << noun << "/" << verb << std::endl;
				candidate = action;
			}
		}
	}
	return candidate;
}

std::vector<const Action*> ActionManager::GetActions(const std::string& noun, VerbType verbType) const
{
	// As we find actions for this noun, we don't want repeated "verbs".
	// For example, if two actions exist for the verb "LOOK", we don't want two look actions on the action bar!
	// So, keep track of the verb-to-action mappings; only the first verb with a true case will be used.
	std::unordered_map<std::string, const Action*> verbToAction;
	
	// Iterate all loaded action sets to find valid actions for this noun.
	for(auto& actionSet : mActionSets)
	{
		// Within a single action set, we only want to use the first matching verb.
		// Ex: if NOUN, VERB, CASE1 matches and is then followed by NOUN, VERB, CASE2 (which also matches), ignore the second one.
		std::unordered_set<std::string> usedVerbs;
		
		// "ANY_OBJECT" is a wildcard. Any action with a noun of "ANY_OBJECT" can be valid for any noun passed in.
		// These are lowest-priority, so we do them first (they might be overwritten later).
		const std::vector<Action>& anyObjectActions = actionSet->GetActions("ANY_OBJECT");
		for(auto& action : anyObjectActions)
		{
			// The "ANY_INV_ITEM" wildcard only matches if a specific verb was provided.
			// This function doesn't let you specify a verb, so this never matches.
			bool isWildcardInvItem = StringUtil::EqualsIgnoreCase(action.verb, "ANY_INV_ITEM");
			if(isWildcardInvItem) { continue; }
			
			// Ignore this action if the verb has already been used in this action set.
			if(usedVerbs.find(action.verb) != usedVerbs.end()) { continue; }
			
			// The action's verb must be of the correct type for us to use it.
			bool validType = false;
			switch(verbType)
			{
			case VerbType::Normal:
				validType = Services::Get<VerbManager>()->IsVerb(action.verb);
				break;
			case VerbType::Inventory:
				validType = Services::Get<VerbManager>()->IsInventoryItem(action.verb);
				break;
			case VerbType::Topic:
				validType = Services::Get<VerbManager>()->IsTopic(action.verb);
				break;
			}
			
			// If type is valid and the action meets any case specified, we can use this action!
			if(validType && IsCaseMet(&action, verbType))
			{
				verbToAction[action.verb] = &action;
				usedVerbs.insert(action.verb);
			}
		}
		
		// Clear used verbs here - we allow an exact match to overwrite a wildcard match.
		// Ex: ANY_OBJECT, LOOK, CASE1 matches, but then CANDY, LOOK, CASE2 matches exactly - use the second one.
		usedVerbs.clear();
		
		// Check actions that map directly to this noun.
		const std::vector<Action>& nounActions = actionSet->GetActions(noun);
		for(auto& action : nounActions)
		{
			// The "ANY_INV_ITEM" wildcard only matches if a specific verb was provided.
			// This function doesn't let you specify a verb, so this never matches.
			bool isWildcardInvItem = StringUtil::EqualsIgnoreCase(action.verb, "ANY_INV_ITEM");
			if(isWildcardInvItem) { continue; }
			
			// Ignore this action if the verb has already been used in this action set.
			if(usedVerbs.find(action.verb) != usedVerbs.end()) { continue; }
						
			// The action's verb must be of the correct type for us to use it.
			bool validType = false;
			switch(verbType)
			{
			case VerbType::Normal:
				validType = Services::Get<VerbManager>()->IsVerb(action.verb);
				break;
			case VerbType::Inventory:
				validType = Services::Get<VerbManager>()->IsInventoryItem(action.verb);
				break;
			case VerbType::Topic:
				validType = Services::Get<VerbManager>()->IsTopic(action.verb);
				break;
			}
			
			// If type is valid and the action meets any case specified, we can use this action!
			if(validType && IsCaseMet(&action, verbType))
			{
				verbToAction[action.verb] = &action;
				usedVerbs.insert(action.verb);
			}
		}
	}
	
	// Finally, convert our map to a vector to return.
	std::vector<const Action*> viableActions;
	for(auto entry : verbToAction)
	{
		viableActions.push_back(entry.second);
	}
	return viableActions;
}

bool ActionManager::HasTopicsLeft(const std::string &noun) const
{
	return GetActions(noun, VerbType::Topic).size() > 0;
}

std::string& ActionManager::GetNoun(int nounEnum)
{
	return mNouns[Math::Clamp(nounEnum, 0, (int)mNouns.size() - 1)];
}

std::string& ActionManager::GetVerb(int verbEnum)
{
	return mVerbs[Math::Clamp(verbEnum, 0, (int)mVerbs.size() - 1)];
}

void OutputActions(const std::vector<const Action*>& actions)
{
	for(auto& action : actions)
	{
		std::cout << "Action " << action->ToString() << std::endl;
	}
}

void ActionManager::ShowActionBar(const std::string& noun, std::function<void(const Action*)> selectCallback)
{
	auto actions = GetActions(noun, VerbType::Normal);
	OutputActions(actions);
	mActionBar->Show(noun, VerbType::Normal, actions, selectCallback, std::bind(&ActionManager::OnActionBarCanceled, this));
}

bool ActionManager::IsActionBarShowing() const
{
	return mActionBar->IsShowing();
}

void ActionManager::ShowTopicBar(const std::string& noun)
{
	// See if we have any more topics to discuss with this noun (person).
	// If not, we will pre-emptively cancel the bar and return.
	auto actions = GetActions(noun, VerbType::Topic);
	if(actions.size() == 0)
    {
        std::cout << "No more topics to discuss!" << std::endl;
        OnActionBarCanceled();
        return;
    }
	
	// Show topics.
	OutputActions(actions);
	mActionBar->Show(noun, VerbType::Topic, actions, nullptr, std::bind(&ActionManager::OnActionBarCanceled, this));
}

void ActionManager::ShowTopicBar()
{
	// Attempt to derive noun from current or last action.
	std::string noun;
	if(mCurrentAction != nullptr)
	{
		noun = mCurrentAction->noun;
	}
	else if(mLastAction != nullptr)
	{
		noun = mLastAction->noun;
	}
	
	// Couldn't derive noun to use...so fail.
	if(noun.empty()) { return; }
	
	// Show topic bar with same noun again.
	ShowTopicBar(noun);
}

bool ActionManager::IsActionSetForTimeblock(const std::string& assetName, const Timeblock& timeblock)
{
	std::string lowerName = StringUtil::ToLowerCopy(assetName);
	
	// First three letters are always the location code.
	// Arguably, we could care that the location code matches the current location, but that's kind of a given.
	// So, we'll just ignore it.
	std::size_t curIndex = 3;
	
	// Next, there mayyy be an underscore, but maybe not.
	// Skip the underscore, in any case.
	if(lowerName[curIndex] == '_')
	{
		++curIndex;
	}
	
	// See if "all" is in the name.
	// If so, it indicates that the actions are used for all timeblocks on one or more days.
	std::size_t allPos = lowerName.find("all", curIndex);
	if(allPos != std::string::npos)
	{
		// If "all" is at the current index, it means there's no day constraint - just ALWAYS load this one!
		if(allPos == curIndex)
		{
			return true;
		}
		else
		{
			// "all" is later in the string, meaning intermediate characters indicate which
			// days are OK to use this NVC. So, see if the current day is included!
			for(std::size_t i = curIndex; i < allPos; ++i)
			{
				if(std::isdigit(lowerName[i]))
				{
					int day = std::stoi(std::string(1, lowerName[i]));
					if(day == timeblock.GetDay())
					{
						return true;
					}
				}
			}
		}
	}
	else
	{
		// If "all" did not appear, we assume this is an action set for a specific timeblock ONLY!
		// See if it's the current timeblock!
		std::string currentTimeblock = timeblock.ToString();
		StringUtil::ToLower(currentTimeblock);
		if(lowerName.find(currentTimeblock) != std::string::npos)
		{
			return true;
		}
	}
	
	// Seemingly, this asset should not be used for the current timeblock.
	return false;
}

bool ActionManager::IsCaseMet(const Action* action, VerbType verbType) const
{
	// Empty condition is automatically met.
	if(action->caseLabel.empty()) { return true; }
	
	// See if any "local" case logic matches this action and execute that case if so.
	// Do this before "global" cases b/c an action set *could* declare an override of a global...
	// I'd consider that "not great practice" but it does occur in the game's files a few times.
	auto it = mCaseLogic.find(action->caseLabel);
	if(it != mCaseLogic.end())
	{
		// Case evaluation logic may have magic variables n$ and v$.
		// These variables should hold int-based identifiers for the noun/verb of the action we're evaluating.
		// So, look those up and save the indexes!
		int n = mNounToEnum.at(action->noun);
		int v = mVerbToEnum.at(action->verb);
		
		// Evaluate our condition logic with our n$ and v$ values.
		return Services::GetSheep()->Evaluate(it->second, n, v);
	}
	
	// Check global case conditions.
	if(StringUtil::EqualsIgnoreCase(action->caseLabel, "all"))
	{
		// all: same as "no condition" - condition is always met!
		return true;
	}
	else if(StringUtil::EqualsIgnoreCase(action->caseLabel, "gabe_all"))
	{
		// gabe_all: condition is met if Ego is Gabriel.
		Scene* scene = GEngine::Instance()->GetScene();
		GKActor* ego = scene != nullptr ? scene->GetEgo() : nullptr;
		return ego != nullptr && StringUtil::EqualsIgnoreCase(ego->GetNoun(), "gabriel");
	}
	else if(StringUtil::EqualsIgnoreCase(action->caseLabel, "grace_all"))
	{
		// grace_all: condition is met if Ego is Grace.
		Scene* scene = GEngine::Instance()->GetScene();
		GKActor* ego = scene != nullptr ? scene->GetEgo() : nullptr;
		return ego != nullptr && StringUtil::EqualsIgnoreCase(ego->GetNoun(), "grace");
	}
	else if(StringUtil::EqualsIgnoreCase(action->caseLabel, "1st_time"))
	{
		// 1st_time: condition is met if this is the first time we've executed this action (noun/verb combo).
		if(verbType == VerbType::Topic)
		{
			return Services::Get<GameProgress>()->GetTopicCount(action->noun, action->verb) == 0;
		}
		else
		{
			return Services::Get<GameProgress>()->GetNounVerbCount(action->noun, action->verb) == 0;
		}
	}
	else if(StringUtil::EqualsIgnoreCase(action->caseLabel, "2cd_time"))
	{
		// 2cd_time: a surprising way to abbreviate "2nd time"...condition is met if this is the 2nd time we did the action.
		if(verbType == VerbType::Topic)
		{
			return Services::Get<GameProgress>()->GetTopicCount(action->noun, action->verb) == 1;
		}
		else
		{
			return Services::Get<GameProgress>()->GetNounVerbCount(action->noun, action->verb) == 1;
		}
	}
	else if(StringUtil::EqualsIgnoreCase(action->caseLabel, "3rd_time"))
	{
		// 3rd_time: and again for good measure.
		if(verbType == VerbType::Topic)
		{
			return Services::Get<GameProgress>()->GetTopicCount(action->noun, action->verb) == 2;
		}
		else
		{
			return Services::Get<GameProgress>()->GetNounVerbCount(action->noun, action->verb) == 2;
		}
	}
	else if(StringUtil::EqualsIgnoreCase(action->caseLabel, "otr_time"))
	{
		// otr_time: condition is met if this IS NOT the first time we've executed this action (noun/verb combo).
		if(verbType == VerbType::Topic)
		{
			return Services::Get<GameProgress>()->GetTopicCount(action->noun, action->verb) > 0;
		}
		else
		{
			return Services::Get<GameProgress>()->GetNounVerbCount(action->noun, action->verb) > 0;
		}
	}
	else if(StringUtil::EqualsIgnoreCase(action->caseLabel, "dialogue_topics_left"))
	{
		// dialogue_topics_left: condition is met if there are any "topic" type actions available for this noun.
		return HasTopicsLeft(action->noun);
	}
	else if(StringUtil::EqualsIgnoreCase(action->caseLabel, "not_dialogue_topics_left"))
	{
		// not_dialogue_topics_left: condition is met if there are no more "topic" type actions available for this noun.
		return !HasTopicsLeft(action->noun);
	}
	else if(StringUtil::EqualsIgnoreCase(action->caseLabel, "time_block_override"))
	{
		// time_block_override: not 100% sure...only appears in timeblock-specific NVC files.
		// Possibilities:
		//   1) Evaluates true if this is the first scene of the timeblock.
		//	 2) Evaluates true if NVC's timeblock equals the current timeblock.
		//   3) Evaluates true always...just acts as a "ALL" for a timeblock.
		
		// Example use of this:
		// RC1_ALL.NVC has (TELE_SIGN, LOOK, GABE_ALL), which executes some VO.
		// RC1110A.NVC has (TELE_SIGN, LOOK, TIME_BLOCK_OVERRIDE), which executes different VO.
		// So...seems the idea would be to play the timeblock-specific VO during that timeblock, but fall back on general one otherwise?
		std::cout << "Using TIME_BLOCK_OVERRIDE global condition!" << std::endl;
		return true;
	}
	else if(StringUtil::EqualsIgnoreCase(action->caseLabel, "time_block"))
	{
		//TODO
	}
	//TODO: Add any more global conditions.
	
	// Assume any not found case is false by default.
	std::cout << "Unknown NVC case " << action->caseLabel << std::endl;
	return false;
}

void ActionManager::OnActionBarCanceled()
{
	std::cout << "Action bar canceled." << std::endl;

    // In the original game, this appears to be called every time the action bar disables, regardless of whether it was for a conversation.
    // This in turn calls EndConversation, which calls to DialogueManager::EndConversation.
    ExecuteSheepAction("GLB_ALL", "CodeCallEndConv$", nullptr);
}

void ActionManager::OnActionExecuteFinished()
{
	// This function should only be called if an action is playing.
	assert(mCurrentAction != nullptr);

    // Clear current action.
    // Do this BEFORE callback and topic bar checks, as those may want to start an action themselves.
    mLastAction = mCurrentAction;
    mCurrentAction = nullptr;

    // Execute finish callback if specified.
    if(mCurrentActionFinishCallback != nullptr)
    {
        auto callback = mCurrentActionFinishCallback;
        mCurrentActionFinishCallback = nullptr;
        callback(mLastAction);
    }
	
	// When a "talk" action ends, try to show the topic bar.
	if(StringUtil::EqualsIgnoreCase(mLastAction->verb, "talk"))
	{
		ShowTopicBar(mLastAction->noun);
	}
	else if(Services::Get<VerbManager>()->IsTopic(mLastAction->verb))
	{
		ShowTopicBar(mLastAction->noun);
	}
}
