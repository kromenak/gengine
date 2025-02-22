#include "ActionManager.h"

#include <cassert>

#include "ActionBar.h"
#include "AssetManager.h"
#include "DialogueManager.h"
#include "GameProgress.h"
#include "GameCamera.h"
#include "GEngine.h"
#include "GKActor.h"
#include "GK3UI.h"
#include "IniParser.h"
#include "Profiler.h"
#include "ReportManager.h"
#include "SceneManager.h"
#include "SheepManager.h"
#include "SheepScript.h"
#include "StringUtil.h"
#include "Timeblock.h"
#include "VerbManager.h"

namespace
{
    void OutputActions(const std::vector<const Action*>& actions)
    {
        for(auto& action : actions)
        {
            std::cout << "Action " << action->ToString() << std::endl;
        }
    }
}

ActionManager gActionManager;

ActionManager::~ActionManager()
{
    // Clean up any leftover script in the custom action.
    if(mCustomAction.script.script != nullptr)
    {
        delete mCustomAction.script.script;
    }
}

void ActionManager::Init()
{
	// Create action bar, which will be used to choose nouns/verbs by the player.
	mActionBar = new ActionBar();
	mActionBar->SetIsDestroyOnLoad(false);
}

void ActionManager::AddActionSet(const std::string& assetName)
{
    // Read in the asset.
	NVC* actionSet = gAssetManager.LoadNVC(assetName, AssetScope::Scene);
    if(actionSet == nullptr) { return; }

    // Log that we're parsing this NVC.
	gReportManager.Log("Generic", StringUtil::Format("Reading NVC file: %s", assetName.c_str()));

    // Populate actions map.
    const std::vector<Action*>& actions = actionSet->GetActions();
    for(auto& action : actions)
    {
        // Retrieve innermost map for this noun/verb combo.
        std::string_map_ci<Action*>& actionsForNounVerb = mActions[action->noun][action->verb];

        // Make sure there isn't already an exact noun/verb/case combo present.
        // This is considered an error, and any duplicates are ignored - only the first instance is used.
        auto it = actionsForNounVerb.find(action->caseLabel);
        if(it != actionsForNounVerb.end())
        {
            //TODO: ERROR!
            continue;
        }

        // Add action to map.
        actionsForNounVerb[action->caseLabel] = action;

        // Add nouns & verbs to lists/maps.
        // This allows us to convert a noun/verb to a unique integer-based ID, and back again.
        // Doing this primarily to support n$ and v$ requirement in Sheep eval logic...
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
		
	// Also build custom case logic map.
    const std::string_map_ci<SheepScriptAndText>& caseLogic = actionSet->GetCases();
    mCaseLogic.insert(caseLogic.begin(), caseLogic.end());
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
    mActions.clear();
	mCaseLogic.clear();
	mNounToEnum.clear();
	mNouns.clear();
	mVerbToEnum.clear();
	mVerbs.clear();
}

bool ActionManager::ExecuteAction(const std::string& noun, const std::string& verb, std::function<void(const Action*)> finishCallback)
{
    // For this noun/verb pair, find the best Action to use in the current scenario.
    Action* action = GetHighestPriorityAction(noun, verb, VerbType::Normal);

    // Execute the action. This handles the null case internally.
    ExecuteAction(action, finishCallback);

    // Return true if we actually played an action, false if not.
    return action != nullptr;
}

void ActionManager::ExecuteAction(const Action* action, std::function<void(const Action*)> finishCallback, bool log)
{
    // Early out if action is null.
	if(action == nullptr)
	{
        if(finishCallback != nullptr)
        {
            finishCallback(action);
        }
		return;
	}
	
	// We should only execute one action at a time.
	if(mCurrentAction != nullptr)
	{
        gReportManager.Log("Actions", StringUtil::Format("Skipping NVC %s - another action is in progress", action->ToString().c_str()));
		return;
	}
	mCurrentAction = action;
    mCurrentActionFinishCallback = finishCallback;
	
	// Log it!
    // This is conditional b/c scene actions are actually logged earlier (before the approach finishes).
    if(log)
    {
        gReportManager.Log("Actions", StringUtil::Format("Playing NVC %s", action->ToString().c_str()));
    }
	
	// Increment action ID.
	++mActionId;
    
    // Remember current camera FOV.
    gSceneManager.GetScene()->GetCamera()->SaveFov();

    // Save frame this action was started on.
    mCurrentActionStartFrame = GEngine::Instance()->GetFrameNumber();
	
	// If no script is associated with the action, that might be an error...
	// But for now, we'll just treat it as action is immediately over.
	if(action->script.script != nullptr)
	{
		// Execute action in Sheep system, call finished function when done.
		gSheepManager.Execute(action->script.script, std::bind(&ActionManager::OnActionExecuteFinished, this), "SceneLayer");
	}
	else
	{
		//TODO: Log?
		OnActionExecuteFinished();
	}
}

void ActionManager::ExecuteSheepAction(const std::string& sheepName, const std::string& functionName, std::function<void(const Action*)> finishCallback)
{
    ExecuteSheepAction("wait CallSheep(\"" + sheepName + "\", \"" + functionName + "\")", finishCallback);
}

void ActionManager::ExecuteSheepAction(const std::string& sheepScriptText, std::function<void(const Action*)> finishCallback)
{
    // This is just a custom action using specific NOUN/VERB/CASE values.
    ExecuteCustomAction("SHEEP_COMMAND", "NONE", "NONE", sheepScriptText, finishCallback);
}

void ActionManager::ExecuteCustomAction(const std::string& noun, const std::string& verb, const std::string& caseLabel, const std::string& sheepScriptText, std::function<void(const Action*)> finishCallback)
{
    // We should only execute one action at a time.
    if(mCurrentAction != nullptr)
    {
        // Kind of a HACK, but we need a temp custom object in this case.
        Action tempAction;
        tempAction.noun = noun;
        tempAction.verb = verb;
        tempAction.caseLabel = caseLabel;
        tempAction.script.text = sheepScriptText;
        gReportManager.Log("Actions", StringUtil::Format("Skipping NVC %s - another action is in progress", tempAction.ToString().c_str()));
        return;
    }

    // Clean up any leftover script from last run.
    if(mCustomAction.script.script != nullptr)
    {
        delete mCustomAction.script.script;
        mCustomAction.script.script = nullptr;
    }
    
    // Populate action.
    mCustomAction.noun = noun;
    mCustomAction.verb = verb;
    mCustomAction.caseLabel = caseLabel;

    // Compile script.
    mCustomAction.script.text = sheepScriptText;
    mCustomAction.script.script = gSheepManager.Compile("ActionSheep", "{ " + sheepScriptText + " }");

    // Use normal action flow from here.
    ExecuteAction(&mCustomAction, finishCallback);
}

void ActionManager::QueueAction(const std::string& noun, const std::string& verb, std::function<void(const Action*)> finishCallback)
{
    // For this noun/verb pair, find the best Action to use in the current scenario.
    Action* action = GetHighestPriorityAction(noun, verb, VerbType::Normal);
    if(action == nullptr) { return; }

    // If no action is playing, we can just play it right now.
    if(!IsActionPlaying())
    {
        ExecuteAction(action, finishCallback);
        return;
    }

    // Otherwise, queue action to play after.
    ActionAndCallback entry;
    entry.action = action;
    entry.callback = finishCallback;
    mActionQueue.push_back(entry);
}

void ActionManager::SkipCurrentAction()
{
    // Avoid recursive calls.
    if(mSkipInProgress) { return; }
    mSkipInProgress = true;

    // Stop any VO or SFX that started playing on or after the start of the action.
    // Assuming that all VO/SFX playing were DUE TO the current action...may not be 100% true. If it's a problem, may have to "tag" sounds somehow.
    gAudioManager.StopOnOrAfterFrame(mCurrentActionStartFrame);

    // The idea here is that the game's execution should immediately "skip" to the end of the current action.
    // The most "global" and unintrusive way I can think to do that is...just run update in a loop until the action is done!
    // So, the game is essentially running in fast-forward, in the background, and not rendering anything until the action has resolved.
    Stopwatch stopwatch;
    int skipCount = 0;
    while(IsActionPlaying())
    {
        // When testing the original game, they seem to fast forward in 10 second increments (which is part of the reason action skip breaks the "5 minute timer" part of the game).
        GEngine::Instance()->UpdateGameWorld(10.0f);
        ++skipCount;
    }
    gReportManager.Log("Console", StringUtil::Format("skipped %i times, skip duration: %i msec", skipCount, static_cast<int>(stopwatch.GetMilliseconds())));

    // Do this again AFTER skipping to stop any audio that may have been triggered during the forced updates.
    //TODO: The audio system, or Sheep system, could mayyyybe not play audio during skips. But that might be quite intrusive.
    gAudioManager.StopOnOrAfterFrame(mCurrentActionStartFrame);

    // Also hide any subtitles, since we skipped everything.
    gGK3UI.HideAllCaptions();

    // Done skipping.
    mSkipInProgress = false;
}

const Action* ActionManager::GetAction(const std::string& noun, const std::string& verb) const
{
	// For any noun/verb pair, there is only ONE possible action that can be performed at any given time.
	// Keep track of the candidate as we iterate from most general/broad to most specific.
	// The most specific valid action will be our candidate.
	const Action* candidate = nullptr;
	
	// If the verb is an inventory item, handle ANY_OBJECT/ANY_INV_ITEM wildcards for noun/verb.
    Action* action = nullptr;
	bool verbIsInventoryItem = gVerbManager.IsInventoryItem(verb);
	if(verbIsInventoryItem)
	{
        action = GetHighestPriorityAction("ANY_OBJECT", "ANY_INV_ITEM", VerbType::Normal);
        if(action != nullptr)
        {
            candidate = action;
        }
	}
	
	// Find any matches for "ANY_OBJECT" and this verb next.
    action = GetHighestPriorityAction("ANY_OBJECT", verb, VerbType::Normal);
    if(action != nullptr)
    {
        candidate = action;
    }
	
	// If the verb is an inventory item, handle noun/ANY_INV_ITEM combo.
	if(verbIsInventoryItem)
	{
        action = GetHighestPriorityAction(noun, "ANY_INV_ITEM", VerbType::Normal);
        if(action != nullptr)
        {
            candidate = action;
        }
	}
	
	// Finally, check for any exact noun/verb matches.
    action = GetHighestPriorityAction(noun, verb, VerbType::Normal);
    if(action != nullptr)
    {
        candidate = action;
    }
	return candidate;
}

std::vector<const Action*> ActionManager::GetActions(const std::string& noun, VerbType verbType) const
{
    // "ANY_OBJECT" is a wildcard. Any action with a noun of "ANY_OBJECT" can be valid for any noun passed in.
    // These are lowest-priority, so we do them first (they might be overwritten later).
    std::unordered_map<std::string, const Action*> verbToAction;
    AddActionsToMap("ANY_OBJECT", verbType, verbToAction);

    // Next, get specific actions for this particular noun.
    std::unordered_map<std::string, const Action*> verbToActionSpecific;
    AddActionsToMap(noun, verbType, verbToActionSpecific);

    // Combine the two maps.
    // If a verb exists in both maps, the specific version overwrites the more general ANY_OBJECT version.
    for(auto& entry : verbToActionSpecific)
    {
        verbToAction[entry.first] = entry.second;
    }

    // SO...in GK3, the nouns LADY_HOWARD & ESTELLE both mysteriously also match the noun LADY_H_ESTELLE.
    // I haven't found any data-driven spot where this equivalence is defined. It *may* be hard-coded in the original game?
    // Anyway, either of these nouns should also match LADY_H_ESTELLE noun.
    if(StringUtil::EqualsIgnoreCase(noun, "LADY_HOWARD") || StringUtil::EqualsIgnoreCase(noun, "ESTELLE"))
    {
        verbToActionSpecific.clear();
        AddActionsToMap("LADY_H_ESTELLE", verbType, verbToActionSpecific);
        for(auto& entry : verbToActionSpecific)
        {
            verbToAction[entry.first] = entry.second;
        }
    }

    // Also GRACE and MOSELY both matching GRACE_N_MOSE...
    if(StringUtil::EqualsIgnoreCase(noun, "GRACE") || StringUtil::EqualsIgnoreCase(noun, "MOSELY"))
    {
        verbToActionSpecific.clear();
        AddActionsToMap("GRACE_N_MOSE", verbType, verbToActionSpecific);
        for(auto& entry : verbToActionSpecific)
        {
            verbToAction[entry.first] = entry.second;
        }
    }

    // Also a boat load of exceptions at Day 2, 2PM, Devil's Armchair...
    if(StringUtil::EqualsIgnoreCase(noun, "DEAD_CLOTHES_HE1") || StringUtil::EqualsIgnoreCase(noun, "DEAD_CLOTHES_HE2"))
    {
        verbToActionSpecific.clear();
        AddActionsToMap("DEAD_CLOTHES", verbType, verbToActionSpecific);
        for(auto& entry : verbToActionSpecific)
        {
            verbToAction[entry.first] = entry.second;
        }
    }
    if(StringUtil::EqualsIgnoreCase(noun, "DEAD_THROAT_HE1") || StringUtil::EqualsIgnoreCase(noun, "DEAD_THROAT_HE2"))
    {
        verbToActionSpecific.clear();
        AddActionsToMap("DEAD_THROATS", verbType, verbToActionSpecific);
        for(auto& entry : verbToActionSpecific)
        {
            verbToAction[entry.first] = entry.second;
        }
    }
    if(StringUtil::EqualsIgnoreCase(noun, "DEAD_FACES_HE1") || StringUtil::EqualsIgnoreCase(noun, "DEAD_FACES_HE2"))
    {
        verbToActionSpecific.clear();
        AddActionsToMap("DEAD_FACES", verbType, verbToActionSpecific);
        for(auto& entry : verbToActionSpecific)
        {
            verbToAction[entry.first] = entry.second;
        }
    }

    // Finally, convert our map to a vector to return.
    std::vector<const Action*> viableActions;
    for(auto entry : verbToAction)
    {
        viableActions.push_back(entry.second);
    }
    //OutputActions(viableActions);
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

void ActionManager::ShowActionBar(const std::string& noun, std::function<void(const Action*)> selectCallback)
{
	std::vector<const Action*> actions = GetActions(noun, VerbType::Normal);
	mActionBar->Show(noun, VerbType::Normal, actions, selectCallback, std::bind(&ActionManager::OnActionBarCanceled, this));
}

bool ActionManager::IsActionBarShowing() const
{
	return mActionBar->IsShowing();
}

void ActionManager::OnPersist(PersistState& ps)
{
    ps.Xfer(PERSIST_VAR(mPlayedTopics));
    ps.Xfer(PERSIST_VAR(mActionId));

    // Since we aren't going to allow saving/loading during actions,
    // we don't have to worry about saving/loading *most* stuff in here...I hope.
}

void ActionManager::ShowTopicBar(const std::string& noun)
{
	// See if we have any more topics to discuss with this noun (person).
	// If not, we will pre-emptively cancel the bar and return.
	auto actions = GetActions(noun, VerbType::Topic);
	if(actions.size() == 0)
    {
        OnActionBarCanceled();
        return;
    }
	
	// Show topics.
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
	// First three letters are always the location code.
	// Arguably, we could care that the location code matches the current location, but that's kind of a given.
	// So, we'll just ignore it.
	std::size_t curIndex = 3;
	
	// Next, there mayyy be an underscore, but maybe not.
	// Skip the underscore, in any case.
	if(assetName[curIndex] == '_')
	{
		++curIndex;
	}

	// See if "all" is in the name.
	// If so, it indicates that the actions are used for all timeblocks on one or more days.
    std::string lowerName = StringUtil::ToLowerCopy(assetName);
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

bool ActionManager::IsCaseMet(const std::string& noun, const std::string& verb, const std::string& caseLabel, VerbType verbType) const
{
	// Empty condition is automatically met.
	if(caseLabel.empty()) { return true; }
    
	// Is this custom case logic? If so, check the custom case!
	auto it = mCaseLogic.find(caseLabel);
	if(it != mCaseLogic.end())
	{
        // For topics, the case logic indicates when a topic should be available, but it DOES NOT indicate when it should no longer be available!
        // As a general rule, topics should only be discussable once (when the case is met), and then they do not appear again after being discussed.
        if(verbType == VerbType::Topic)
        {
            // However, there are exceptions...these topics are allowed to appear multiple times magically.
            //TODO: Unsure how the original game differentiates these...might be related to cancel button presence?
            static std::string_set_ci sIgnoreImplicitTopicCount = {
                "T_HANDSHAKE_A",
                "T_HANDSHAKE_B",
                "T_HANDSHAKE_C",
                "T_HANDSHAKE_D",
                "T_HANDSHAKE_E"
            };
            if(sIgnoreImplicitTopicCount.find(verb) == sIgnoreImplicitTopicCount.end())
            {
                // If we've already discussed this topic, the noun/verb/case combo will exist in this set.
                // If already present, we should NOT show this topic.
                auto it1 = mPlayedTopics.find(noun);
                if(it1 != mPlayedTopics.end())
                {
                    auto it2 = it1->second.find(verb);
                    if(it2 != it1->second.end())
                    {
                        if(it2->second.count(caseLabel) > 0)
                        {
                            return false;
                        }
                    }
                }
            }
        }

		// Case evaluation logic may have magic variables n$ and v$.
		// These variables should hold int-based identifiers for the noun/verb of the action we're evaluating.
		// So, look those up and save the indexes!
		int n = mNounToEnum.at(noun);
		int v = mVerbToEnum.at(verb);
		
		// Evaluate our condition logic with our n$ and v$ values.
		return gSheepManager.Evaluate(it->second.script, n, v);
	}
	
	// Check global case conditions.
	if(StringUtil::EqualsIgnoreCase(caseLabel, "ALL"))
	{
        // For topics, "ALL" has some strange behavior. Despite appearances, it is not ALWAYS available! It is the last thing to be said about a topic.
        // For example, take JEAN:T_TWO_MEN in Lobby on Day 1, 10AM. If you don't do this special logic, the last dialogue can be played forever.
        // So, get total things that can be said about this topic, and if we are one away from that, this condition is met.
        if(verbType == VerbType::Topic)
        {
            int topicCount = 0;
            auto it2 = mActions.find(noun);
            if(it2 != mActions.end())
            {
                auto it3 = it2->second.find(verb);
                if(it3 != it2->second.end())
                {
                    topicCount = it3->second.size();
                }
            }
            return gGameProgress.GetTopicCount(noun, verb) == (topicCount - 1);
        }

		// "ALL" is always met!
		return true;
	}
	else if(StringUtil::EqualsIgnoreCase(caseLabel, "GABE_ALL"))
	{
		// Condition is met if Ego is Gabriel.
        return StringUtil::EqualsIgnoreCase(Scene::GetEgoName(), "Gabriel");
	}
	else if(StringUtil::EqualsIgnoreCase(caseLabel, "GRACE_ALL"))
	{
		// Condition is met if Ego is Grace.
        return StringUtil::EqualsIgnoreCase(Scene::GetEgoName(), "Grace");
	}
	else if(StringUtil::EqualsIgnoreCase(caseLabel, "1ST_TIME"))
	{
		// Condition is met if this is the first time we've executed this action (noun/verb combo).
		if(verbType == VerbType::Topic)
		{
			return gGameProgress.GetTopicCount(noun, verb) == 0;
		}
		else
		{
			return gGameProgress.GetNounVerbCount(noun, verb) == 0;
		}
	}
	else if(StringUtil::EqualsIgnoreCase(caseLabel, "2CD_TIME"))
	{
		// A surprising way to abbreviate "2nd time"...
        // Condition is met if this is the 2nd time we did the action.
		if(verbType == VerbType::Topic)
		{
			return gGameProgress.GetTopicCount(noun, verb) == 1;
		}
		else
		{
			return gGameProgress.GetNounVerbCount(noun, verb) == 1;
		}
	}
	else if(StringUtil::EqualsIgnoreCase(caseLabel, "3RD_TIME"))
	{
		// And again for good measure. True if this is the 3rd time we did the action.
		if(verbType == VerbType::Topic)
		{
			return gGameProgress.GetTopicCount(noun, verb) == 2;
		}
		else
		{
			return gGameProgress.GetNounVerbCount(noun, verb) == 2;
		}
	}
	else if(StringUtil::EqualsIgnoreCase(caseLabel, "OTR_TIME"))
	{
		// Condition is met if this IS NOT the first time we've executed this action (noun/verb combo).
		if(verbType == VerbType::Topic)
		{
			return gGameProgress.GetTopicCount(noun, verb) > 0;
		}
		else
		{
			return gGameProgress.GetNounVerbCount(noun, verb) > 0;
		}
	}
	else if(StringUtil::EqualsIgnoreCase(caseLabel, "DIALOGUE_TOPICS_LEFT"))
	{
		// Condition is met if there are any "topic" type actions available for this noun.
		return HasTopicsLeft(noun);
	}
	else if(StringUtil::EqualsIgnoreCase(caseLabel, "NOT_DIALOGUE_TOPICS_LEFT"))
	{
		// Condition is met if there are no more "topic" type actions available for this noun.
		return !HasTopicsLeft(noun);
	}
    else if(StringUtil::EqualsIgnoreCase(caseLabel, "TIME_BLOCK"))
    {
        // This condition always returns true.
        // In an NVC file, it typically signifies a variant action for the specific timeblock that overrides one of the general SIF actions.
        return true;
    }
	else if(StringUtil::EqualsIgnoreCase(caseLabel, "TIME_BLOCK_OVERRIDE"))
	{
        // This condition is identical to TIME_BLOCK, but it has higher priority when multiple actions can be used.
        return true;
	}
    else if(StringUtil::EqualsIgnoreCase(caseLabel, "EGG"))
    {
        //TODO: Return true if easter eggs are enabled.
        return false;
    }
	//TODO: Add any more global conditions.
	
	// Assume any not found case is false by default.
	std::cout << "Unknown NVC case " << caseLabel << std::endl;
	return false;
}

Action* ActionManager::GetHighestPriorityAction(const std::string& noun, const std::string& verb, VerbType verbType) const
{
    // The action to return - we'll figure this out as we iterate.
    Action* action = nullptr;

    // Find the maps for this noun/verb. If they don't exist, we return nullptr.
    auto nounEntry = mActions.find(noun);
    if(nounEntry != mActions.end())
    {
        auto verbEntry = nounEntry->second.find(verb);
        if(verbEntry != nounEntry->second.end())
        {
            // For a single noun/verb combo, we only want to return a *single* action.
            // HOWEVER, there may be multiple Actions whose cases are met under current game conditions.
            // To resolve this, we must assign different cases different priorities, and return the highest priority valid one at this time!
            int highestScore = 0;
            for(auto& entry : verbEntry->second)
            {
                // The case must be met, for one.
                bool caseMet = IsCaseMet(noun, verb, entry.first, verbType);
                if(!caseMet) { continue; }

                // OK, this Action is totally valid!
                // The only reason we wouldn't use it is if a higher-priority case is met.

                // Determine a "score" value for this action's CASE label. A higher score means the CASE has higher priority.
                // CASE Priority (Lowest to Highest):
                // ALL
                // GABE_ALL / GRACE_ALL
                // TIME_BLOCK
                // OTR_TIME (or custom case with _OTR in it)
                // DIALOGUE_TOPICS_LEFT / NOT_DIALOGUE_TOPICS_LEFT
                // TIME_BLOCK_OVERRIDE
                // Custom Logic - alphabetical order (very strange, imo)
                // 1ST_TIME / 2CD_TIME / 3RD_TIME
                int caseScore = 0;
                if(StringUtil::EqualsIgnoreCase(entry.first, "ALL"))
                {
                    caseScore = 1;
                }
                else if(StringUtil::EqualsIgnoreCase(entry.first, "GABE_ALL") ||
                        StringUtil::EqualsIgnoreCase(entry.first, "GRACE_ALL") ||
                        StringUtil::EqualsIgnoreCase(entry.first, "GABE_ALL_INV") ||
                        StringUtil::EqualsIgnoreCase(entry.first, "GRACE_ALL_INV"))
                {
                    caseScore = 2;
                }
                else if(StringUtil::EqualsIgnoreCase(entry.first, "TIME_BLOCK"))
                {
                    caseScore = 3;
                }
                else if(StringUtil::EqualsIgnoreCase(entry.first, "OTR_TIME") ||
                        StringUtil::ContainsIgnoreCase(entry.first, "_OTR")) // Custom logic with _OTR also get this priority.
                {
                    caseScore = 4;
                }
                else if(StringUtil::EqualsIgnoreCase(entry.first, "DIALOGUE_TOPICS_LEFT") ||
                        StringUtil::EqualsIgnoreCase(entry.first, "NOT_DIALOGUE_TOPICS_LEFT"))
                {
                    caseScore = 5;
                }
                else if(StringUtil::EqualsIgnoreCase(entry.first, "TIME_BLOCK_OVERRIDE"))
                {
                    caseScore = 6;
                }
                else if(StringUtil::EqualsIgnoreCase(entry.first, "1ST_TIME") ||
                        StringUtil::EqualsIgnoreCase(entry.first, "2CD_TIME") ||
                        StringUtil::EqualsIgnoreCase(entry.first, "3RD_TIME"))
                {
                    caseScore = 8;
                }
                else // This must be custom case logic.
                {
                    // Custom case logic is only overridden by 1st/2nd/3rd time cases.
                    caseScore = 7;

                    //TODO: This logic doesn't seem entirely accurate - for example, it doesn't work correctly with GABE_ALL_INV vs. IN_SIDNEY_ADD_DATA.
                    //TODO: Added custom logic above to deal with that, but...we may need to revise this!
                    // If we've already encountered a valid custom case before, AND this custom case is also valid, ties are handled alphabetically.
                    // This seems strange to me, but then again, you've got to handle a tie somehow I guess?
                    if(action != nullptr && highestScore == 7)
                    {
                        // If this new case comes first alphabetically, we'll use the new action instead.
                        // No need to overwrite score, since it's still the same.
                        std::string prevCase = StringUtil::ToLowerCopy(action->caseLabel);
                        std::string newCase = StringUtil::ToLowerCopy(entry.first);
                        if(strcmp(newCase.c_str(), prevCase.c_str()) < 0)
                        {
                            action = entry.second;
                        }
                    }
                }

                // If we found a case with a higher score, we'll use that instead.
                if(caseScore > highestScore)
                {
                    highestScore = caseScore;
                    action = entry.second;
                }
            }
        }
    }
    return action;
}

void ActionManager::AddActionsToMap(const std::string& noun, VerbType verbType, std::unordered_map<std::string, const Action*>& map) const
{
    // Find this noun in the action map.
    auto nounEntry = mActions.find(noun);
    if(nounEntry != mActions.end())
    {
        // Iterate over each verb entry, and try to find ONE valid action per verb.
        // In the situation a verb has multiple valid actions, some tie-breaking logic is used.
        for(auto& verbEntry : nounEntry->second)
        {
            // The "ANY_INV_ITEM" wildcard verb only matches if a specific verb was provided.
            // This function doesn't let you specify a verb, so this should never match.
            bool isWildcardInvItem = StringUtil::EqualsIgnoreCase(verbEntry.first, "ANY_INV_ITEM");
            if(isWildcardInvItem) { continue; }

            // The verb must be of the correct type for us to use it.
            bool validType = false;
            switch(verbType)
            {
            case VerbType::Normal:
                validType = gVerbManager.IsVerb(verbEntry.first);
                break;
            case VerbType::Inventory:
                validType = gVerbManager.IsInventoryItem(verbEntry.first);
                break;
            case VerbType::Topic:
                validType = gVerbManager.IsTopic(verbEntry.first);
                break;
            }
            if(!validType) { continue; }

            // OK, this noun/verb combo seems fine.
            // Now, iterate all possible *cases* for this noun/verb combo, and settle on a single one that we'll use.
            Action* action = GetHighestPriorityAction(noun, verbEntry.first, verbType);

            // Map verb to action.
            if(action != nullptr)
            {
                map[verbEntry.first] = action;
            }
        }
    }
}

void ActionManager::OnActionBarCanceled()
{
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
    
    // Restore current camera FOV.
    gSceneManager.GetScene()->GetCamera()->RestoreFov();

    // If this is a topic, automatically increment topic counts.
    if(gVerbManager.IsTopic(mLastAction->verb))
    {
        // Increment topic count automatically.
        gGameProgress.IncTopicCount(mLastAction->noun, mLastAction->verb);

        // Make a record that this topic action was performed.
        mPlayedTopics[mLastAction->noun][mLastAction->verb].insert(mLastAction->caseLabel);
    }

    // If this is a chat, automatically increment chat counts.
    if(StringUtil::EqualsIgnoreCase(mLastAction->verb, "Z_CHAT"))
    {
        gGameProgress.IncChatCount(mLastAction->noun);
    }
    
    // Execute finish callback if specified.
    if(mCurrentActionFinishCallback != nullptr)
    {
        auto callback = mCurrentActionFinishCallback;
        mCurrentActionFinishCallback = nullptr;
        callback(mLastAction);
    }
	
	// When a "talk" action ends, try to show the topic bar.
	if(StringUtil::EqualsIgnoreCase(mLastAction->verb, "TALK"))
	{
		ShowTopicBar(mLastAction->noun);
	}
	else if(gVerbManager.IsTopic(mLastAction->verb))
	{
		ShowTopicBar(mLastAction->noun);
	}
    else if(gDialogueManager.InConversation()) // *seems* necessary to end conversations started during cutscenes (ex: Gabe/Mosely scene in Dining Room)
    {
        if(!mLastAction->talkTo.empty())
        {
            ShowTopicBar(mLastAction->talkTo);
        }
        else
        {
            ShowTopicBar(mLastAction->noun);
        }
    }
    else if(!IsActionPlaying() && !mActionQueue.empty())
    {
        // Retrieve front item, but remove it BEFORE executing.
        ActionAndCallback front = mActionQueue.front();
        mActionQueue.erase(mActionQueue.begin());

        // Execute the action.
        ExecuteAction(front.action, front.callback);
    }
}
