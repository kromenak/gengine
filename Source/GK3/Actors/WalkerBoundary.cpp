#include "WalkerBoundary.h"

#include <queue>
#include <unordered_map>
#include <unordered_set>

#include "Debug.h"
#include "GMath.h"
#include "Texture.h"
#include "ResizableQueue.h"

bool WalkerBoundary::FindPath(const Vector3& from, const Vector3& to, std::vector<Vector3>& outPath) const
{
	// Make sure path vector is empty.
	outPath.clear();
	
	// Pick goal position. If "to" is walkable, we can use it directly.
    // Otherwise, find the nearest walkable position to "to".
	Vector2 goal;
	if(IsWorldPosWalkable(to))
	{
		goal = WorldPosToTexturePos(to);
	}
	else
	{
		// If "to" is not walkable, we need to find nearest walkable position as our goal.
		goal = FindNearestWalkableTexturePosToWorldPos(to);
	}
    
	// Pick start position. If "from" is walkable, we can use it directly.
	Vector2 start;
	if(IsWorldPosWalkable(from))
	{
		start = WorldPosToTexturePos(from);
	}
	else
	{
		// If "from" is not walkable, find nearest walkable and use that instead.
		// Walker will move from current (unwalkable) position to this position when it starts walking.
		start = FindNearestWalkableTexturePosToWorldPos(from);
	}
    
    // Use BFS to find a path.
    // I found that BFS resulted in way better performance than A*, and similar results.
    // In hindsight, I think this is because: a) the graph has a ton of nodes, and b) edges between nodes _aren't really_ weighted.
    std::vector<Vector2> path;
    if(FindPathBFS(start, goal, path))
    // if(FindPathAStar(start, goal, path))
    {
        // Ok, we found a path, but it's probably too close to walls and such.
        // So, let's try to condition it a little bit to fix that.
        for(int i = path.size() - 1; i >= 0; --i)
        {
            // We should ignore the first few nodes and last few nodes when doing conditioning.
            // This is because start/end nodes are _exact_ destinations (i.e. character starts here and wants to get there - don't mess with it).
            const int kFuzzyIgnore = 4;
            if(i > path.size() - kFuzzyIgnore) { continue; }
            if(i < kFuzzyIgnore) { break; }

            // See if a neighbor is more walkable.
            int index = mTexture->GetPaletteIndex(path[i].x, path[i].y);
            while(index != 0)
            {
                if(mTexture->GetPaletteIndex(path[i].x + 1, path[i].y) < index)
                {
                    path[i].x += 1;
                }
                else if(mTexture->GetPaletteIndex(path[i].x - 1, path[i].y) < index)
                {
                    path[i].x -= 1;
                }
                else if(mTexture->GetPaletteIndex(path[i].x, path[i].y + 1) < index)
                {
                    path[i].y += 1;
                }
                else if(mTexture->GetPaletteIndex(path[i].x, path[i].y - 1) < index)
                {
                    path[i].y -= 1;
                }
                else
                {
                    // No neighbor is more walkable, so break out of this loop.
                    break;
                }
                index = mTexture->GetPaletteIndex(path[i].x, path[i].y);
            }
        }

        // Convert texture-space path to world-space path.
        for(auto& node : path)
        {
            outPath.push_back(TexturePosToWorldPos(node));
        }
        return true;
    }

    printf("Failed to find path!\n");
    return false;
}

Vector3 WalkerBoundary::FindNearestWalkablePosition(const Vector3& position) const
{
	// Easy case: the position provided is already walkable.
	if(IsWorldPosWalkable(position)) { return position; }
	
	// Find nearest walkable position on texture, convert to world space.
	Vector2 walkableTexturePos = FindNearestWalkableTexturePosToWorldPos(position);
	return TexturePosToWorldPos(walkableTexturePos);
}

void WalkerBoundary::SetRegionBlocked(int regionIndex, int regionBoundaryIndex, bool blocked)
{
    bool inRegion = false;
    for(int y = 0; y < mTexture->GetHeight(); ++y)
    {
        inRegion = false;
        for(int x = 0; x < mTexture->GetWidth(); ++x)
        {
            int index = mTexture->GetPaletteIndex(x, y);

            // Determine whether in or out of region.
            if(!inRegion && index == regionBoundaryIndex)
            {
                inRegion = true;
            }
            if(inRegion && index != regionBoundaryIndex && index != regionIndex)
            {
                inRegion = false;
            }

            // If in region and blocking, convert region's index to 255.
            // If in region and unblocking, convert black to region's index.
            if(inRegion && ((blocked && index == regionIndex) || (!blocked && index == 255)))
            {
                // If blocked, set to 255, which equates to "can't walk here."
                // If not blocked, set back to original region index.
                mTexture->SetPaletteIndex(x, y, blocked ? 255 : regionIndex);
            }
        }
    }
}

bool WalkerBoundary::IsWorldPosWalkable(const Vector3& worldPos) const
{
	// Convert to texture position and check that.
	return IsTexturePosWalkable(WorldPosToTexturePos(worldPos));
}

bool WalkerBoundary::IsTexturePosWalkable(const Vector2& texturePos) const
{
	// If no texture...can walk anywhere?
	if(mTexture == nullptr) { return true; }

    // Make sure the position isn't outside the texture - that is NOT walkable for sure.
    if(texturePos.x < 0 || texturePos.x >= mTexture->GetWidth()) { return false; }
    if(texturePos.y < 0 || texturePos.y >= mTexture->GetHeight()) { return false; }

    // Walker boundary data is defined in an indexed (palettized) texture.
    // Palette index 0 is walkable, with indexes 1-9 indicating less and less walkable areas.
    // Palette index 255 is "unwalkable" area.
    // Palette indexes 128-254 are for special regions.
	uint8 paletteIndex = mTexture->GetPaletteIndex(texturePos.x, texturePos.y);

	// If the palette index is not 255, you can walk there.
    // Also trying to disallow 9/8/7. These are all very close to the edge, and it looks weird if walker goes there (e.g. clipping into walls).
	return paletteIndex != 255 && paletteIndex != 9 && paletteIndex != 8 && paletteIndex != 7;
}

Vector2 WalkerBoundary::WorldPosToTexturePos(const Vector3& worldPos) const
{
	// If no texture, the end result is going to be zero.
	if(mTexture == nullptr) { return Vector2::Zero; }
    
	// Add walker boundary's world position offset.
	// This causes the position to be relative to the texture's origin (lower left) instead of the world origin.
	Vector2 texturePos;
	texturePos.x = worldPos.x + mOffset.x;
	texturePos.y = worldPos.z + mOffset.y;
	//std::cout << "Offset Pos: " << position << std::endl;
	
	// Divide position by walkable area size to get a normalized position within that area.
	// Hopefully 0-1, but could be outside those bounds. If so, not walkable.
	texturePos.x = texturePos.x / mSize.x;
	texturePos.y = texturePos.y / mSize.y;
	//std::cout << "Normalized Pos: " << position << std::endl;
	
	// Multiply by texture width/height to determine the pixel within the texture.
	texturePos.x = texturePos.x * mTexture->GetWidth();
	texturePos.y = texturePos.y * mTexture->GetHeight();
	//std::cout << "Pixel Pos: " << position << std::endl;
	
	// Need to flip Y because the calculated value is from lower-left of the walkable area.
	// But texture sample X/Y are from upper-left.
	texturePos.y = mTexture->GetHeight() - texturePos.y;
	
	// Texture positions are integers.
	texturePos.x = (int)texturePos.x;
	texturePos.y = (int)texturePos.y;
	return texturePos;
}

Vector3 WalkerBoundary::TexturePosToWorldPos(Vector2 texturePos) const
{
	// If no texture, the end result is going to be zero.
	if(mTexture == nullptr) { return Vector3::Zero; }
    
    // A texture pos actually correlates to the bottom-left corner of the pixel.
    // But we want center of pixel...so let's offset before the conversion!
    texturePos.x = texturePos.x + 0.5f;
    texturePos.y = texturePos.y + 0.5f;
    
	// Flip y because texture pos is from top-left, but we need lower-left for world pos conversion.
	texturePos.y = mTexture->GetHeight() - texturePos.y;
    
	// Divide by texture width/height to get normalized position within the texture (0-1).
	Vector3 worldPos;
	worldPos.x = texturePos.x / mTexture->GetWidth();
	worldPos.z = texturePos.y / mTexture->GetHeight();
	
	// Multiply by size to get unit in world space.
	worldPos.x = worldPos.x * mSize.x;
	worldPos.z = worldPos.z * mSize.y;
	
	// Subtract offset to go from "texture space" to "world space".
	worldPos.x = worldPos.x - mOffset.x;
	worldPos.z = worldPos.z - mOffset.y;
	return worldPos;
}

Vector2 WalkerBoundary::FindNearestWalkableTexturePosToWorldPos(const Vector3& worldPos) const
{
	// We need a texture.
	if(mTexture == nullptr) { return Vector2::Zero; }
	
	// If the passed in position is already walkable, just return that position in texture space.
	if(IsWorldPosWalkable(worldPos))
	{
		return WorldPosToTexturePos(worldPos);
	}
	
	// Convert target position to texture position.
	Vector2 targetTexturePos = WorldPosToTexturePos(worldPos);
	
	// Let's just brute force this for now - search O(n^2) for the nearest walkable position.
	// This can probably be more efficient based on whether target is to left/right/above/below/inside the texture.
	// But these walker boundary textures are really small, and this doesn't get called often, so this might work fine.
	Vector2 nearestWalkableTexturePos;
	float nearestDistanceSq = 9999.0f;
	for(int x = 0; x < mTexture->GetWidth(); ++x)
	{
		for(int y = 0; y < mTexture->GetHeight(); ++y)
		{
			Vector2 pos(x, y);
			if(IsTexturePosWalkable(pos))
			{
				float distSq = (pos - targetTexturePos).GetLengthSq();
				if(distSq < nearestDistanceSq)
				{
					nearestWalkableTexturePos = pos;
					nearestDistanceSq = distSq;
				}
			}
		}
	}
	return nearestWalkableTexturePos;
}

bool WalkerBoundary::FindPathBFS(const Vector2& start, const Vector2& goal, std::vector<Vector2>& outPath) const
{
    struct Node
    {
        Vector2 value;
        Node* parent;
        bool closed;
    };

    // Create set of nodes for searching.
    // Since a lot of texture pixels are unwalkable (in most cases), this does waste a bit of memory.
    // But it works fine (for the moment). Perhaps it can be optimized by allocating the memory one time rather than each time this function is called.
    int width = mTexture->GetWidth();
    int height = mTexture->GetHeight();
    int nodeCount = width * height;

    Node* nodes = new Node[nodeCount];
    memset(nodes, 0, nodeCount * sizeof(Node));
    for(int y = 0; y < height; ++y)
    {
        for(int x = 0; x < width; ++x)
        {
            nodes[y * width + x].value = Vector2(x, y);
        }
    }

    // The open set when doing the search.
    // This does some dynamic allocation that could maybe be optimized if needed.
    ResizableQueue<Node*> openSet(nodeCount);

    // Put start node on the open set, mark as closed/explored.
    Node* startNode = &nodes[static_cast<int>(start.y * width + start.x)];
    startNode->closed = true;
    openSet.Push(startNode);

    // Iterate until we either find the goal, or the open set is empty.
    while(!openSet.Empty())
    {
        // If we find the goal, we purposely don't pop the node off the open set.
        // This is used after the while-loop to check success/failure of the search.
        Node* current = openSet.Front();
        if(current->value == goal) { break; }

        // Create neighbors array - including diagonals!
        Vector2 neighbors[8];
        neighbors[0] = current->value + Vector2(0, 1);
        neighbors[1] = current->value + Vector2(0, -1);
        neighbors[2] = current->value + Vector2(1, 0);
        neighbors[3] = current->value + Vector2(-1, 0);

        neighbors[4] = current->value + Vector2(1, 1);
        neighbors[5] = current->value + Vector2(1, -1);
        neighbors[6] = current->value + Vector2(-1, 1);
        neighbors[7] = current->value + Vector2(-1, -1);

        // See if we should add neighbors to open set.
        for(auto& neighbor : neighbors)
        {
            // Ignore any neighbor that has pixel color black (not walkable).
            if(!IsTexturePosWalkable(neighbor))
            {
                continue;
            }

            // Ignore closed/explored neighbors.
            int nodeIndex = static_cast<int>(neighbor.y * width + neighbor.x);
            if(nodeIndex < 0 || nodeIndex >= width * height)
            {
                continue;
            }

            Node* neighborNode = &nodes[nodeIndex];
            if(neighborNode->closed) { continue; }

            // Add to open set.
            neighborNode->parent = current;
            neighborNode->closed = true;
            openSet.Push(neighborNode);
        }

        // Done with this node - remove from open set.
        openSet.Pop();
    }

    // If open set is empty, we did not find the goal. No path can be generated.
    if(openSet.Empty())
    {
        delete[] nodes;
        return false;
    }

    // Iterate back to start, pushing world position of each node onto our path.
    // This leaves the path with start node at back, goal node at front - caller can traverse back-to-front.
    Node* current = openSet.Front();
    while(current != nullptr)
    {
        outPath.push_back(current->value);
        current = current->parent;
    }

    // We found a path! Noice.
    delete[] nodes;
    return true;
}