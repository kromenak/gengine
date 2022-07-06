#include "DrivingScreenBlip.h"

DrivingScreenBlip::DrivingScreenBlip(const DrivingScreen::PathData& pathData) : Actor(Actor::TransformType::RectTransform),
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
            }
        }
    }
}

void DrivingScreenBlip::ClearPath()
{
    mPath.clear();
    mPathIndex = 0;
    mConnection = nullptr;
    mConnectionIndex = 0;
    mLoopPath = false;
    mFollowBlip = nullptr;
}

void DrivingScreenBlip::SkipToPathNode(const std::string& nodeName)
{
    for(int i = 0; i < mPath.size(); ++i)
    {
        if(StringUtil::EqualsIgnoreCase(mPath[i]->name, nodeName))
        {
            mPathIndex = i;
            SetMapPosition(mPath[i]->point);
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

    // Update blip to follow any path it is on.
    if(mPathIndex + 1 < mPath.size())
    {
        // Wait for enough time to pass before leaving the current spot.
        mPathFollowTimer += deltaTime;
        if(mPathFollowTimer >= kPathNodeWaitTime)
        {
            mPathFollowTimer = 0.0f;

            // At a node, need to follow a connection segment.
            if(mConnection == nullptr)
            {
                // When looping, we may set path index to -1 to represent "last node in path."
                int currIndex = mPathIndex == -1 ? mPath.size() - 1 : mPathIndex;

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

                    // If at the last node, but this is a looping path, set path index to -1 so we can catch this special case on next move.
                    if(mLoopPath && mPathIndex == mPath.size() - 1)
                    {
                        mPathIndex = -1;
                    }
                }
            }
        }
    }
    else if(!mPath.empty() && mPathCompleteCallback != nullptr) // Apparently at the end of the path.
    {
        mPathCompleteCallback();
        mPathCompleteCallback = nullptr;
    }

    // A follow blip overrides everything.
    if(mFollowBlip != nullptr)
    {
        mImage->GetRectTransform()->SetAnchoredPosition(mFollowBlip->mImage->GetRectTransform()->GetAnchoredPosition());
    }
}