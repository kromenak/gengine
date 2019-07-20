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

struct NVCItem
{
	// The noun is the thing we are interacting with.
    std::string noun;
	
	// The verb is what action we perform on the noun.
    std::string verb;
	
	// The condition can be global or local.
	// Some global conditions: ALL, GABE_ALL, GRACE_ALL,
	// 1ST_TIME_BLOCK, TIME_BLOCK_OVERRIDE, OTR_TIME, DIALOGUE_TOPICS_LEFT
	// Can also be a local condition, defined in the file.
    std::string condition;
	
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
	
	// A script to execute when using this NVC item.
    SheepScript* script = nullptr;
	
	void Execute() const;
};

class NVC : public Asset
{
public:
    NVC(std::string name, char* data, int dataLength);
	
    NVCItem* GetNVC(std::string noun, std::string verb);
	
	bool IsCaseMet(const NVCItem* item, GKActor* ego) const;
	
	const std::vector<NVCItem>& GetActionsForNoun(std::string noun);
	const NVCItem* GetAction(std::string noun, std::string verb);
	
private:
	std::vector<NVCItem> mEmptyActions;
	
    // Mapping of noun name to NVC items.
    std::unordered_map<std::string, std::vector<NVCItem>> mNounToItems;
    
    // Mapping of case name to sheep script to eval.
    std::unordered_map<std::string, SheepScript*> mCaseToSheep;
	
	void ParseFromData(char* data, int dataLength);
};
