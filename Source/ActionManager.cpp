//
// ActionManager.cpp
//
// Clark Kromenaker
//
#include "ActionManager.h"

#include "ActionBar.h"
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
	for(auto& nvc : mActionSets)
	{
		const std::vector<Action>& allActions = nvc->GetActions(noun);
		for(auto& action : allActions)
		{
			if(nvc->IsCaseMet(&action, ego))
			{
				verbsToActions[action.verb] = &action;
			}
		}
	}
	
	// Finally, convert our map to a vector to return.
	std::vector<const Action*> viableActions;
	for(auto entry : verbsToActions)
	{
		viableActions.push_back(entry.second);
	}
	return viableActions;
}

const Action* ActionManager::GetAction(const std::string& noun, const std::string& verb, GKActor* ego) const
{
	// Cycle through NVCs, trying to find a valid action.
	// Again, any later match will overwrite an earlier match.
	const Action* action = nullptr;
	for(auto& nvc : mActionSets)
	{
		const Action* possibleAction = nvc->GetAction(noun, verb);
		if(possibleAction != nullptr && nvc->IsCaseMet(possibleAction, ego))
		{
			action = possibleAction;
		}
	}
	return action;
}

void ActionManager::ShowActionBar(const std::string& noun, std::function<void(const Action*)> selectCallback)
{
	ShowActionBar(GetActions(noun, GEngine::inst->GetScene()->GetEgo()), selectCallback);
}

void ActionManager::ShowActionBar(const std::vector<const Action*>& actions, std::function<void(const Action*)> selectCallback)
{
	mActionBar->Show(actions, selectCallback);
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
