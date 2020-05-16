//
// ActionManager.cpp
//
// Clark Kromenaker
//
#include "ActionManager.h"

#include "ActionBar.h"
#include "ButtonIconManager.h"
#include "Scene.h"
#include "Services.h"
#include "StringUtil.h"
#include "Timeblock.h"

TYPE_DEF_BASE(ActionManager);

ActionManager::ActionManager()
{
	// Create action bar, which will be used to choose nouns/verbs by the player.
	mActionBar = new ActionBar();
	mActionBar->SetIsDestroyOnLoad(false);
}

void ActionManager::AddActionSet(const std::string& assetName)
{
	NVC* actionSet = Services::GetAssets()->LoadNVC(assetName);
	if(actionSet != nullptr)
	{
		std::cout << "Using NVC " << assetName << std::endl;
		mActionSets.push_back(actionSet);
	}
}

void ActionManager::AddActionSetIfForTimeblock(const std::string& assetName, const Timeblock& timeblock)
{
	if(IsActionSetForTimeblock(assetName, timeblock))
	{
		AddActionSet(assetName);
	}
}

void ActionManager::AddGlobalAndInventoryActionSets(const Timeblock& timeblock)
{
	for(auto& actionSet : kGlobalActionSets)
	{
		AddActionSetIfForTimeblock(actionSet, timeblock);
	}
	for(auto& actionSet : kInventoryActionSets)
	{
		AddActionSetIfForTimeblock(actionSet, timeblock);
	}
}

std::vector<const Action*> ActionManager::GetActions(const std::string& noun, GKActor* ego) const
{
	// As we iterate, we'll use this to keep track of what verbs are in use.
	// We don't want verb repeats - a new item with the same verb will overwrite the old item.
	std::unordered_map<std::string, const Action*> verbsToActions;
	
	// Check "ANY_OBJECT" wildcard nouns first. They are lowest priority, but match all nouns.
	for(auto& nvc : mActionSets)
	{
		const std::vector<Action>& anyObjectActions = nvc->GetActions("ANY_OBJECT");
		for(auto& action : anyObjectActions)
		{
			// Wildcard verb can't match here. It only matches when a specific verb is provided.
			bool verbIsWildcard = StringUtil::EqualsIgnoreCase(action.verb, "ANY_INV_ITEM");
			if(verbIsWildcard) { continue; }
			
			// Action's verb must ACTUALLY be a verb - no inventory item, no topic.
			// Inventory items only match when a specific verb is provided (see GetActions(noun, verb)).
			// Topics only match for conversations.
			bool isVerb = Services::Get<ButtonIconManager>()->IsVerb(action.verb);
			if(isVerb && nvc->IsCaseMet(&action, ego))
			{
				verbsToActions[action.verb] = &action;
			}
		}
	}
	
	// Check actions that map directly to this noun.
	for(auto& nvc : mActionSets)
	{
		const std::vector<Action>& nounActions = nvc->GetActions(noun);
		for(auto& action : nounActions)
		{
			// Wildcard verb can't match here. It only matches when a specific verb is provided.
			bool verbIsWildcard = StringUtil::EqualsIgnoreCase(action.verb, "ANY_INV_ITEM");
			if(verbIsWildcard) { continue; }
			
			// Again, must be a verb in this scenario.
			bool isVerb = Services::Get<ButtonIconManager>()->IsVerb(action.verb);
			if(isVerb && nvc->IsCaseMet(&action, ego))
			{
				verbsToActions[action.verb] = &action;
			}
		}
	}
	
	// Finally, convert our map to a vector to return.
	std::vector<const Action*> viableActions;
	for(auto entry : verbsToActions)
	{
		std::cout << "Found action " << entry.second->ToString() << std::endl;
		viableActions.push_back(entry.second);
	}
	return viableActions;
}

const Action* ActionManager::GetAction(const std::string& noun, const std::string& verb, GKActor* ego) const
{
	// For any noun/verb pair, there is only ONE possible action that can be performed at any given time in the game.
	// Keep track of the candidate as we iterate from most general/broad to most specific.
	// The most specific valid action will be our candidate.
	const Action* candidate = nullptr;
	
	// If the verb is an inventory item, handle ANY_OBJECT/ANY_INV_ITEM wildcards for noun/verb.
	bool verbIsInventoryItem = Services::Get<ButtonIconManager>()->IsInventoryItem(verb);
	if(verbIsInventoryItem)
	{
		for(auto& nvc : mActionSets)
		{
			std::vector<const Action*> actionsForAnyObject = nvc->GetActions("ANY_OBJECT", "ANY_INV_ITEM");
			for(auto& action : actionsForAnyObject)
			{
				if(nvc->IsCaseMet(action, ego))
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
			if(nvc->IsCaseMet(action, ego))
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
				if(nvc->IsCaseMet(action, ego))
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
			if(nvc->IsCaseMet(action, ego))
			{
				std::cout << "Candidate action for " << noun << "/" << verb << " matches " << noun << "/" << verb << std::endl;
				candidate = action;
			}
		}
	}
	return candidate;
}

void ActionManager::ShowActionBar(const std::string& noun, std::function<void(const Action*)> selectCallback)
{
	mActionBar->Show(noun, GetActions(noun, GEngine::inst->GetScene()->GetEgo()), selectCallback);
}

bool ActionManager::IsActionBarShowing() const
{
	return mActionBar->IsShowing();
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
