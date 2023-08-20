//
// Clark Kromenaker
//
// A (noun, verb, case) file, also called an "action" file.
// Specifies actions available to player in a specific location
// during a specific time or range of times.
//
// In-memory representation of .NVC assets.
//
#pragma once
#include "Asset.h"

#include <unordered_map>
#include <vector>

#include "SheepScript.h"
#include "StringUtil.h"

class GKActor;
class SheepScript;

struct SheepScriptAndText
{
    // The sheep text; unfortunately required in some cases to check script contents.
    std::string text;

    // The compiled script.
    SheepScript* script = nullptr;
};

struct Action
{
	// The noun is the thing we are interacting with.
    std::string noun;
	
	// The verb is what action we perform on the noun.
    std::string verb;
	
	// The "case" for this action. A label that refers to a case under which this action is valid.
	// The label can refer to arbitrary SheepScript that evaluates to true/false in the NVC file.
	// Or, it can refer to a hard-coded global condition (e.g. ALL, GABE_ALL, GRACE_ALL).
    std::string caseLabel;
	
	// If desired, an approach can be specified. Ego will "approach" the target
	// before executing the associated script.
	enum class Approach
	{
		None,		// no approach; execute immediately
		WalkTo,		// target will be name of a position (as defined in SIF)
		Anim,		// target will be an animation name (.anm file)
		Near,		// target will be name of a position (as defined in SIF)
		NearModel,	// target will be name of a model
		Region,		// target will be name of a region (as defined in SIF)
		TurnTo,		// target will be name of a position (as defined in SIF)
		TurnToModel,// target will be name of a model
		WalkToSee	// target will be name of a model
	};
	Approach approach = Approach::None;
	
	// A target for the approach; interpreted differently depending on the approach specified.
    std::string target;
    
    // If specified, executing this action begins talking to this person.
	std::string talkTo;
	
	// A script to run when this action is executed.
    SheepScriptAndText script;
	
	std::string ToString() const { return "'" + noun + ":" + verb + ":" + caseLabel + "': " + script.text; }
};

class NVC : public Asset
{
public:
    NVC(const std::string& name, AssetScope scope) : Asset(name, scope) { }

    void Load(uint8_t* data, uint32_t dataLength);
	
	const std::vector<Action*>& GetActions() const { return mActions; }
	const std::vector<Action>& GetActions(const std::string& noun) const;
	std::vector<const Action*> GetActions(const std::string& noun, const std::string& verb) const;
    int GetActionsCount(const std::string& noun, const std::string& verb) const;
	const Action* GetAction(const std::string& noun, const std::string& verb) const;
	
	const std::string_map_ci<SheepScriptAndText>& GetCases() const { return mCaseLogic; }
	
private:
    // A list of all actions contained in this NVC (not sorted in any particular way).
    // Pointers into the noun-to-action map.
	std::vector<Action*> mActions;
	
    // Mapping of noun to actions.
    std::string_map_ci<std::vector<Action>> mNounToActions;
    
    // Mapping of case name to sheep script to eval.
    std::string_map_ci<SheepScriptAndText> mCaseLogic;
	
	void ParseFromData(uint8_t* data, uint32_t dataLength);
};
