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
	bool FindPath(Vector3 from, Vector3 to, std::vector<Vector3>& outPath) const;
	Vector3 FindNearestWalkablePosition(const Vector3& position) const;
	
	void SetTexture(Texture* texture) { mTexture = texture; }
	Texture* GetTexture() const { return mTexture; }
	
	void SetSize(const Vector2& size) { mSize = size; }
	Vector2 GetSize() const { return mSize; }
	
	void SetOffset(const Vector2& offset) { mOffset = offset; }
	Vector2 GetOffset() const { return mOffset; }
	
private:
	// The texture provides vital data about walkable areas.
	// Each pixel correlates to a spot in the scene.
	// The pixel color indicates whether a spot is walkable and how walkable.
	Texture* mTexture = nullptr;
	
	// Size specifies scale of the walker bounds relative to the 3D scene.
	Vector2 mSize;
	
	// An offset for the bottom-left of the walker bounds from the origin.
	Vector2 mOffset;
	
	bool IsWorldPosWalkable(Vector3 worldPos) const;
	bool IsTexturePosWalkable(Vector2 texturePos) const;
	
	Vector2 WorldPosToTexturePos(Vector3 worldPos) const;
	Vector3 TexturePosToWorldPos(Vector2 texturePos) const;
	
	Vector2 FindNearestWalkableTexturePosToWorldPos(const Vector3& worldPos) const;
};
