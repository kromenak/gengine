//
// GKObject.h
//
// Clark Kromenaker
//
// Base class for an object of interest in a GK3 scene.
//
// Could be a piece of interactable BSP geometry, a prop model in the scene,
// or even a full-fledged humanoid actor that pathfinds, emotes, etc.
//
#pragma once
#include "Actor.h"

#include "Heading.h"

class GKObject : public Actor
{
public:
	GKObject();
	
	// GK3 rotations are often defined in terms of a heading (360 degrees about Y-axis).
	virtual void SetHeading(const Heading& heading);
	Heading GetHeading() const;
    
    void DumpPosition();
	
	void SetNoun(const std::string& noun) { mNoun = noun; }
	const std::string& GetNoun() const { return mNoun; }
	
	void SetVerb(const std::string& verb) { mVerb = verb; }
	const std::string& GetVerb() const { return mVerb; }
	
	bool CanInteract() const { return IsActive() && !mNoun.empty(); }
	
private:
	// A noun is used to refer to the objects in NVC logic.
	// Only objects with nouns can be interacted with!
	std::string mNoun;
	
	// When defined, the verb indicates that interacting with this object ONLY
	// executes this verb in NVC logic. This often also changes the cursor.
	std::string mVerb;
};
