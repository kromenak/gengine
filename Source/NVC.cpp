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

void Action::Execute() const
{
	Services::GetSheep()->Execute(script);
}

NVC::NVC(std::string name, char* data, int dataLength) : Asset(name)
{
    ParseFromData(data, dataLength);
}

bool NVC::IsCaseMet(const Action* item, GKActor* ego) const
{
	// Empty condition is automatically met.
	if(item->condition.empty()) { return true; }
	
	// Check global case conditions.
	if(StringUtil::EqualsIgnoreCase(item->condition, "all"))
	{
		return true;
	}
	else if(StringUtil::EqualsIgnoreCase(item->condition, "gabe_all"))
	{
		return ego != nullptr && StringUtil::EqualsIgnoreCase(ego->GetNoun(), "gabriel");
	}
	else if(StringUtil::EqualsIgnoreCase(item->condition, "grace_all"))
	{
		return ego != nullptr && StringUtil::EqualsIgnoreCase(ego->GetNoun(), "grace");
	}
	//TODO: Add any more global conditions.
	
	// Next, see if we have a local condition that matches.
	auto it = mCaseToSheep.find(item->condition);
	if(it != mCaseToSheep.end())
	{
		return Services::GetSheep()->Evaluate(it->second);
	}
	
	// Assume any not found case is false by default.
	std::cout << "Unknown NVC case " << item->condition << std::endl;
	return false;
}

const std::vector<Action>& NVC::GetActions(const std::string& noun) const
{
	auto it = mNounToItems.find(StringUtil::ToLowerCopy(noun));
	if(it != mNounToItems.end())
	{
		return it->second;
	}
	return mEmptyActions;
}

const Action* NVC::GetAction(const std::string& noun, const std::string& verb) const
{
	const std::vector<Action>& actionsForNoun = GetActions(noun);
	for(auto& action : actionsForNoun)
	{
		if(StringUtil::EqualsIgnoreCase(action.verb, verb))
		{
			return &action;
		}
	}
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
        Action item;
        
        // The first item is always the noun.
		IniKeyValue& first = line.entries.front();
        item.noun = first.key;
		StringUtil::ToLower(item.noun);
		
		// Second item is always the verb.
		IniKeyValue& second = line.entries[1];
        item.verb = second.key;
        
		// Third item is always the case (requires a bit of trimming/conditioning sometimes).
		IniKeyValue& third = line.entries[2];
		std::string condition = third.key;
		StringUtil::RemoveAll(condition, '\t'); //TODO: Still needed? We also do this in IniParser layer now.
		StringUtil::Trim(condition);
		item.condition = condition;
        
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
					item.approach = Action::Approach::WalkTo;
				}
				else if(StringUtil::EqualsIgnoreCase(keyValue.value, "Anim"))
				{
					item.approach = Action::Approach::Anim;
				}
				else if(StringUtil::EqualsIgnoreCase(keyValue.value, "Near"))
				{
					item.approach = Action::Approach::Near;
				}
				else if(StringUtil::EqualsIgnoreCase(keyValue.value, "NearModel"))
				{
					item.approach = Action::Approach::NearModel;
				}
				else if(StringUtil::EqualsIgnoreCase(keyValue.value, "Region"))
				{
					item.approach = Action::Approach::Region;
				}
				else if(StringUtil::EqualsIgnoreCase(keyValue.value, "TurnTo"))
				{
					item.approach = Action::Approach::TurnTo;
				}
				else if(StringUtil::EqualsIgnoreCase(keyValue.value, "TurnToModel"))
				{
					item.approach = Action::Approach::TurnToModel;
				}
				else if(StringUtil::EqualsIgnoreCase(keyValue.value, "WalkToSee"))
				{
					item.approach = Action::Approach::WalkToSee;
				}
				else
				{
					std::cout << "ERROR: invalid approach " << keyValue.value << std::endl;
				}
            }
            else if(StringUtil::EqualsIgnoreCase(keyValue.key, "Target"))
            {
                item.target = keyValue.value;
            }
            else if(StringUtil::EqualsIgnoreCase(keyValue.key, "Script"))
            {
                // A sheep expression to be evaluated for this item.
				//TODO: Should we compile this immediately, or save it as a string and compile/execute on-demand?
				//TODO: Based on debug output from GK3, the string value is stored SOMEWHERE in memory for debug and dump purposes.
				item.scriptText = keyValue.value;
				item.script = Services::GetSheep()->Compile(keyValue.value);
            }
		}
        
        // Add item to map.
        auto it = mNounToItems.find(item.noun);
        if(it == mNounToItems.end())
        {
            mNounToItems[item.noun] = std::vector<Action>();
        }
        mNounToItems[item.noun].push_back(item);
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
        auto it = mCaseToSheep.find(caseLabel);
        if(it == mCaseToSheep.end())
        {
			//TODO: Again, should save string value somewhere???
            mCaseToSheep[caseLabel] = Services::GetSheep()->Compile(first.value);
        }
        else
        {
            std::cout << "Multiple case labels for " << caseLabel << std::endl;
        }
    }
}
