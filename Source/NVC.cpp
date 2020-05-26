//
// NVC.cpp
//
// Clark Kromenaker
//
#include "NVC.h"

#include "GKActor.h"
#include "IniParser.h"
#include "SheepCompiler.h"
#include "SheepScript.h"
#include "StringUtil.h"

/*static*/ std::vector<Action> NVC::mEmptyActions;

NVC::NVC(std::string name, char* data, int dataLength) : Asset(name)
{
    ParseFromData(data, dataLength);
}

const std::vector<Action>& NVC::GetActions(const std::string& noun) const
{
	auto it = mNounToActions.find(StringUtil::ToLowerCopy(noun));
	if(it != mNounToActions.end())
	{
		return it->second;
	}
	return mEmptyActions;
}

std::vector<const Action*> NVC::GetActions(const std::string& noun, const std::string& verb) const
{
	std::vector<const Action*> actions;
	
	// Find any exact matches for this noun/verb combo.
	const std::vector<Action>& actionsForNoun = GetActions(noun);
	for(auto& action : actionsForNoun)
	{
		if(StringUtil::EqualsIgnoreCase(action.verb, verb))
		{
			actions.push_back(&action);
		}
	}
	
	return actions;
}

const Action* NVC::GetAction(const std::string& noun, const std::string& verb) const
{
	const std::vector<Action>& actionsForNoun = GetActions(noun);
	for(auto& action : actionsForNoun)
	{
		// This action matches if the action's verb exactly matches the passed in verb.
		if(StringUtil::EqualsIgnoreCase(action.verb, verb))
		{
			return &action;
		}
	}
	
	// Couldn't find any action for this noun/verb pair.
	return nullptr;
}

void NVC::ParseFromData(char *data, int dataLength)
{
    IniParser parser(data, dataLength);
    parser.ParseAll();
    
    // Main section contains all the Noun/Verb/Cases on individual lines.
    IniSection mainSection = parser.GetSection("");
    for(auto& line : mainSection.lines)
    {
        Action action;
        
        // The first entry is the noun.
		IniKeyValue& first = line.entries.front();
        action.noun = first.key;
		StringUtil::ToLower(action.noun);
		
		// Second entry is the verb.
		IniKeyValue& second = line.entries[1];
        action.verb = second.key;
		StringUtil::ToLower(action.verb);
        
		// Third entry is always the case (requires a bit of trimming/conditioning sometimes).
		IniKeyValue& third = line.entries[2];
		std::string caseLabel = third.key;
		StringUtil::RemoveAll(caseLabel, '\t'); //TODO: Still needed? We also do this in IniParser layer now.
		StringUtil::Trim(caseLabel);
		action.caseLabel = caseLabel;
        
        // From here, we have some optional stuff.
		for(int i = 3; i < line.entries.size(); ++i)
		{
			IniKeyValue& keyValue = line.entries[i];
			
			//TODO: Still needed? We also do this in IniParser layer now.
			StringUtil::RemoveAll(keyValue.key, '\t');
			StringUtil::Trim(keyValue.key);
			
			if(StringUtil::EqualsIgnoreCase(keyValue.key, "Approach"))
            {
				if(StringUtil::EqualsIgnoreCase(keyValue.value, "None"))
				{
					// Do nothing.
				}
				else if(StringUtil::EqualsIgnoreCase(keyValue.value, "WalkTo"))
				{
					action.approach = Action::Approach::WalkTo;
				}
				else if(StringUtil::EqualsIgnoreCase(keyValue.value, "Anim"))
				{
					action.approach = Action::Approach::Anim;
				}
				else if(StringUtil::EqualsIgnoreCase(keyValue.value, "Near"))
				{
					action.approach = Action::Approach::Near;
				}
				else if(StringUtil::EqualsIgnoreCase(keyValue.value, "NearModel"))
				{
					action.approach = Action::Approach::NearModel;
				}
				else if(StringUtil::EqualsIgnoreCase(keyValue.value, "Region"))
				{
					action.approach = Action::Approach::Region;
				}
				else if(StringUtil::EqualsIgnoreCase(keyValue.value, "TurnTo"))
				{
					action.approach = Action::Approach::TurnTo;
				}
				else if(StringUtil::EqualsIgnoreCase(keyValue.value, "TurnToModel"))
				{
					action.approach = Action::Approach::TurnToModel;
				}
				else if(StringUtil::EqualsIgnoreCase(keyValue.value, "WalkToSee"))
				{
					action.approach = Action::Approach::WalkToSee;
				}
				else
				{
					std::cout << "ERROR: invalid approach " << keyValue.value << std::endl;
				}
            }
            else if(StringUtil::EqualsIgnoreCase(keyValue.key, "Target"))
            {
                action.target = keyValue.value;
            }
            else if(StringUtil::EqualsIgnoreCase(keyValue.key, "Script"))
            {
                // A sheep expression to be evaluated for this item.
				//TODO: Should we compile this immediately, or save it as a string and compile/execute on-demand?
				//TODO: Based on debug output from GK3, the string value is stored SOMEWHERE in memory for debug and dump purposes.
				action.scriptText = keyValue.value;
				action.script = Services::GetSheep()->Compile("Case Evaluation", keyValue.value);
            }
		}
        
        // Add item to map.
        auto it = mNounToActions.find(action.noun);
        if(it == mNounToActions.end())
        {
            mNounToActions[action.noun] = std::vector<Action>();
        }
        mNounToActions[action.noun].push_back(action);
    }
	
	// After all actions have been read in, iterate and save pointers to each in a vector.
	// When action set will be used, we must iterate all actions to map nouns and verbs.
	// Perhaps I can make this more efficient at some point...
	for(auto it = mNounToActions.begin(); it != mNounToActions.end(); it++)
	{
		std::vector<Action>& actions = it->second;
		for(auto& action : actions)
		{
			mActions.push_back(&action);
		}
	}
	
    // Some "CASE" values are special, and handled by the system (like ALL, GABE_ALL, GRACE_ALL)
    // But an NVC item can also specify a custom case value. In that case,
    // this section maps the case value to a sheep expression to evaluate, to see whether the case is met.
    IniSection logicSection = parser.GetSection("LOGIC");
    for(auto& line : logicSection.lines)
    {
        // Only add this case entry if it isn't a duplicate entry.
		IniKeyValue& first = line.entries.front();
        std::string caseLabel = first.key;
		
		// We need to trim the case label to get the right label value.
		// These are often formatted as "CASE    = blah" (note the extra whitespaces after the case label).
		StringUtil::Trim(caseLabel); //TODO: Still needed?
		
		// Add case to dictionary, if not already in there.
		// Multiple cases are considered an error - only the first is kept.
        auto it = mCaseLogic.find(caseLabel);
        if(it == mCaseLogic.end())
        {
			//TODO: Again, should save string value somewhere???
            mCaseLogic[caseLabel] = Services::GetSheep()->CompileEval(first.value);
        }
        else
        {
            std::cout << "Multiple case labels for " << caseLabel << std::endl;
        }
    }
}
