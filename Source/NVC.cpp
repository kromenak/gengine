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

void NVCItem::Execute() const
{
	Services::GetSheep()->Execute(script);
}

NVC::NVC(std::string name, char* data, int dataLength) : Asset(name)
{
    ParseFromData(data, dataLength);
}

NVCItem* NVC::GetNVC(std::string noun, std::string verb)
{
    // See if we have an entry for this noun. If not, we're done.
    auto it = mNounToItems.find(noun);
    if(it == mNounToItems.end()) { return nullptr; }
    
    // See if we have an item with the given verb.
    std::vector<NVCItem> items = mNounToItems[noun];
    for(auto& item : items)
    {
        if(item.verb == verb)
        {
            return &item;
        }
    }
    
    // Found no matching verb entry.
    return nullptr;
}

bool NVC::IsCaseMet(const NVCItem* item, GKActor* ego) const
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

const std::vector<NVCItem>& NVC::GetActionsForNoun(std::string noun)
{
	auto it = mNounToItems.find(noun);
	if(it != mNounToItems.end())
	{
		return mNounToItems[noun];
	}
	return mEmptyActions;
}

const NVCItem* NVC::GetAction(std::string noun, std::string verb)
{
	const std::vector<NVCItem>& actionsForNoun = GetActionsForNoun(noun);
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
    for(auto& entry : mainSection.entries)
    {
        NVCItem item;
        
        // The first three items must be the noun, verb, and case.
        item.noun = entry->key;
        
        IniKeyValue* keyValue = entry->next;
        item.verb = keyValue->key;
        
        keyValue = keyValue->next;
		std::string condition = keyValue->key;
		StringUtil::RemoveAll(condition, '\t');
		StringUtil::Trim(condition);
		item.condition = condition;
        
        // From here, we have some optional stuff.
        keyValue = keyValue->next;
        while(keyValue != nullptr)
        {
			StringUtil::RemoveAll(keyValue->key, '\t');
			StringUtil::Trim(keyValue->key);
			
			if(StringUtil::EqualsIgnoreCase(keyValue->key, "Approach"))
            {
				if(StringUtil::EqualsIgnoreCase(keyValue->value, "WalkTo"))
				{
					item.approach = NVCItem::Approach::WalkTo;
				}
				else if(StringUtil::EqualsIgnoreCase(keyValue->value, "Anim"))
				{
					item.approach = NVCItem::Approach::Anim;
				}
				else if(StringUtil::EqualsIgnoreCase(keyValue->value, "Near"))
				{
					item.approach = NVCItem::Approach::Near;
				}
				else if(StringUtil::EqualsIgnoreCase(keyValue->value, "NearModel"))
				{
					item.approach = NVCItem::Approach::NearModel;
				}
				else if(StringUtil::EqualsIgnoreCase(keyValue->value, "Region"))
				{
					item.approach = NVCItem::Approach::Region;
				}
				else if(StringUtil::EqualsIgnoreCase(keyValue->value, "TurnTo"))
				{
					item.approach = NVCItem::Approach::TurnTo;
				}
				else if(StringUtil::EqualsIgnoreCase(keyValue->value, "TurnToModel"))
				{
					item.approach = NVCItem::Approach::TurnToModel;
				}
				else if(StringUtil::EqualsIgnoreCase(keyValue->value, "WalkToSee"))
				{
					item.approach = NVCItem::Approach::WalkToSee;
				}
				else
				{
					std::cout << "ERROR: invalid approach " << keyValue->value << std::endl;
				}
            }
            else if(StringUtil::EqualsIgnoreCase(keyValue->key, "Target"))
            {
                item.target = keyValue->value;
            }
            else if(StringUtil::EqualsIgnoreCase(keyValue->key, "Script"))
            {
                // A sheep expression to be evaluated for this item.
                SheepCompiler compiler;
                item.script = compiler.Compile(keyValue->value);
            }
            keyValue = keyValue->next;
        }
        
        // Add item to map.
        auto it = mNounToItems.find(item.noun);
        if(it == mNounToItems.end())
        {
            mNounToItems[item.noun] = std::vector<NVCItem>();
        }
        mNounToItems[item.noun].push_back(item);
    }
    
    // Some "CASE" values are special, and handled by the system (like ALL, GABE_ALL, GRACE_ALL)
    // But an NVC item can also specify a custom case value. In that case,
    // this section maps the case value to a sheep expression to evaluate, to see whether the case is met.
    IniSection logicSection = parser.GetSection("LOGIC");
    for(auto& entry : logicSection.entries)
    {
        // Only add this case entry if it isn't a duplicate entry.
        std::string caseLabel = entry->key;
		
		// We need to trim the case label to get the right label value.
		// These are often formatted as "CASE    = blah" (note the extra whitespaces after the case label).
		StringUtil::Trim(caseLabel);
		
		// Add case to dictionary, if not already in there.
		// Multiple cases are considered an error - only the first is kept.
        auto it = mCaseToSheep.find(caseLabel);
        if(it == mCaseToSheep.end())
        {
            SheepCompiler compiler;
            mCaseToSheep[caseLabel] = compiler.Compile(entry->value);
        }
        else
        {
            std::cout << "Multiple case labels for " << caseLabel << std::endl;
        }
    }
}
