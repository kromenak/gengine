#include "WalkerBoundary.h"

#include <queue>

#include "Actor.h"
#include "Debug.h"
#include "GMath.h"
#include "PersistState.h"
#include "ResizableQueue.h"
#include "Texture.h"
#include "Walker.h"

namespace
{
    void MoveToward(Vector2& current, const Vector2& end)
    {
        // Move current toward end, one unit at a time.
        if(current.x < end.x)
        {
            current.x += 1;
        }
        else if(current.x > end.x)
        {
            current.x -= 1;
        }
        if(current.y < end.y)
        {
            current.y += 1;
        }
        else if(current.y > end.y)
        {
            current.y -= 1;
        }
    }
}

bool WalkerBoundary::FindPath(const Vector3& fromWorldPos, const Vector3& toWorldPos, std::vector<Vector3>& outPath)
{
    // Make sure path vector is empty.
    outPath.clear();

    // Pick goal position. If "to" is walkable, we can use it directly.
    // Otherwise, find the nearest walkable position to "to".
    Vector2 goal;
    if(IsWorldPosWalkable(toWorldPos))
    {
        goal = WorldPosToTexturePos(toWorldPos);
    }
    else
    {
        // If "to" is not walkable, we need to find nearest walkable position as our goal.
        goal = FindNearestWalkableTexturePosToWorldPos(toWorldPos);
    }

    // Pick start position. If "from" is walkable, we can use it directly.
    Vector2 start;
    if(IsWorldPosWalkable(fromWorldPos))
    {
        start = WorldPosToTexturePos(fromWorldPos);
    }
    else
    {
        // If "from" is not walkable, find nearest walkable and use that instead.
        // Walker will move from current (unwalkable) position to this position when it starts walking.
        start = FindNearestWalkableTexturePosToWorldPos(fromWorldPos);
    }

    // Use BFS to find a path.
    // I have implementations of both BFS and A* below - I've consistently found BFS to have better performance and results than A*.
    // In hindsight, I think this is because: a) the graph has a ton of nodes, and b) edges between nodes _aren't really_ weighted.

    // The loop here is to try using sparser graphs (and save a lot of time) if we can.
    // In a complex scene with a large walker boundary texture, the number of grid nodes is very large (170k in one case).
    // Usually, the system can successfully find a path when skipping a lot of those nodes. But worst case, we can use all nodes.
    bool foundPath = false;
    std::vector<Vector2> path;
    while(!foundPath)
    {
        foundPath = FindPathBFS(start, goal, path, mPathfindingNodeSkip);
        if(!foundPath)
        {
            if(mPathfindingNodeSkip > 1)
            {
                printf("Failed to find path - trying again with higher fidelity\n");
                mPathfindingNodeSkip /= 2;
            }
            else
            {
                // If skip interval is already 1, we can't get any higher fidelity - the path just doesn't exist.
                break;
            }
        }
    }

    // If a path was generated, do some conditioning on it to make it look more intelligent.
    // Note that a path can be generated, even if "foundPath" is false. In that case, the path is a "best effort" to get close to the goal.
    if(!path.empty())
    {
        // Ok, we found a path, but it's probably too close to walls and such.
        // So, let's try to condition it a little bit to fix that.
        for(int i = path.size() - 1; i >= 0; --i)
        {
            // We should ignore the first few nodes and last few nodes when doing conditioning.
            // This is because start/end nodes are _exact_ destinations (i.e. character starts here and wants to get there - don't mess with it).
            const int kFuzzyIgnore = 1;
            if(i >= path.size() - kFuzzyIgnore) { continue; }
            if(i < kFuzzyIgnore) { break; }

            // Palette indexes on walker boundary textures provide some indication of how "walkable" the current position is.
            // Palette index 255 (black) is completely unwalkable, and should already have been discarded by the BFS.
            // Palette indexes 127 or less are walkable, but a lower index is preferrable if available.
            // Palette indexes 128 or greater are conditionally walkable (the game can turn them on or off). But if walkable, they should be considered walkable as-is (BFS figures that out).

            // ANYWAY, here's the idea: if the current index is less than 128, see if a neighbor is a lower palette index.
            // If so, we will want to walk there instead. If no, this is a less than ideal place to walk, but at least it is walkable.
            int index = mTexture->GetPixelPaletteIndex(path[i].x, path[i].y);
            if(index < 128)
            {
                while(true)
                {
                    // If any up/down/left/right has a lower palette index, go there!
                    if(mTexture->GetPixelPaletteIndex(path[i].x + 1, path[i].y) < index &&
                       IsTexturePosWalkable(Vector2(path[i].x + 1, path[i].y)))
                    {
                        path[i].x += 1;
                    }
                    else if(mTexture->GetPixelPaletteIndex(path[i].x - 1, path[i].y) < index &&
                            IsTexturePosWalkable(Vector2(path[i].x - 1, path[i].y)))
                    {
                        path[i].x -= 1;
                    }
                    else if(mTexture->GetPixelPaletteIndex(path[i].x, path[i].y + 1) < index &&
                            IsTexturePosWalkable(Vector2(path[i].x, path[i].y + 1)))
                    {
                        path[i].y += 1;
                    }
                    else if(mTexture->GetPixelPaletteIndex(path[i].x, path[i].y - 1) < index &&
                            IsTexturePosWalkable(Vector2(path[i].x, path[i].y - 1)))
                    {
                        path[i].y -= 1;
                    }
                    else if(mTexture->GetPixelPaletteIndex(path[i].x + 1, path[i].y + 1) < index &&
                            IsTexturePosWalkable(Vector2(path[i].x + 1, path[i].y + 1)))
                    {
                        path[i].x += 1;
                        path[i].y += 1;
                    }
                    else if(mTexture->GetPixelPaletteIndex(path[i].x + 1, path[i].y - 1) < index &&
                            IsTexturePosWalkable(Vector2(path[i].x + 1, path[i].y - 1)))
                    {
                        path[i].x += 1;
                        path[i].y -= 1;
                    }
                    else if(mTexture->GetPixelPaletteIndex(path[i].x - 1, path[i].y - 1) < index &&
                            IsTexturePosWalkable(Vector2(path[i].x - 1, path[i].y - 1)))
                    {
                        path[i].x -= 1;
                        path[i].y -= 1;
                    }
                    else if(mTexture->GetPixelPaletteIndex(path[i].x - 1, path[i].y + 1) < index &&
                            IsTexturePosWalkable(Vector2(path[i].x - 1, path[i].y + 1)))
                    {
                        path[i].x -= 1;
                        path[i].y += 1;
                    }
                    else
                    {
                        // No neighbor is more walkable, so break out of this loop.
                        break;
                    }

                    // Update index being considered for next run through loop.
                    index = mTexture->GetPixelPaletteIndex(path[i].x, path[i].y);

                    // If the palette index is below some threshold, we're in an "acceptably walkable" zone, so we can stop iterating.
                    if(index < 4)
                    {
                        break;
                    }
                }
            }
        }

        // This is a sort of "string pulling" algorithm:
        // Given three points, if we can walk directly between the first and third point, we can get rid of the second point.
        if(path.size() > 2)
        {
            int startNodeIndex = 0;
            int endNodeIndex = startNodeIndex + 2;
            int erasedNodeCount = 0;

            while(endNodeIndex < path.size())
            {
                Vector2 current = path[startNodeIndex];
                Vector2 end = path[endNodeIndex];

                bool canWalk = true;
                while(current != end)
                {
                    MoveToward(current, end);

                    if(!IsTexturePosWalkable(current))
                    {
                        canWalk = false;
                        break;
                    }
                    else
                    {
                        int paletteIndex = mTexture->GetPixelPaletteIndex(current.x, current.y);
                        if(paletteIndex > 6 && paletteIndex < 128)
                        {
                            canWalk = false;
                            break;
                        }
                    }
                }

                if(canWalk)
                {
                    // In this case, we erase the intermediate node.
                    // "endNodeIndex" stays the same, and we try again with the next three positions in the path.
                    path.erase(path.begin() + startNodeIndex + 1);
                    ++erasedNodeCount;
                }

                // In this case, there wasn't a straight line between start/end, so the middle node was important for the path.
                // Move up to that middle node, and do checks from there now.
                if(!canWalk)
                {
                    ++startNodeIndex;
                    endNodeIndex = startNodeIndex + 2;
                }
                else if(erasedNodeCount > 3) // don't erase TOO many points - set a limit before moving on
                {
                    ++startNodeIndex;
                    endNodeIndex = startNodeIndex + 2;
                    erasedNodeCount = 0;
                }
            }
        }

        // Convert texture-space path to world-space path.
        for(auto& node : path)
        {
            outPath.push_back(TexturePosToWorldPos(node));
        }
    }

    // Whether a path was generated or not, return whether we found a path.
    // The caller can decide if the "best effort" path is worth using at all, or if the walk should just be abandoned.
    return foundPath;
}

Vector3 WalkerBoundary::FindNearestWalkablePosition(const Vector3& worldPos) const
{
    // Easy case: the position provided is already walkable.
    if(IsWorldPosWalkable(worldPos)) { return worldPos; }

    // Find nearest walkable position on texture, convert to world space.
    Vector2 walkableTexturePos = FindNearestWalkableTexturePosToWorldPos(worldPos);
    return TexturePosToWorldPos(walkableTexturePos);
}

void WalkerBoundary::SetRegionBlocked(int regionIndex, int regionBoundaryIndex, bool blocked)
{
    if(blocked)
    {
        mUnwalkableRegions.insert(regionIndex);
        mUnwalkableRegions.insert(regionBoundaryIndex);
    }
    else
    {
        mUnwalkableRegions.erase(regionIndex);
        mUnwalkableRegions.erase(regionBoundaryIndex);
    }
}

int WalkerBoundary::GetRegionIndex(const Vector3& worldPos)
{
    return GetRegionForTexturePos(WorldPosToTexturePos(worldPos));
}

void WalkerBoundary::SetUnwalkableRect(const std::string& name, const Rect& worldRect)
{
    // If this name already exists, we'll update instead of add.
    int index = -1;
    for(size_t i = 0; i < mUnwalkableRects.size(); ++i)
    {
        if(StringUtil::EqualsIgnoreCase(mUnwalkableRects[i].first, name))
        {
            index = i;
            break;
        }
    }

    // Convert rect min/max to texture space.
    // The annoying thing here is that the rect's x/y correspond to x/z in world space.
    Vector2 worldMin = worldRect.GetMin();
    Vector2 worldMax = worldRect.GetMax();
    Vector2 textureMin = WorldPosToTexturePos(Vector3(worldMin.x, 0.0f, worldMin.y));
    Vector2 textureMax = WorldPosToTexturePos(Vector3(worldMax.x, 0.0f, worldMax.y));

    // Add or replace unwalkable rect.
    if(index == -1)
    {
        mUnwalkableRects.emplace_back(std::make_pair(name, Rect(textureMin, textureMax)));
    }
    else
    {
        mUnwalkableRects[index].second = Rect(textureMin, textureMax);
    }
}

void WalkerBoundary::ClearUnwalkableRect(const std::string& name)
{
    // Find and erase by name.
    for(size_t i = 0; i < mUnwalkableRects.size(); ++i)
    {
        if(StringUtil::EqualsIgnoreCase(mUnwalkableRects[i].first, name))
        {
            mUnwalkableRects.erase(mUnwalkableRects.begin() + i);
            return;
        }
    }
}

void WalkerBoundary::DrawUnwalkableAreas()
{
    // Draw visualization of rectangular areas that will be pathed around.
    for(auto& entry : mUnwalkableRects)
    {
        Vector3 worldMin = TexturePosToWorldPos(entry.second.GetMin());
        Vector3 worldMax = TexturePosToWorldPos(entry.second.GetMax());
        Debug::DrawRectXZ(Rect(Vector2(worldMin.x, worldMin.z), Vector2(worldMax.x, worldMax.z)), 15.0f, Color32::Orange);
    }

    // Draw visualizations of walkers who will be pathed around.
    for(Walker* walker : mWalkers)
    {
        Debug::DrawSphere(walker->GetOwner()->GetPosition(), 10.0f, Color32::Orange);
    }
}

void WalkerBoundary::AddWalker(Walker* walker)
{
    auto it = std::find(mWalkers.begin(), mWalkers.end(), walker);
    if(it == mWalkers.end())
    {
        mWalkers.push_back(walker);
    }
}

void WalkerBoundary::RemoveWalker(Walker * walker)
{
    auto it = std::find(mWalkers.begin(), mWalkers.end(), walker);
    if(it != mWalkers.end())
    {
        mWalkers.erase(it);
    }
}

void WalkerBoundary::OnPersist(PersistState& ps)
{
    ps.Xfer(PERSIST_VAR(mUnwalkableRegions));
    ps.Xfer(PERSIST_VAR(mUnwalkableRects));
}

bool WalkerBoundary::IsWorldPosWalkable(const Vector3& worldPos) const
{
    // Convert to texture position and check that.
    return IsTexturePosWalkable(WorldPosToTexturePos(worldPos));
}

bool WalkerBoundary::IsTexturePosWalkable(const Vector2& texturePos) const
{
    // Unwalkable if region associated with this texture pos is in the unwalkable regions set.
    if(mUnwalkableRegions.count(GetRegionForTexturePos(texturePos)) > 0)
    {
        return false;
    }

    // Also unwalkable if this position is inside an unwalkable rect.
    for(auto& unwalkableRect : mUnwalkableRects)
    {
        if(unwalkableRect.second.Contains(texturePos))
        {
            return false;
        }
    }

    // Also unwalkable if this position is too close to a walker in the scene.
    Vector3 worldPos = TexturePosToWorldPos(texturePos);
    for(Walker* walker : mWalkers)
    {
        // Make y-pos equal so that we only consider distance on the x/z plane.
        Vector3 walkerPos = walker->GetOwner()->GetPosition();
        walkerPos.y = worldPos.y;
        float distSq = (worldPos - walkerPos).GetLengthSq();

        // Each walker's radius is about 10 units. There are outliers (like Chicken or Demon), but this mostly works.
        // BUT we need to factor in the radius of this walker AND myself - so we actually use 20 here!
        //TODO: If the radius differed per walker, we'd want to query the walkers and sum the radii.
        const float kCombinedRadiiSq = 20.0f * 20.0f;
        if(distSq <= kCombinedRadiiSq)
        {
            return false;
        }
    }

    // Looks like it's walkable!
    return true;
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

int WalkerBoundary::GetRegionForTexturePos(const Vector2& texturePos) const
{
    // If no walker texture, return zero, which is always a walkable region.
    if(mTexture == nullptr) { return 0; }

    // It position is out of bounds, use unwalkable index 255.
    if(texturePos.x < 0 || texturePos.x >= mTexture->GetWidth()) { return 255; }
    if(texturePos.y < 0 || texturePos.y >= mTexture->GetHeight()) { return 255; }

    // The region is just the palette index.
    // Palette index 0 is walkable, with indexes 1-9 indicating less and less walkable areas.
    // Palette index 255 is "unwalkable" area.
    // Palette indexes 128-254 are for special regions.
    return mTexture->GetPixelPaletteIndex(texturePos.x, texturePos.y);
}

namespace
{
    // The nodes used to track state during pathfinding search.
    struct Node
    {
        // Index (in nodes list) of parent of this node.
        size_t parentIndex = 0;

        // Is this node closed/explored in the current search?
        bool closed = false;
    };
    std::vector<Node> nodes;

    // The open set when doing a pathfinding search.
    // Each element is an index into the nodes array.
    ResizableQueue<size_t> openSet;
}

bool WalkerBoundary::FindPathBFS(const Vector2& start, const Vector2& goal, std::vector<Vector2>& outPath, int nodeSkipInterval) const
{
    //TIMER_SCOPED("BFS");

    // Figure out how many nodes we need for the current walker boundary texture.
    if(mTexture == nullptr) { return false; }
    uint32_t width = mTexture->GetWidth();
    uint32_t height = mTexture->GetHeight();
    uint32_t nodeCount = width * height;
    if(nodeCount == 0) { return false; }

    // Make sure node set is the right size. When entering a new scene, a resize up or down will likely be needed.
    // Note that resizing doesn't ever reduce capacity, so this will eventually be the size of the largest texture in the current play session.
    if(nodes.size() != nodeCount)
    {
        nodes.resize(nodeCount);
    }

    // Regardless of whether a resize occurred, we need to reset the working variables in each node.
    memset(&nodes[0], 0, sizeof(nodes[0]) * nodes.size());

    // Make sure open set is empty.
    openSet.Clear();

    // Make sure out path is clear.
    outPath.clear();

    // What do we mean by "fidelity"?
    // These walker graphs are very dense (1 pixel equals one node). By skipping some pixels, we get a simpler graph, and a faster algorithm.
    // A fidelity of 2 only uses every other pixel, 3 only uses every third pixel, and so on.

    // Calculate start point index, ensuring it aligns with the desired graph fidelity.
    uint32_t startX = static_cast<uint32_t>(start.x);
    uint32_t startY = static_cast<uint32_t>(start.y);
    if(startX % nodeSkipInterval != 0)
    {
        startX = startX / nodeSkipInterval * nodeSkipInterval;
    }
    if(startY % nodeSkipInterval != 0)
    {
        startY = startY / nodeSkipInterval * nodeSkipInterval;
    }
    size_t startIndex = static_cast<size_t>(startY * width + startX);

    // Close start node, put it on the open set.
    nodes[startIndex].closed = true;
    openSet.Push(startIndex);

    // Cache goal index to quickly check if we reached the goal. Same idea with the fidelity here.
    uint32_t goalX = static_cast<uint32_t>(goal.x);
    uint32_t goalY = static_cast<uint32_t>(goal.y);
    if(goalX % nodeSkipInterval != 0)
    {
        goalX = goalX / nodeSkipInterval * nodeSkipInterval;
    }
    if(goalY % nodeSkipInterval != 0)
    {
        goalY = goalY / nodeSkipInterval * nodeSkipInterval;
    }
    size_t goalIndex = static_cast<size_t>(goalY * width + goalX);
    Vector2 goalValue(goalX, goalY);

    // If start and goal are the same point, we technically found a path.
    if(startIndex == goalIndex)
    {
        return true;
    }

    // As we do the BFS, keep track of which node from the open set comes closest to the goal node.
    // We'll use this as a backup for generating a path if the goal node is unreachable.
    size_t closestToGoalNodeIndex = nodes.size();
    float closestToGoalDistSq = 0.0f;

    // Iterate until we either find the goal, or the open set is empty.
    Vector2 neighbors[8];
    while(!openSet.Empty())
    {
        // If we find the goal, we purposely don't pop the node off the open set.
        // This is used after the while-loop to check success/failure of the search.
        size_t currentIndex = openSet.Front();
        if(currentIndex == goalIndex) { break; }

        // Create neighbors array - including diagonals!
        Vector2 currentValue(currentIndex % width, currentIndex / width);
        neighbors[0] = currentValue + Vector2(0, nodeSkipInterval);
        neighbors[1] = currentValue + Vector2(0, -nodeSkipInterval);
        neighbors[2] = currentValue + Vector2(nodeSkipInterval, 0);
        neighbors[3] = currentValue + Vector2(-nodeSkipInterval, 0);

        neighbors[4] = currentValue + Vector2(nodeSkipInterval, nodeSkipInterval);
        neighbors[5] = currentValue + Vector2(nodeSkipInterval, -nodeSkipInterval);
        neighbors[6] = currentValue + Vector2(-nodeSkipInterval, nodeSkipInterval);
        neighbors[7] = currentValue + Vector2(-nodeSkipInterval, -nodeSkipInterval);

        // If this node is closer to the goal than any node we've yet seen, save it as the closest.
        float distToGoalSq = (goalValue - currentValue).GetLengthSq();
        if(closestToGoalNodeIndex >= nodes.size() || distToGoalSq < closestToGoalDistSq)
        {
            closestToGoalDistSq = distToGoalSq;
            closestToGoalNodeIndex = currentIndex;
        }

        // See if we should add neighbors to open set.
        for(Vector2& neighbor : neighbors)
        {
            // Ignore any x/y that appears to be out of bounds.
            if(neighbor.x < 0 || neighbor.x >= width || neighbor.y < 0 || neighbor.y >= height)
            {
                continue;
            }

            // Ignore closed/explored neighbors.
            int neighborNodeIndex = static_cast<int>(neighbor.y * width + neighbor.x);
            Node& neighborNode = nodes[neighborNodeIndex];
            if(neighborNode.closed) { continue; }

            // Ignore any neighbor that is not walkable.
            // When pathing at higher skip intervals, we still need to check in-between nodes, in case they are unwalkable.
            bool walkable = true;
            Vector2 checkWalkableValue = currentValue;
            while(checkWalkableValue != neighbor)
            {
                MoveToward(checkWalkableValue, neighbor);
                if(!IsTexturePosWalkable(checkWalkableValue))
                {
                    walkable = false;
                    break;
                }
            }
            if(!walkable) { continue; }

            // Add to open set.
            neighborNode.parentIndex = currentIndex;
            neighborNode.closed = true;
            openSet.Push(neighborNodeIndex);
        }

        // Done with this node - remove from open set.
        openSet.Pop();
    }

    // If the open set is empty, it means we didn't find a path to the goal.
    // However, we can still provide an "as close as possible" path that attempts to get as close to the goal as possible.
    if(openSet.Empty())
    {
        // If no closest to goal node was identified (unlikely), then we really did find no path.
        // Return false with an empty path.
        if(closestToGoalNodeIndex >= nodes.size())
        {
            return false;
        }

        // Otherwise, generate a path from the closest node back to the start.
        size_t current = closestToGoalNodeIndex;
        while(current != startIndex)
        {
            outPath.push_back(Vector2(current % width, current / width));
            current = nodes[current].parentIndex;
        }
        outPath.push_back(start);

        // We still return false here (didn't find a path to the goal).
        // But the caller can choose to just use the provided "close as possible" path if it makes sense to do so.
        return false;
    }
    else // we found a path!
    {
        // Make sure the actual goal is the first point on the path.
        outPath.push_back(goal);

        // Iterate back to start, pushing world position of each node onto our path.
        // This leaves the path with start node at back, goal node at front - caller can traverse back-to-front.
        size_t current = nodes[openSet.Front()].parentIndex;
        while(current != startIndex)
        {
            outPath.push_back(Vector2(current % width, current / width));
            current = nodes[current].parentIndex;
        }

        // Make sure the actual start is the last point on the path.
        outPath.push_back(start);

        // We found a path! Noice.
        return true;
    }
}

namespace
{
    // A subclass of the built-in priority queue that provides a "clear" function, and orders elements appropriately for A*
    typedef std::pair<uint32_t, size_t> CostAndIndex;
    class AStarPriorityQueue : public std::priority_queue<CostAndIndex, std::vector<CostAndIndex>, std::greater<CostAndIndex>>
    {
    public:
        void clear()
        {
            this->c.clear();
        }
    };
    AStarPriorityQueue openSetAS;

    // Maps each node index to its parent.
    std::vector<size_t> parents;

    // The g(x) cost to each node, and the f(x) priority for each node.
    std::vector<uint32_t> g;
    std::vector<uint32_t> f;
}

bool WalkerBoundary::FindPathAStar(const Vector2& start, const Vector2& goal, std::vector<Vector2>& outPath) const
{
    //TIMER_SCOPED("A*");

    // Figure out how many nodes we need for the current walker boundary texture.
    uint32_t width = mTexture->GetWidth();
    uint32_t height = mTexture->GetHeight();
    uint32_t nodeCount = width * height;
    if(nodeCount == 0) { return false; }

    // Make sure node set is the right size. When entering a new scene, a resize up or down will likely be needed.
    // Note that resizing doesn't ever reduce capacity, so this will eventually be the size of the largest texture in the current play session.
    if(parents.size() != nodeCount)
    {
        parents.resize(nodeCount);
        g.resize(nodeCount);
        f.resize(nodeCount);
    }

    // Regardless of whether a resize occurred, we need to reset the working variables in each node.
    memset(&parents[0], 0, sizeof(parents[0]) * parents.size());
    memset(&g[0], 0, sizeof(g[0]) * g.size());
    memset(&f[0], 0, sizeof(f[0]) * f.size());

    // Make sure open set is empty.
    openSetAS.clear();

    // Cache goal index to quickly check if we reached the goal.
    size_t goalIndex = static_cast<size_t>(goal.y * width + goal.x);
    bool foundPath = false;

    // Add start node.
    size_t startIndex = static_cast<size_t>(start.y * width + start.x);
    openSetAS.emplace(0, startIndex);

    Vector2 neighbors[4];
    while(!openSetAS.empty())
    {
        uint32_t topPriority = openSetAS.top().first;
        size_t currentIndex = openSetAS.top().second;
        openSetAS.pop();

        // If the priority for this entry doesn't match the latest priority for that index, skip it.
        // This means we updated the priority of this node at some point, so this old entry is stale and can be ignored.
        if(topPriority != f[currentIndex])
        {
            continue;
        }

        // If we find the goal, break out of the loop.
        // This leaves the goal on the open set.
        if(currentIndex == goalIndex)
        {
            foundPath = true;
            break;
        }

        // Create neighbors array - including diagonals!
        Vector2 currentValue(currentIndex % width, currentIndex / width);
        neighbors[0] = currentValue + Vector2(0, 1);
        neighbors[1] = currentValue + Vector2(0, -1);
        neighbors[2] = currentValue + Vector2(1, 0);
        neighbors[3] = currentValue + Vector2(-1, 0);

        // See if we should add neighbors to open set.
        for(Vector2& neighbor : neighbors)
        {
            // Ignore any x/y that appears to be out of bounds.
            if(neighbor.x < 0 || neighbor.x >= width || neighbor.y < 0 || neighbor.y >= height)
            {
                continue;
            }

            // Ignore any neighbor that is not walkable.
            if(!IsTexturePosWalkable(neighbor))
            {
                continue;
            }

            // The new cost for this node is current node plus one (all edges are cost 1 in this graph).
            uint32_t newCost = g[currentIndex] + 1;

            // This node is in the closed set if it's "g" value has already been calculated.
            // One notable exception is the start index, which is always zero, but is also always in the closed set.
            int neighborIndex = static_cast<int>(neighbor.y * width + neighbor.x);
            bool inClosedSet = g[neighborIndex] > 0 || neighborIndex == startIndex;

            // We'll process this node if it's either a node we've never seen before, OR if the new cost is less than what we previously recorded.
            if(!inClosedSet || newCost < g[neighborIndex])
            {
                // Update to new cost.
                g[neighborIndex] = newCost;

                // Calculate a new f(x) - this is the nodes priority.
                uint32_t heuristic = static_cast<uint32_t>(Math::Abs(goal.x - neighbor.x) + Math::Abs(goal.y - neighbor.y));
                uint32_t priority = newCost + heuristic;
                f[neighborIndex] = priority;

                // Put in open set with this priority.
                // Note that duplicate entries are possible with this approach. There might be multiple open set entries for a single node with different priorities.
                // But we catch this at the top of the while loop, and ignore stale priorities.
                openSetAS.emplace(priority, neighborIndex);

                // Update parent of this neighbor to the current node.
                parents[neighborIndex] = currentIndex;
            }
        }
    }

    // If open set is empty, we did not find the goal. No path can be generated.
    if(!foundPath)
    {
        return false;
    }

    // Iterate back to start, pushing world position of each node onto our path.
    // This leaves the path with start node at back, goal node at front - caller can traverse back-to-front.
    size_t currentIndex = goalIndex;
    while(currentIndex != startIndex)
    {
        outPath.push_back(Vector2(currentIndex % width, currentIndex / width));
        currentIndex = parents[currentIndex];
    }
    outPath.push_back(start);

    // We found a path! Noice.
    return true;
}
