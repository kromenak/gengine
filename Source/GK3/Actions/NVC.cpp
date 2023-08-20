#include "NVC.h"

#include "IniParser.h"
#include "SheepManager.h"

void NVC::Load(uint8_t* data, uint32_t dataLength)
{
    ParseFromData(data, dataLength);
}

const std::vector<Action>& NVC::GetActions(const std::string& noun) const
{
	auto it = mNounToActions.find(noun);
	if(it != mNounToActions.end())
	{
        return it->second;
	}
    else
    {
        static std::vector<Action> mEmptyActions;
        return mEmptyActions;
    }
}

std::vector<const Action*> NVC::GetActions(const std::string& noun, const std::string& verb) const
{
	// Find any exact matches for this noun/verb combo.
    std::vector<const Action*> actions;
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

int NVC::GetActionsCount(const std::string& noun, const std::string& verb) const
{
    // Similar logic to GetActions above, just doesn't create/return a list.
    int count = 0;
    const std::vector<Action>& actionsForNoun = GetActions(noun);
    for(auto& action : actionsForNoun)
    {
        if(StringUtil::EqualsIgnoreCase(action.verb, verb))
        {
            ++count;
        }
    }
    return count;
}

const Action* NVC::GetAction(const std::string& noun, const std::string& verb) const
{
    // Similar to GetActions, but only returns a single Action.
	const std::vector<Action>& actionsForNoun = GetActions(noun);
	for(auto& action : actionsForNoun)
	{
		// This action matches if the action's verb exactly matches the passed in verb.
		if(StringUtil::EqualsIgnoreCase(action.verb, verb))
		{
			return &action;
		}
	}
	return nullptr;
}

void NVC::ParseFromData(uint8_t* data, uint32_t dataLength)
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
		
		// Second entry is the verb.
		IniKeyValue& second = line.entries[1];
        action.verb = second.key;
        
		// Third entry is always the case (requires a bit of trimming/conditioning sometimes).
		IniKeyValue& third = line.entries[2];
		std::string caseLabel = third.key;
		StringUtil::RemoveAll(caseLabel, '\t'); //TODO: Still needed? We also do this in IniParser layer now.
		StringUtil::Trim(caseLabel);
		action.caseLabel = caseLabel;
        
        // From here, we have some optional stuff.
		for(size_t i = 3; i < line.entries.size(); ++i)
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
            else if(StringUtil::EqualsIgnoreCase(keyValue.key, "TalkTo"))
            {
                action.talkTo = keyValue.value;
            }
            else if(StringUtil::EqualsIgnoreCase(keyValue.key, "Script"))
            {
                // A sheep expression to be evaluated for this item.
				//TODO: Should we compile this immediately, or save it as a string and compile/execute on-demand?
				//TODO: Based on debug output from GK3, the string value is stored SOMEWHERE in memory for debug and dump purposes.
				action.script.text = keyValue.value;
				
				// Some NVC scripts actually have a typo - missing close bracket.
				// If that's the case, let's add it...might also be able to deal with this by adjusting parser/grammer.
				if(action.script.text.back() != '}')
				{
                    action.script.text.push_back('}');
				}
				
				// Compile and save script.
				action.script.script = gSheepManager.Compile("Case Evaluation", action.script.text);
            }
		}

        // If a target was specified, but no approach, default to "Walk To" approach.
        if(action.approach == Action::Approach::None && !action.target.empty())
        {
            action.approach = Action::Approach::WalkTo;
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
	for(auto& mNounToAction : mNounToActions)
	{
		std::vector<Action>& actions = mNounToAction.second;
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
            SheepScriptAndText caseLogic;
            caseLogic.text = first.value;
            caseLogic.script = gSheepManager.CompileEval(first.value);
            mCaseLogic[caseLabel] = caseLogic;
        }
        else
        {
            std::cout << "Multiple case labels for " << caseLabel << std::endl;
        }
    }
}
