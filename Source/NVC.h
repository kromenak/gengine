//
// NVC.h
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

class GKActor;
class SheepScript;

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
	
	//TODO: An undocumented parameter: after the action executes, immediately begin a conversation with this person.
	//std::string talkTo;
	
	// A script to run when this action is executed.
	std::string scriptText;
    SheepScript* script = nullptr;
	
	std::string ToString() const { return "'" + noun + ":" + verb + ":" + caseLabel + "': " + scriptText; }
};

class NVC : public Asset
{
public:
    NVC(std::string name, char* data, int dataLength);
	
	const std::vector<Action*> GetActions() const { return mActions; }
	const std::vector<Action>& GetActions(const std::string& noun) const;
	std::vector<const Action*> GetActions(const std::string& noun, const std::string& verb) const;
	const Action* GetAction(const std::string& noun, const std::string& verb) const;
	
	const std::unordered_map<std::string, SheepScript*>& GetCases() const { return mCaseLogic; }
	
private:
	// If attempting to get actions for a noun that doesn't exist,
	// We just return a reference to this empty vector.
	static std::vector<Action> mEmptyActions;
	
	std::vector<Action*> mActions;
	
    // Mapping of noun to actions.
    std::unordered_map<std::string, std::vector<Action>> mNounToActions;
    
    // Mapping of case name to sheep script to eval.
    std::unordered_map<std::string, SheepScript*> mCaseLogic;
	
	void ParseFromData(char* data, int dataLength);
};
