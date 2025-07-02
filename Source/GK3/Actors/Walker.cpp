#include "Walker.h"

#include "ActionManager.h"
#include "Actor.h"
#include "Animation.h"
#include "Animator.h"
#include "Camera.h"
#include "CharacterManager.h"
#include "Debug.h"
#include "Frustum.h"
#include "GKActor.h"
#include "Heading.h"
#include "Interpolate.h"
#include "LineSegment.h"
#include "MeshRenderer.h"
#include "Ray.h"
#include "Renderer.h"
#include "ReportManager.h"
#include "SceneManager.h"
#include "StringUtil.h"
#include "Vector3.h"
#include "WalkerBoundary.h"

//#define DEBUG_WALKER

TYPEINFO_INIT(Walker, Component, 7)
{
    TYPEINFO_VAR(Walker, VariableType::Float, mCurrentWalkActionTimer);
    TYPEINFO_VAR(Walker, VariableType::Vector3, mTurnToFaceDir);
    TYPEINFO_VAR(Walker, VariableType::Bool, mFromAutoscript);
    TYPEINFO_VAR(Walker, VariableType::Bool, mNeedContinueWalkAnim);
}

Walker::Walker(Actor* owner) : Component(owner),
    mGKOwner(static_cast<GKActor*>(owner))
{

}

void Walker::SetCharacterConfig(const CharacterConfig& characterConfig)
{
    mCharConfig = &characterConfig;
    SetWalkAnims(mCharConfig->walkStartAnim, mCharConfig->walkLoopAnim,
                 mCharConfig->walkStartTurnLeftAnim, mCharConfig->walkStartTurnRightAnim);
}

void Walker::SetWalkAnims(Animation* startAnim, Animation* loopAnim,
                          Animation* startTurnLeftAnim, Animation* startTurnRightAnim)
{
    mWalkStartAnim = startAnim;
    mWalkLoopAnim = loopAnim;
    mWalkStartTurnLeftAnim = startTurnLeftAnim;
    mWalkStartTurnRightAnim = startTurnRightAnim;
}

void Walker::WalkToBestEffort(const Vector3& position, const Heading& heading, const std::function<void()>& finishCallback)
{
    // Since this is "best effort", it's OK to walk and not exactly get to the desired position.
    // This version is used if a player requests a walk action, or if an action approach tells us to "walk near" some object.
    WalkToInternal(position, heading, finishCallback, false, false);
}

void Walker::WalkToExact(const Vector3& position, const Heading& heading, const std::function<void()>& finishCallback)
{
    // This is "exact", which means it will always leave the walker at the exact position/heading specified.
    // This version is used for specific ScenePositions, or when scripts demand a specific walk pos internally.
    WalkToInternal(position, heading, finishCallback, false, true);
}

void Walker::WalkToGas(const Vector3& position, const Heading& heading, const std::function<void()>& finishCallback)
{
    // We're going to assume that if an autoscript tells us to walk somewhere, they want it to be the exact position/heading!
    WalkToInternal(position, heading, finishCallback, true, true);
}

void Walker::WalkToSee(GKObject* target, const std::function<void()>& finishCallback)
{
    mWalkToSeeTarget = target;

    // Be sure to save finish callback in this case - it usually happens in walk to.
    mFinishedPathCallback = finishCallback;

    // Not from autoscript, for sure.
    mFromAutoscript = false;

    // Check whether thing is already in view.
    // If so, we don't even need to walk (but may need to turn-to-face).
    Vector3 facingDir;
    if(IsWalkToSeeTargetInView(facingDir))
    {
        WalkOp currentWalkOp = GetCurrentWalkOp();

        // Time to create a new walk plan.
        mWalkActions.clear();

        // We don't need the item to be *exactly* in front of our face.
        // We only need to turn if the item is fairly far from our current facing direction.
        float dot = Vector3::Dot(mGKOwner->GetForward(), facingDir);
        if(dot < 0.5f)
        {
            mTurnToFaceDir = facingDir;
            mWalkActions.push_back(WalkOp::TurnToFace);
        }

        // If we were walking, we no longer need to!
        // We should stop our walk, play walk end anim, and do the turn to face.
        if(currentWalkOp > WalkOp::None && currentWalkOp < WalkOp::FollowPathEnd)
        {
            StopAllWalkAnimations();
            mWalkActions.push_back(WalkOp::FollowPathEnd);
        }
        OutputWalkerPlan();

        // OK, walk sequence has been created - let's start doing it!
        if(currentWalkOp != GetCurrentWalkOp())
        {
            NextAction();
        }
        else if(GetCurrentWalkOp() == WalkOp::None)
        {
            OnWalkToFinished();
        }
    }
    else
    {
        // Specify a "dummy" heading here so that the "turn to face" action is put into the walk plan.
        // Later on, when the object comes into view, we'll replace this with the actual direction to turn.
        //
        // This walk doesn't have to get us to the exact destination - the goal is to just want near it so we can see it.
        WalkToInternal(target->GetAABB().GetCenter(), Heading::FromDegrees(0.0f), finishCallback, false, false);
    }
}

void Walker::WalkOutOfRegion(int regionIndex, const Vector3& exitPosition, const Heading& exitHeading, const std::function<void()>& finishCallback)
{
    // For now, we can only track one "exit region" request at a time. So clear other if set.
    if(mExitRegionCallback != nullptr)
    {
        std::function<void()> callback = mExitRegionCallback;
        mExitRegionCallback = nullptr;
        callback();
    }

    // If already not in region, we don't have anything to do.
    if(mWalkerBoundary == nullptr || mWalkerBoundary->GetRegionIndex(GetOwner()->GetPosition()) != regionIndex)
    {
        if(finishCallback != nullptr)
        {
            finishCallback();
        }
        return;
    }

    // Save exit region and callback.
    mExitRegionIndex = regionIndex;
    mExitRegionCallback = finishCallback;

    // Attempt to leave the region by moving towards the exit position/heading.
    // Even if already outside the region, we want to let this go for at least one frame. When Update is called, it'll get cleared.
    WalkToExact(exitPosition, exitHeading, nullptr);
}

void Walker::SkipToEnd(bool alsoSkipWalkEndAnim)
{
    // If not walking, or at the end of the walk, nothing to skip.
    WalkOp currentWalkOp = GetCurrentWalkOp();
    if(currentWalkOp == WalkOp::None)
    {
        return;
    }
    if(!alsoSkipWalkEndAnim && (currentWalkOp == WalkOp::FollowPathEnd || currentWalkOp == WalkOp::TurnToFace))
    {
        return;
    }

    // Stop all walk animations, since we're about to force the walk end anim to play.
    StopAllWalkAnimations();

    // Sample walk start anim to ensure 3d model is in a sane default state, facing straight ahead.
    gSceneManager.GetScene()->GetAnimator()->Sample(mWalkStartAnim, 0);

    // Warp actor to end of path.
    if(!mPath.empty())
    {
        // Make sure front of path is at floor height.
        mPath.front().y = gSceneManager.GetScene()->GetFloorY(mPath.front());

        // Start with sane defaults for warp position/heading.
        Vector3 warpPos = mPath.front();
        Vector3 warpDir;
        if(mWalkActions.front() == WalkOp::TurnToFace)
        {
            warpDir = mTurnToFaceDir;
        }
        else if(mPath.size() > 1)
        {
            warpDir = mPath.front() - mPath[1];
        }
        else
        {
            warpDir = mPath.front() - mGKOwner->GetPosition();
        }

        // If we have a "walk to see" target, we can't necessarily just skip to the end of the path.
        // We should skip to a position along the path where the target becomes visible to us.
        if(mWalkToSeeTarget != nullptr)
        {
            bool sawTargetAlongPath = false;
            for(int i = mPath.size() - 1; i > 0; --i)
            {
                // Make sure path node is at floor height.
                mPath[i].y = gSceneManager.GetScene()->GetFloorY(mPath[i]);

                // See if the walk to see target would be in view if the character's head was roughly at this position.
                Vector3 headPosAtNode = mPath[i] + Vector3::UnitY * mCharConfig->walkerHeight;
                if(IsWalkToSeeTargetInView(headPosAtNode, warpDir))
                {
                    warpPos = mPath[i];
                    sawTargetAlongPath = true;
                    break;
                }

                // For better fidelity, also check a few spots along the path from this node to next node.
                LineSegment ls(mPath[i], mPath[i - 1]);
                for(float t = 0.0f; t < 1.0f; t += 0.25f)
                {
                    Vector3 pointAlongLine = ls.GetPoint(t);
                    if(IsWalkToSeeTargetInView(pointAlongLine, warpDir))
                    {
                        warpPos = pointAlongLine;
                        sawTargetAlongPath = true;
                        break;
                    }
                }
                if(sawTargetAlongPath) { break; }
            }

            // If we crawled the entire path and never saw the target, we need a fallback.
            if(!sawTargetAlongPath)
            {
                warpDir = mWalkToSeeTarget->GetAABB().GetCenter() - mPath.front();
            }

            // "Walk to see" uses a final turn to face, so make sure that's set as well.
            mTurnToFaceDir = warpDir;
        }

        // Do the warp!
        mGKOwner->SetPosition(warpPos);
        mGKOwner->SetHeading(Heading::FromDirection(warpDir));
    }

    // Clear the path - we don't need to follow it anymore.
    mPath.clear();

    // If we also wanted to skip the end walk anim, we're done - clear all walk actions and fire callback.
    if(alsoSkipWalkEndAnim)
    {
        mWalkActions.clear();
        OnWalkToFinished();
    }
    else // even though we skipped, still want to play "end walk" anim.
    {
        // Remove all walk ops except for the "follow path end" op.
        bool poppedAction = false;
        while(!mWalkActions.empty() && mWalkActions.back() != WalkOp::FollowPathEnd)
        {
            mWalkActions.pop_back();
            poppedAction = true;
        }
        if(poppedAction)
        {
            NextAction();
        }
    }
}

void Walker::StopWalk()
{
    // This function is used when the walk should stop IMMEDIATELY - only used in rare circumstances where anims interrupt the walk.
    // Clear the path - we're not using it anymore.
    mPath.clear();

    // Clear walk actions - we're not walking anymore.
    mWalkActions.clear();
    mPrevWalkOp = WalkOp::None;

    // No more finish callback needed
    mFinishedPathCallback = nullptr;

    // Stop any walk anims in progress.
    StopAllWalkAnimations();
}

bool Walker::AtPosition(const Vector3& position, float maxDistance)
{
    Vector3 myPosition = mGKOwner->GetPosition();
    myPosition.y = position.y;
    float distSq = (myPosition - position).GetLengthSq();
    return distSq <= maxDistance * maxDistance;
}

bool Walker::IsWalkAnimation(VertexAnimation* vertexAnim) const
{
    if(mWalkStartAnim != nullptr && mWalkStartAnim->ContainsVertexAnimation(vertexAnim))
    {
        return true;
    }
    if(mWalkLoopAnim != nullptr && mWalkLoopAnim->ContainsVertexAnimation(vertexAnim))
    {
        return true;
    }
    if(mWalkStartTurnLeftAnim != nullptr && mWalkStartTurnLeftAnim->ContainsVertexAnimation(vertexAnim))
    {
        return true;
    }
    if(mWalkStartTurnRightAnim != nullptr && mWalkStartTurnRightAnim->ContainsVertexAnimation(vertexAnim))
    {
        return true;
    }
    return false;
}

void Walker::OnUpdate(float deltaTime)
{
    // Debug draw the path if desired.
    if(mPath.size() > 0 && Debug::GetFlag("ShowWalkerPaths"))
    {
        // Draw a white line from the actor to the point they are walking to.
        Debug::DrawLine(mGKOwner->GetPosition(), mPath.back(), Color32::White);
        Debug::DrawLine(mPath.back(), mPath.back() + Vector3::UnitY * 10.0f, Color32::White);

        // Draw an orange line between path points, and a vertical line at each path point.
        Vector3 prev = mPath.back();
        for(int i = static_cast<int>(mPath.size()) - 2; i >= 0; i--)
        {
            Debug::DrawLine(prev, mPath[i], Color32::Orange);
            Debug::DrawLine(mPath[i], mPath[i] + Vector3::UnitY * 10.0f, Color32::Orange);
            prev = mPath[i];
        }
    }

    // Process outstanding walk actions.
    WalkOp currentWalkOp = GetCurrentWalkOp();
    if(currentWalkOp != WalkOp::None)
    {
        // Kind of a HACK: if we're walking, and action manager is skipping, move to end of movement ASAP.
        // Without this, walks during fast-forwards can get stuck and cause the game to freeze.
        //TODO: *Probably* a better way to handle this, with a substantial refactor...
        if(gActionManager.IsSkippingCurrentAction())
        {
            SkipToEnd(true);
            return;
        }

        // Increase current action timer.
        mCurrentWalkActionTimer += deltaTime;

        // Stay on the ground.
        mGKOwner->SnapToFloor();

        // Handle the current walk action.
        if(currentWalkOp == WalkOp::FollowPathStart)
        {
            // Possible to finish path while still in the start phase, for very short paths.
            Vector3 facingDir;
            if(IsWalkToSeeTargetInView(facingDir))
            {
                // Make sure final action is a "turn to face" using this facing dir.
                mTurnToFaceDir = facingDir;
                PopAndNextAction();
            }
            else if(AdvancePath())
            {
                // In this case, we want to skip all follow path bits and go right to "follow path end".
                while(GetCurrentWalkOp() != WalkOp::FollowPathEnd)
                {
                    mWalkActions.pop_back();
                }
                NextAction();
            }
            else
            {
                Vector3 toNext = mPath.back() - GetOwner()->GetPosition();
                toNext.y = 0.0f;
                TurnToFace(deltaTime, toNext.Normalize(), GetWalkTurnSpeed(toNext));
            }
        }
        else if(currentWalkOp == WalkOp::FollowPathStartTurnLeft || currentWalkOp == WalkOp::FollowPathStartTurnRight)
        {
            Vector3 facingDir;
            if(IsWalkToSeeTargetInView(facingDir))
            {
                // Make sure final action is a "turn to face" using this facing dir.
                mTurnToFaceDir = facingDir;
                PopAndNextAction();
            }
            else if(!AdvancePath())
            {
                // HACK: Wait until the "turn" anim has the character put its foot down before starting to turn to the target.
                // About 0.66s seems to look OK, but could probably be finessed a bit.
                if(mCurrentWalkActionTimer > 0.66f)
                {
                    Vector3 toNext = mPath.back() - GetOwner()->GetPosition();
                    toNext.y = 0.0f;
                    TurnToFace(deltaTime, toNext.Normalize(), GetWalkTurnSpeed(toNext));
                }
            }
        }
        else if(currentWalkOp == WalkOp::FollowPath)
        {
            // If we have a "walk to see" target, check whether it has come into view.
            Vector3 facingDir;
            if(IsWalkToSeeTargetInView(facingDir))
            {
                // Make sure final action is a "turn to face" using this facing dir.
                mTurnToFaceDir = facingDir;
                PopAndNextAction();
            }
            else if(AdvancePath()) // Otherwise, see if finished following path.
            {
                // If we get here and we have a "walk to see" target, it means we got to end of path without ever actully seeing the thing!
                // So, force-set it to something reasonable!
                if(mWalkToSeeTarget != nullptr)
                {
                    Vector3 dir = mWalkToSeeTarget->GetPosition() - mGKOwner->GetPosition();
                    dir.y = 0.0f;
                    mTurnToFaceDir = Vector3::Normalize(dir);
                }

                // Path is finished - move on to next step in sequence.
                PopAndNextAction();
            }
            else // Otherwise, just keep on following that path!
            {
                // If walk anim has ended, we need to start it again.
                if(mNeedContinueWalkAnim)
                {
                    mNeedContinueWalkAnim = false;

                    AnimParams animParams;
                    animParams.animation = mWalkLoopAnim;
                    animParams.allowMove = true;
                    animParams.parent = mGKOwner->GetMeshRenderer()->GetOwner();
                    animParams.fromAutoScript = mFromAutoscript;
                    animParams.finishCallback = std::bind(&Walker::OnWalkAnimFinished, this);
                    gSceneManager.GetScene()->GetAnimator()->Start(animParams);
                }

                // Still following path - turn to face next node in path.
                Vector3 toNext = mPath.back() - GetOwner()->GetPosition();
                toNext.y = 0.0f;
                TurnToFace(deltaTime, toNext.Normalize(), GetWalkTurnSpeed(toNext));
            }
        }
        else if(currentWalkOp == WalkOp::FollowPathEnd)
        {
            // The walker is allowed to start any "turn to face" while playing the stop walking anim.
            if(mWalkActions.front() == WalkOp::TurnToFace)
            {
                TurnToFace(deltaTime, mTurnToFaceDir, kTurnSpeed);
            }
        }
        else if(currentWalkOp == WalkOp::TurnToFace)
        {
            if(TurnToFace(deltaTime, mTurnToFaceDir, kTurnSpeed))
            {
                #if defined(DEBUG_WALKER)
                std::cout << "Finished turning towards " << mTurnToFaceDir << std::endl;
                #endif

                // Done turning to face, do next action in sequence.
                PopAndNextAction();
            }
        }
    }

    // Check if we exited any desired region.
    if(mExitRegionIndex >= 0)
    {
        if(mExitRegionCallback == nullptr)
        {
            mExitRegionIndex = -1;
        }
        else if(mWalkerBoundary == nullptr || mWalkerBoundary->GetRegionIndex(GetOwner()->GetPosition()) != mExitRegionIndex)
        {
            mExitRegionIndex = -1;

            std::function<void()> callback = mExitRegionCallback;
            mExitRegionCallback = nullptr;
            callback();
        }
    }
}

void Walker::WalkToInternal(const Vector3& position, const Heading& heading, const std::function<void()>& finishCallback, bool fromAutoscript, bool mustReachDestination)
{
    // Save if from autoscript.
    mFromAutoscript = fromAutoscript;

    // Save finish callback.
    mFinishedPathCallback = finishCallback;

    // Time to create a new walk plan.
    WalkOp currentWalkOp = GetCurrentWalkOp();
    mWalkActions.clear();

    // If action skipping, we don't need to find a path or do anything - just put the walker directly at the desired position/heading!
    if(gActionManager.IsSkippingCurrentAction())
    {
        StopAllWalkAnimations();
        mGKOwner->SetPosition(position);
        if(heading.IsValid())
        {
            mGKOwner->SetHeading(heading);
        }
        if(finishCallback != nullptr)
        {
            finishCallback();
        }
        return;
    }

    // If heading is specified, save "turn to face" action.
    if(heading.IsValid())
    {
        mTurnToFaceDir = heading.ToDirection();
        mWalkActions.push_back(WalkOp::TurnToFace);
    }

    // Make sure the passed in position is *actually* the position we will walk to.
    // Sometimes the position given is under the floor or floating in the air - ground it.
    Vector3 walkPosition = position;
    walkPosition.y = gSceneManager.GetScene()->GetFloorY(position);

    // Do we need to walk?
    if(!AtPosition(walkPosition))
    {
        mPath.clear();

        // Attempt to find a path between current position and walk position.
        Vector3 startPos = GetOwner()->GetPosition();
        Vector3 endPos = walkPosition;
        if(mWalkerBoundary != nullptr)
        {
            mWalkerBoundary->FindPath(startPos, endPos, mPath);
        }

        // Whether a path was found or not actually isn't that important here - even when a path isn't found, mPath contains a "best effort" to get close to the goal.
        // What IS important is whether we MUST reach the goal or if "best effort" is good enough.
        //
        // If it's not important to reach our destination exactly, then there's nothing more to do - we already have a "best effort" path.
        // If it is important though, we must ensure the path actually HAS the destination position present!
        if(mustReachDestination)
        {
            // If the path is empty, and we must reach our destination, we've just gotta go straight there - that's the best we can do at this point.
            if(mPath.empty())
            {
                mPath.push_back(endPos);
                mPath.push_back(startPos);
            }
            else
            {
                // If a path exists, whether it's a full path or "best effort", we should make 100% sure the end pos is at the end of the path.
                // The walker boundary goal position is always a "walkable" position on the grid, but the end pos can be something off the grid in some cases.
                mPath.insert(mPath.begin(), endPos);
            }
        }

        // Make sure first node is at the right y-pos.
        // Also put all other nodes in the path at the same y-pos for starters.
        UpdateNextNodesYPos();
        for(int i = 0; i < mPath.size(); ++i)
        {
            mPath[i].y = mPath.back().y;
        }

        // Attempt to shorten the path, if applicable, to speed up the walk process.
        bool shortened = SkipPathNodesOutsideFrustum();

        // Again, since the skip path nodes code may have removed some nodes.
        UpdateNextNodesYPos();

        // There's a small chance the path will be empty at this point - if so, we don't walk anywhere.
        if(!mPath.empty())
        {
            // Cache final position for later use.
            mFinalPosition = mPath.front();
            mFinalPosition.y = gSceneManager.GetScene()->GetFloorY(mFinalPosition);

            /*
            Debug::DrawLine(mPath[0], mPath[0] + Vector3::UnitY * 100.0f, Color32::Orange, 10.0f);
            for(int i = 1; i < mPath.size(); ++i)
            {
                Debug::DrawLine(mPath[i - 1], mPath[i], Color32::Red, 10.0f);
                Debug::DrawLine(mPath[i], mPath[i] + Vector3::UnitY * 100.0f, Color32::Orange, 10.0f);
            }
            */

            // We will walk, so there's always a "walk end" action.
            mWalkActions.push_back(WalkOp::FollowPathEnd);

            // We need an action to follow the path.
            mWalkActions.push_back(WalkOp::FollowPath);

            // We need a start walk, but NOT if the path was shortened.
            // In that case, the walker warps to the nearest offscreen position, so the start walk can be skipped.
            if(!shortened)
            {
                // Need to decide which start anim to play.
                WalkOp startOp = WalkOp::FollowPathStart;

                bool hasTurnToStartAnims = mWalkStartTurnLeftAnim != nullptr && mWalkStartTurnRightAnim != nullptr;
                if(hasTurnToStartAnims)
                {
                    Vector3 currToGoal = (mPath.front() - GetOwner()->GetPosition());
                    if(currToGoal.GetLengthSq() > 30.0f * 30.0f)
                    {
                        Vector3 toGoal = currToGoal.Normalize();
                        if(Vector3::Dot(GetOwner()->GetForward(), toGoal) <= 0.0f)
                        {
                            // For initial direction to turn, let's use the goal node direction.
                            // Just thinking about the real world...you usually turn towards your goal, right?
                            Vector3 cross = Vector3::Cross(GetOwner()->GetForward(), toGoal);
                            if(cross.y > 0)
                            {
                                startOp = WalkOp::FollowPathStartTurnRight;
                            }
                            else
                            {
                                startOp = WalkOp::FollowPathStartTurnLeft;
                            }
                        }
                    }
                }

                // Ok, we chose a start walk op.
                mWalkActions.push_back(startOp);
            }
        }
    }

    // OK, so full walk sequence is created.
    // However, we need to account for starting a new walk sequence while a previous one was already in progress!
    bool doNextAction = true;
    {
        // We are already doing a start anim, so we can skip doing one now.
        // When the current walk start anim finishes, it will continue the walk with the new path.
        if(currentWalkOp >= WalkOp::FollowPathStart && currentWalkOp <= WalkOp::FollowPathStartTurnRight)
        {
            doNextAction = false;
        }

        // We are already walking.
        // We can completely skip the start anims, and just wait for the current walk anim to finish.
        if(currentWalkOp == WalkOp::FollowPath)
        {
            while(GetCurrentWalkOp() < WalkOp::FollowPath && GetCurrentWalkOp() != WalkOp::None)
            {
                mWalkActions.pop_back();
            }
            doNextAction = false;
        }

        // We are ending a previous walk.
        // Let's just wait for the walk end anim to finish, and then it can kick off the walk start action in its callback.
        if(currentWalkOp == WalkOp::FollowPathEnd)
        {
            doNextAction = false;
        }

        // Here, the old walk op was either "turn to face" or "none".
        // In both cases, we don't really have to do anything. But we do want to play the next action.
    }
    OutputWalkerPlan();

    // OK, walk sequence has been created - let's start doing it!
    //std::cout << "Walk Sequence Begin" << std::endl;
    if(doNextAction)
    {
        NextAction();
    }
}

void Walker::PopAndNextAction()
{
    // Pop action from sequence.
    if(mWalkActions.size() > 0)
    {
        mPrevWalkOp = mWalkActions.back();
        mWalkActions.pop_back();

        #if defined(DEBUG_WALKER)
        std::cout << "Popped walk action." << std::endl;
        #endif
    }

    // Go to next action.
    NextAction();
}

void Walker::NextAction()
{
    mCurrentWalkActionTimer = 0.0f;

    // Do next action in sequence, if any.
    WalkOp currentWalkOp = GetCurrentWalkOp();
    if(currentWalkOp == WalkOp::FollowPathStart)
    {
        #if defined(DEBUG_WALKER)
        std::cout << "Follow Path Start" << std::endl;
        #endif

        AnimParams animParams;
        animParams.animation = mWalkStartAnim;
        animParams.allowMove = true;
        animParams.parent = mGKOwner->GetMeshRenderer()->GetOwner();
        animParams.fromAutoScript = mFromAutoscript;
        gSceneManager.GetScene()->GetAnimator()->Start(animParams, [this](){
            if(!mWalkActions.empty() && mWalkActions.back() <= WalkOp::FollowPathStartTurnRight)
            {
                PopAndNextAction();
            }
        });
    }
    else if(currentWalkOp == WalkOp::FollowPathStartTurnLeft)
    {
        #if defined(DEBUG_WALKER)
        std::cout << "Follow Path Start (Turn Left)" << std::endl;
        #endif

        AnimParams animParams;
        animParams.animation = mWalkStartTurnLeftAnim;
        animParams.allowMove = true;
        animParams.parent = mGKOwner->GetMeshRenderer()->GetOwner();
        animParams.fromAutoScript = mFromAutoscript;
        gSceneManager.GetScene()->GetAnimator()->Start(animParams, [this](){
            if(!mWalkActions.empty() && mWalkActions.back() <= WalkOp::FollowPathStartTurnRight)
            {
                PopAndNextAction();
            }
        });
    }
    else if(currentWalkOp == WalkOp::FollowPathStartTurnRight)
    {
        #if defined(DEBUG_WALKER)
        std::cout << "Follow Path Start (Turn Right)" << std::endl;
        #endif

        AnimParams animParams;
        animParams.animation = mWalkStartTurnRightAnim;
        animParams.allowMove = true;
        animParams.parent = mGKOwner->GetMeshRenderer()->GetOwner();
        animParams.fromAutoScript = mFromAutoscript;
        gSceneManager.GetScene()->GetAnimator()->Start(animParams, [this](){
            if(!mWalkActions.empty() && mWalkActions.back() <= WalkOp::FollowPathStartTurnRight)
            {
                PopAndNextAction();
            }
        });
    }
    else if(currentWalkOp == WalkOp::FollowPath)
    {
        #if defined(DEBUG_WALKER)
        std::cout << "Follow Path" << std::endl;
        #endif

        AnimParams animParams;
        animParams.animation = mWalkLoopAnim;
        animParams.allowMove = true;
        animParams.parent = mGKOwner->GetMeshRenderer()->GetOwner();
        animParams.fromAutoScript = mFromAutoscript;
        animParams.finishCallback = std::bind(&Walker::OnWalkAnimFinished, this);

        //TODO: Trying to make the transition from turn-right to walk anim more smooth, but this is probably not right.
        if(mPrevWalkOp == WalkOp::FollowPathStartTurnRight)
        {
            animParams.startFrame = 10;
        }

        gSceneManager.GetScene()->GetAnimator()->Start(animParams);
    }
    else if(currentWalkOp == WalkOp::FollowPathEnd)
    {
        #if defined(DEBUG_WALKER)
        std::cout << "Follow Path End" << std::endl;
        #endif
        gSceneManager.GetScene()->GetAnimator()->Stop(mWalkStartAnim);
        gSceneManager.GetScene()->GetAnimator()->Stop(mWalkLoopAnim);


        // While testing the original game, I noticed that "walk end" anims don't seem to actually ever be used!
        // For example, if you insert a "scene texture swap" anim node into the walk end anim, it never happens. But it will if you put it in walk start/continue anims.
        // So, perhaps they intended to use "walk end" anims, but then decided not to for some reason?
        /*
        AnimParams animParams;
        animParams.animation = mCharConfig->walkStopAnim;
        animParams.allowMove = true;
        animParams.parent = mGKOwner->GetMeshRenderer()->GetOwner();
        animParams.fromAutoScript = mFromAutoscript;
        animParams.finishCallback = std::bind(&Walker::PopAndNextAction, this);
        gSceneManager.GetScene()->GetAnimator()->Start(animParams);
        */

        // If I had to guess, why "walk end" anims weren't used: it's too hard to 100% ensure the walker ends up at the correct final position!
        // To absolutely ensure that, let's set the owner to the final position just to be sure (except for when we don't care about exact end position, such as a walk-to-see situation).
        if(mWalkToSeeTarget == nullptr)
        {
            mGKOwner->SetPosition(mFinalPosition);
        }

        // Sampling the walk start anim ensures that the walker is in a default pose, looking straight ahead, before any "turn to face" logic occurs.
        // Without this, the "turn to face" logic might happen when the walker was in the middle of a long stride, and the turn behavior would look all wrong.
        gSceneManager.GetScene()->GetAnimator()->Sample(mWalkStartAnim, 0);

        // No anim to play, so just move right on to the next walk action.
        PopAndNextAction();
    }
    else if(currentWalkOp == WalkOp::TurnToFace)
    {
        #if defined(DEBUG_WALKER)
        std::cout << "Turn To Face " << mTurnToFaceDir << std::endl;
        #endif
        // Handled in Update
    }
    else // WalkOp::None
    {
        // No actions left in sequence => walk is finished.
        OnWalkToFinished();
    }
}

void Walker::OnWalkAnimFinished()
{
    // Set flag so that we continue the walk anim during the next update loop.
    // A flag-based system is needed (rather than just continuing right here) to avoid infinite loops with high delta time values!
    mNeedContinueWalkAnim = true;
}

void Walker::OnWalkToFinished()
{
    #if defined(DEBUG_WALKER)
    std::cout << "Walk Sequence Done!" << std::endl;
    #endif

    // No more path.
    mPath.clear();

    // No more target.
    mWalkToSeeTarget = nullptr;

    // Idle fidget should restart when walk finishes.
    // But do this BEFORE callback, as callback may trigger anims or whatnot that want to stop fidgets.
    if(!mFromAutoscript)
    {
        mGKOwner->StartFidget(GKActor::FidgetType::Idle);
    }

    // Call finished callback.
    if(mFinishedPathCallback != nullptr)
    {
        std::function<void()> callback = mFinishedPathCallback;
        mFinishedPathCallback = nullptr;
        callback();
    }
}

bool Walker::IsWalkToSeeTargetInView(Vector3& outTurnToFaceDir) const
{
    return IsWalkToSeeTargetInView(mGKOwner->GetHeadPosition(), outTurnToFaceDir);
}

bool Walker::IsWalkToSeeTargetInView(const Vector3& headPosition, Vector3& outTurnToFaceDir) const
{
    // Not in view if it doesn't exist!
    if(mWalkToSeeTarget == nullptr) { return false; }

    //TODO: Could maybe cache this AABB to reduce per-frame computation.
    AABB targetAABB = mWalkToSeeTarget->GetAABB();
    //Debug::DrawAABB(targetAABB, Color32::Orange, 5.0f);

    // To see if the target is in view, we need to cast some rays from our head to the target's AABB.
    // But a single ray at the center of the AABB might not do the trick - we need to cast a few.
    // For example, if the target is standing behind a desk, a ray to the center will likely not hit, but a ray to the top would.
    Ray rays[] = {
        Ray(headPosition, (targetAABB.GetMax() - headPosition).Normalize()),
        Ray(headPosition, (targetAABB.GetCenter() - headPosition).Normalize()),
        Ray(headPosition, (targetAABB.GetMin() - headPosition).Normalize())
    };
    for(Ray& ray : rays)
    {
        //Debug::DrawLine(headPosition, headPosition + ray.direction * 100.0f, Color32::Red);

        // Cast a ray from our head (ignoring ourself).
        GKObject* obj = static_cast<GKObject*>(mGKOwner);
        SceneCastResult result = gSceneManager.GetScene()->RaycastAABBs(ray, &obj, 1);

        // If hit the target with the ray, it must be in view.
        if(StringUtil::EqualsIgnoreCase(result.hitInfo.name, mWalkToSeeTarget->GetNoun()) ||
           StringUtil::EqualsIgnoreCase(result.hitInfo.name, mWalkToSeeTarget->GetName()) ||
           (result.hitObject != nullptr && StringUtil::EqualsIgnoreCase(result.hitObject->GetNoun(), mWalkToSeeTarget->GetNoun())))
        {
            // Regardless of what ray hit (min/center/max), we always want to look at the center.
            outTurnToFaceDir = (targetAABB.GetCenter() - headPosition);
            outTurnToFaceDir.y = 0.0f;
            //Debug::DrawLine(headPosition, headPosition + outTurnToFaceDir.Normalize() * 100.0f, Color32::Green, 30.0f);
            return true;
        }
        /*
        else
        {
            result = gSceneManager.GetScene()->Raycast(ray, false, &obj, 1);
            if(StringUtil::EqualsIgnoreCase(result.hitInfo.name, mWalkToSeeTarget->GetNoun()) ||
               StringUtil::EqualsIgnoreCase(result.hitInfo.name, mWalkToSeeTarget->GetName()) ||
               (result.hitObject != nullptr && StringUtil::EqualsIgnoreCase(result.hitObject->GetNoun(), mWalkToSeeTarget->GetNoun())))
            {
                // Convert ray direction to a "facing" direction,
                dir.y = 0.0f;
                outTurnToFaceDir = dir.Normalize();
                return true;
            }
        }
        */
    }

    // If execution gets here, the walk to see target doesn't seem to be in view.
    return false;
}

bool Walker::SkipPathNodesOutsideFrustum()
{
    // Ok, we have a path (in mPath) that we can follow.
    // But if 90% of the path is behind the camera, the walker can just skip to the node right before the camera and walk from there!

    // Skipping path nodes outside the view frustum is mainly meant for user-initiated walk actions.
    // Don't do this for AI walkers (autoscript), if this isn't the Ego, or if we're in a cutscene/action.
    if(mFromAutoscript || mGKOwner != gSceneManager.GetScene()->GetEgo() || mPath.empty())
    {
        //printf("Early out skip path nodes - not ego, from autoscript, or path is empty.\n");
        return false;
    }
    //printf("Path size is %zu.\n", mPath.size());

    // Get the camera view frustum.
    Camera* camera = gRenderer.GetCamera();
    Frustum frustum = camera->GetWorldSpaceViewFrustum();

    // Find the first node on the path that is inside the view frustum.
    int firstInFrustumIndex = -1;
    for(int i = mPath.size() - 1; i >= 0; --i)
    {
        // In addition to the path node itself, we also need to check the expected HEAD position for the character.
        // Even if a node is off-screen, if the character's head at that position IS on-screen, we can't skip to it.
        Vector3 headPosAtNode = mPath[i] + Vector3::UnitY * mCharConfig->walkerHeight;
        if(frustum.ContainsPoint(mPath[i]) || frustum.ContainsPoint(headPosAtNode))
        {
            firstInFrustumIndex = i;
            break;
        }
    }
    //printf("First path node in frustum is %d\n", firstInFrustumIndex);

    // If the entire path is inside the frustum, we can't skip anything - early out.
    if(firstInFrustumIndex == mPath.size() - 1)
    {
        //printf("Early out skip path nodes - the first path node is in the frustum, so nothing can be skipped.\n");
        return false;
    }

    // We now know the first node index in the frustum. From here, there are two options:
    // 1) No node is in the view frustum (index will be -1). In this case, we can teleport directly to the final node.
    // 2) Some node is in the view frustum, but we can't teleport directly there - we must go to one *BEFORE* to avoid character popping onto screen.
    int lastNeededNodeIndex = firstInFrustumIndex == -1 ? 0 : firstInFrustumIndex + 1;

    // Remove all nodes that we don't need from the path.
    Vector3 dirFromLastPoppedNode;
    for(int i = mPath.size() - 1; i > lastNeededNodeIndex; --i)
    {
        Vector3 currentPathPos = mPath.back();
        Vector3 nextPathPos = mPath[mPath.size() - 2];
        dirFromLastPoppedNode = nextPathPos - currentPathPos;

        //printf("Removed path node %d\n", i);
        mPath.pop_back();
    }

    // Calculate skip heading.
    Heading warpHeading = Heading::FromDirection(dirFromLastPoppedNode);

    // Log that we're skipping to the view.
    std::string log1 = StringUtil::Format("%s SkipToView from Point(%.1f, %.1f)", mGKOwner->GetName().c_str(), mGKOwner->GetPosition().x, mGKOwner->GetPosition().z);
    gReportManager.Log("Generic", log1);

    std::string log2 = StringUtil::Format("%s SkipToView to Point(%.1f, %.1f) Heading(%.1f)", mGKOwner->GetName().c_str(), mPath.back().x, mPath.back().z, warpHeading.ToDegrees());
    gReportManager.Log("Generic", log2);

    // Do the skip!
    mGKOwner->SetPosition(mPath.back());
    mGKOwner->SetHeading(warpHeading);
    return true;
}

void Walker::RemoveExcessPathNodes()
{
    // If path is long enough, attempt to remove nodes that are too close to one another.
    if(mPath.size() > 2)
    {
        const float kCloseDist = 5.0f;
        Vector3 prev = mPath[0];
        for(auto it = mPath.begin() + 1; it != mPath.end() - 1;)
        {
            // If this node is too close to the previous node, remove it.
            Vector3 curr = *it;
            if((curr - prev).GetLengthSq() < kCloseDist * kCloseDist)
            {
                it = mPath.erase(it);
            }
            else
            {
                prev = curr;
                ++it;
            }
        }
    }
}

bool Walker::AdvancePath()
{
    if(!mPath.empty())
    {
        //Debug::DrawLine(GetOwner()->GetPosition(), mPath.back(), Color32::White);

        // When walking, we're always mainly interested in getting to the end of the path.
        // Particularly, don't ONLY check distance to last node!
        // If we skipped a node (maybe overshot it), that's OK!
        int atNode = -1;
        for(int i = 0; i < mPath.size(); i++)
        {
            float maxDist = i == 0 ? 2.0f : kAtNodeDist;
            if(AtPosition(mPath[i], maxDist))
            {
                atNode = i;
                break;
            }
        }
        if(atNode > -1)
        {
            while(mPath.size() > atNode)
            {
                mPath.pop_back();
            }

            // When we're about to walk to a path node, make sure it's y-pos is accurate.
            // The pathing system operates on the XZ plane, but the world being navigated has height!
            UpdateNextNodesYPos();
        }
    }

    // Return whether path is completed.
    return mPath.empty();
}

void Walker::UpdateNextNodesYPos()
{
    if(!mPath.empty())
    {
        mPath.back().y = gSceneManager.GetScene()->GetFloorY(mPath.back());
    }
}

float Walker::GetWalkTurnSpeed(Vector3 toNext)
{
    const float kFastestTurnSpeedDist = 30.0f;
    const float kSlowestTurnSpeedDist = 100.0f;
    float t = Math::Clamp((toNext.GetLength() - kFastestTurnSpeedDist) / (kSlowestTurnSpeedDist - kFastestTurnSpeedDist), 0.0f, 1.0f);
    return Interpolate::CubicOut(kWalkTurnSpeedMax, kWalkTurnSpeedMin, t);
}

bool Walker::TurnToFace(float deltaTime, const Vector3& desiredDir, float turnSpeed)
{
    bool doneTurning = false;
    Vector3 currentDir = GetOwner()->GetForward();

    // What angle do I need to rotate to face the desired direction?
    float currToDesiredAngle = Math::Acos(Vector3::Dot(currentDir, desiredDir));
    if(currToDesiredAngle > kAtHeadingRadians)
    {
        float rotateDirection = 0;
        if(rotateDirection == 0)
        {
            // Which way do I rotate to get to facing direction I want?
            // Can use y-axis of cross product to determine this.
            Vector3 cross = Vector3::Cross(currentDir, desiredDir);

            // If y-axis is zero, it means vectors are parallel (either exactly facing or exactly NOT facing).
            // In that case, 1.0f default is fine. Otherwise, we want either 1.0f or -1.0f.
            rotateDirection = cross.y >= 0.0f ? 1.0f : -1.0f;
        }

        // Determine how much we'll rotate this frame.
        // If it's enough to where our angle will be less than the "at heading" rotation, just set heading exactly.
        float thisFrameRotateAngle = turnSpeed * deltaTime;
        if(currToDesiredAngle - thisFrameRotateAngle <= kAtHeadingRadians)
        {
            thisFrameRotateAngle = currToDesiredAngle;
            doneTurning = true;
        }

        // Factor direction into angle.
        thisFrameRotateAngle *= rotateDirection;

        // Rotate actor's mesh to face the desired direction, rotating around the actor's position.
        // Remember, the GKActor follows the mesh's position during animations (based on hip bone placement). But the mesh itself may have a vastly different origin b/c of playing animation.
        mGKOwner->Rotate(thisFrameRotateAngle);
    }
    else
    {
        doneTurning = true;
    }
    return doneTurning;
}

void Walker::StopAllWalkAnimations()
{
    gSceneManager.GetScene()->GetAnimator()->Stop(mWalkStartAnim, true);
    gSceneManager.GetScene()->GetAnimator()->Stop(mWalkStartTurnLeftAnim, true);
    gSceneManager.GetScene()->GetAnimator()->Stop(mWalkStartTurnRightAnim, true);
    gSceneManager.GetScene()->GetAnimator()->Stop(mWalkLoopAnim, true);
    gSceneManager.GetScene()->GetAnimator()->Stop(mCharConfig->walkStopAnim, true);
}

void Walker::OutputWalkerPlan()
{
    #if defined(DEBUG_WALKER)
    printf("%s: Walker Plan Created:\n", mGKOwner->GetNoun().c_str());
    for(int i = mWalkActions.size() - 1; i >= 0; --i)
    {
        int num = mWalkActions.size() - i - 1;
        switch(mWalkActions[i])
        {
        case WalkOp::None:
            printf("\t%i: None\n", num);
            break;
        case WalkOp::FollowPathStart:
            printf("\t%i: FollowPathStart\n", num);
            break;
        case WalkOp::FollowPathStartTurnLeft:
            printf("\t%i: FollowPathStartTurnLeft\n", num);
            break;
        case WalkOp::FollowPathStartTurnRight:
            printf("\t%i: FollowPathStartTurnRight\n", num);
            break;
        case WalkOp::FollowPath:
            printf("\t%i: FollowPath\n", num);
            break;
        case WalkOp::FollowPathEnd:
            printf("\t%i: FollowPathEnd\n", num);
            break;
        case WalkOp::TurnToFace:
            printf("\t%i: TurnToFace ", num);
            std::cout << mTurnToFaceDir << std::endl;
            break;
        }
    }
    #endif
}