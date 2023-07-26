#include "DrivingScreenBlip.h"

#include "StringUtil.h"

DrivingScreenBlip::DrivingScreenBlip(const DrivingScreen::PathData& pathData) : Actor(TransformType::RectTransform),
    mPathData(pathData)
{
    // Position from top-left corner of map.
    GetComponent<RectTransform>()->SetAnchor(0.0f, 1.0f);
}

void DrivingScreenBlip::AddPathNode(const std::string& nodeName)
{
    // Find node with this name and add it to path.
    for(auto& node : mPathData.nodes)
    {
        if(StringUtil::EqualsIgnoreCase(node.name, nodeName))
        {
            mPath.push_back(&node);

            // If this was the very first node added to the path, make sure blip is positioned there.
            if(mPath.size() == 1)
            {
                SetMapPosition(mPath.back()->point);

                // When starting a follow, the follower should share our location.
                mFollowPoint = mImage->GetRectTransform()->GetAnchoredPosition();
            }
        }
    }
}

void DrivingScreenBlip::ClearPath()
{
    // Just clear all path-related vars.
    mPath.clear();
    mPathIndex = 0;
    mConnection = nullptr;
    mConnectionIndex = 0;
    mLoopPath = false;
    mFollowBlip = nullptr;
}

void DrivingScreenBlip::SkipToPathNode(const std::string& nodeName)
{
    for(size_t i = 0; i < mPath.size(); ++i)
    {
        if(StringUtil::EqualsIgnoreCase(mPath[i]->name, nodeName))
        {
            mPathIndex = i;
            SetMapPosition(mPath[i]->point);

            // When starting a follow, the follower should share our location.
            mFollowPoint = mImage->GetRectTransform()->GetAnchoredPosition();
            break;
        }
    }
}

void DrivingScreenBlip::SetMapPosition(const std::string& nodeName)
{
    // Find node with this name and add it to path.
    for(auto& node : mPathData.nodes)
    {
        if(StringUtil::EqualsIgnoreCase(node.name, nodeName))
        {
            SetMapPosition(node.point);
        }
    }
}

void DrivingScreenBlip::SetMapPosition(const Vector2& position)
{
    // Update follow point to be our current point. This causes follower to appear one point behind us on map.
    mFollowPoint = mImage->GetRectTransform()->GetAnchoredPosition();

    // Move to new point.
    mImage->GetRectTransform()->SetAnchoredPosition(position.x * mMapScale.x, -position.y * mMapScale.y);
}

void DrivingScreenBlip::OnUpdate(float deltaTime)
{
    // Update blink behavior.
    if(mBlinkInterval > 0.0f)
    {
        mBlinkTimer += deltaTime;
        if(mBlinkTimer > mBlinkInterval)
        {
            mImage->SetEnabled(!mImage->IsEnabled());
            mBlinkTimer = 0.0f;
        }
    }
    else
    {
        mImage->SetEnabled(true);
    }

    // Update blip to follow any path it is on.
    UpdatePathing(deltaTime);
   
    // A follow blip overrides everything.
    if(mFollowBlip != nullptr)
    {
        mImage->GetRectTransform()->SetAnchoredPosition(mFollowBlip->mFollowPoint);
    }
}

void DrivingScreenBlip::UpdatePathing(float deltaTime)
{
    // No path to follow...
    if(mPath.empty()) { return; }

    // We are at the end  of the path!
    if(mPathIndex == mPath.size() - 1)
    {
        // If looping, set path index to -1. The logic below detects this and loops the path correctly.
        if(mLoopPath)
        {
            mPathIndex = -1;
        }
        else
        {
            // Otherwise, we're not looping - we're at our destination.
            // Have any follower share our position in this case.
            mFollowPoint = mImage->GetRectTransform()->GetAnchoredPosition();

            // Execute path complete callback.
            if(mPathCompleteCallback != nullptr)
            {
                mPathCompleteCallback();
                mPathCompleteCallback = nullptr;
            }
            return;
        }
    }

    // Otherwise, path is not empty, and we're not at the end yet!
    mPathFollowTimer += deltaTime;
    if(mPathFollowTimer >= kPathNodeWaitTime)
    {
        // Reset timer - this technically can lose fractions of a second over time, but I think it gives the correct effect.
        mPathFollowTimer = 0.0f;

        // When we're at a node, our connection will be null.
        if(mConnection == nullptr)
        {
            // When looping, we may set path index to -1 to represent "last node in path."
            int currIndex = (mPathIndex == -1) ? mPath.size() - 1 : mPathIndex;

            // Find connection of current node that goes to next node.
            for(auto& connection : mPath[currIndex]->connections)
            {
                if(connection.to == mPath[mPathIndex + 1])
                {
                    mConnection = &connection;
                    mConnectionIndex = 0;
                    break;
                }
            }

            // Move to first spot on that connection's segment.
            if(mConnection != nullptr && mConnection->segment != nullptr)
            {
                SetMapPosition(mConnection->GetPoint(mConnectionIndex));
            }
            else
            {
                // Annoyingly, some nodes (like IN3 to PL2) don't have valid connections/segments between them.
                // In this case, we'll just skip to the next path index I suppose.
                ++mPathIndex;
                SetMapPosition(mPath[mPathIndex]->point);
                mConnection = nullptr;
            }
        }
        else // On a connection's segment.
        {
            ++mConnectionIndex;
            if(mConnectionIndex < mConnection->segment->points.size())
            {
                // Move to the next point on the connection's segment.
                SetMapPosition(mConnection->GetPoint(mConnectionIndex));
            }
            else
            {
                // Ok, we are at the next path node, increment that.
                ++mPathIndex;
                SetMapPosition(mPath[mPathIndex]->point);
                mConnection = nullptr;
            }
        }
    }
}