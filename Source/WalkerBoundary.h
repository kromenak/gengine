//
// WalkerBoundary.h
//
// Clark Kromenaker
//
// Encapsulates logic related to determining where a walker can walk in a scene,
// the path it should take, and any debug/rendering helpers.
//
#pragma once

#include <vector>

#include "Vector2.h"
#include "Vector3.h"

class Texture;

class WalkerBoundary
{
public:
	bool CanWalkTo(Vector3 position) const;
	std::vector<Vector3> FindPath(Vector3 from, Vector3 to) const;
	
	void SetTexture(Texture* texture) { mTexture = texture; }
	void SetSize(Vector2 size) { mSize = size; }
	void SetOffset(Vector2 offset) { mOffset = offset; }
	
private:
	// The texture provides vital data about walkable areas.
	// Each pixel correlates to a spot in the scene. The pixel color indicates
	// whether a spot is walkable, and the specific color seems to indicate desirability/weight.
	Texture* mTexture = nullptr;
	
	// The size and offset are used to map the walker boundary texture to the 3D scene.
	// Both are on the X/Z plane (no height).
	Vector2 mSize;
	Vector2 mOffset;
	
	Vector2 WorldPosToTexturePos(Vector3 worldPos) const;
};
