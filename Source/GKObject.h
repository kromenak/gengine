//
// GKObject.h
//
// Clark Kromenaker
//
// Base class for in-scene dynamic objects in GK3.
// Holds functionality that's shared for both Props and Actors in GK3.
//
#pragma once
#include "Actor.h"

#include <string>

#include "Heading.h"

class GasPlayer;
class MeshRenderer;
class VertexAnimationPlayer;

class GKObject : public Actor
{
public:
	GKObject();
	GKObject(bool defaultSetup);
	virtual ~GKObject() { }
	
	void SetNoun(std::string noun) { mNoun = noun; }
	std::string GetNoun() const { return mNoun; }
	
	// GK3 objects are often defined in terms of a heading (360 degrees about Y-axis).
	void SetHeading(const Heading& heading);
	Heading GetHeading() const;
	
	void PlayAnimation(VertexAnimation* animation);
	void PlayAnimation(VertexAnimation* animation, int framesPerSecond);
	void SampleAnimation(VertexAnimation* animation, int frame);
	
	MeshRenderer* GetMeshRenderer() const { return mMeshRenderer; }
	
protected:
	// Allows the object to render a 3D mesh.
	MeshRenderer* mMeshRenderer = nullptr;
	
	// Many objects animate using vertex animations.
	VertexAnimationPlayer* mVertexAnimationPlayer = nullptr;
	
	// GAS player allows object to animate in an automated/scripted fashion based on some simple command statements.
	// Used by both actors and props.
	GasPlayer* mGasPlayer = nullptr;
	
private:
	// The object's noun. Almost all props and actors have a noun that is used
	// to refer to the object in SIF/NVC/Sheep logic.
	std::string mNoun;
};
