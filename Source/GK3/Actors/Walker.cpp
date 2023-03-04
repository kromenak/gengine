#include "Walker.h"

#include "Actor.h"
#include "Animator.h"
#include "Camera.h"
#include "CharacterManager.h"
#include "Debug.h"
#include "Frustum.h"
#include "GEngine.h"
#include "GKActor.h"
#include "GKProp.h"
#include "Heading.h"
#include "MeshRenderer.h"
#include "Quaternion.h"
#include "Ray.h"
#include "Scene.h"
#include "Services.h"
#include "StringUtil.h"
#include "Vector3.h"
#include "VertexAnimator.h"
#include "WalkerBoundary.h"

TYPE_DEF_CHILD(Component, Walker);

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

void Walker::WalkTo(const Vector3& position, std::function<void()> finishCallback)
{
    WalkTo(position, Heading::None, finishCallback);
}

void Walker::WalkTo(const Vector3& position, const Heading& heading, std::function<void()> finishCallback)
{
    WalkToInternal(position, heading, finishCallback, false);
}

void Walker::WalkToGas(const Vector3& position, const Heading& heading, std::function<void()> finishCallback)
{
    WalkToInternal(position, heading, finishCallback, true);
}

void Walker::WalkToSee(GKObject* target, std::function<void()> finishCallback)
{
    mWalkToSeeTarget = target;

    // Not from autoscript, for sure.
    mFromAutoscript = false;

    // Check whether thing is already in view.
    // If so, we don't even need to walk (but may need to turn-to-face).
    Vector3 facingDir;
    if(IsWalkToSeeTargetInView(facingDir))
    {
        // Be sure to save finish callback in this case - it usually happens in walk to.
        mFinishedPathCallback = finishCallback;

        // Clear current walk.
        bool wasMidWalk = IsMidWalk();
        mWalkActions.clear();

        // If mid-walk, we need to stop walking before turning.
        if(wasMidWalk)
        {
            WalkAction stopWalkAction;
            stopWalkAction.op = WalkOp::FollowPathEnd;
            mWalkActions.push_back(stopWalkAction);
        }

        // No need to walk, but do turn to face the thing.
        WalkAction turnAction;
        turnAction.op = WalkOp::TurnToFace;
        turnAction.facingDir = facingDir;
        mWalkActions.push_back(turnAction);

        // OK, walk sequence has been created - let's start doing it!
        NextAction();
    }
    else
    {
        // Specify a "dummy" heading here so that the "turn to face" action is put into the walk plan.
        // Later on, when the object comes into view, we'll replace this with the actual direction to turn.
        WalkTo(target->GetPosition(), Heading::FromDegrees(0.0f), finishCallback);
    }
}

void Walker::WalkOutOfRegion(int regionIndex, const Vector3& exitPosition, const Heading& exitHeading, std::function<void()> finishCallback)
{
    // For now, we can only track one "exit region" request at a time. So clear other if set.
    if(mExitRegionCallback != nullptr)
    {
        auto callback = mExitRegionCallback;
        mExitRegionCallback = nullptr;
        callback();
    }
    
    // Save exit region and callback.
    mExitRegionIndex = regionIndex;
    mExitRegionCallback = finishCallback;

    // Attempt to leave the region by moving towards the exit position/heading.
    // Even if already outside the region, we want to let this go for at least one frame. When Update is called, it'll get cleared.
    WalkTo(exitPosition, exitHeading, nullptr);
}

void Walker::SkipToEnd()
{
    // If not walking, or at the end of the walk, nothing to skip.
    if(!IsWalking() || mWalkActions.back().op == WalkOp::FollowPathEnd || mWalkActions.back().op == WalkOp::TurnToFace)
    {
        return;
    }

    // Warp actor to end of path.
    if(!mPath.empty())
    {
        mGKOwner->SetPosition(mPath.front());
        if(mPath.size() > 1)
        {
            mGKOwner->SetHeading(Heading::FromDirection(Vector3::Normalize(mPath.front() - mPath[1])));
        }
    }

    // Move actor back by "at position" amount, so end walk anim puts us in right spot.
    mGKOwner->SetPosition(mGKOwner->GetPosition() - (mGKOwner->GetForward() * kAtNodeDist));

    // Remove all walk ops except for the "follow path end" op.
    bool poppedAction = false;
    while(!mWalkActions.empty() && mWalkActions.back().op != WalkOp::FollowPathEnd)
    {
        mWalkActions.pop_back();
        poppedAction = true;
    }
    if(poppedAction)
    {
        NextAction();
    }
}

bool Walker::AtPosition(const Vector3& position)
{
    return (GetOwner()->GetPosition() - position).GetLengthSq() <= kAtNodeDistSq;
}

Texture* Walker::GetFloorTypeWalkingOn() const
{
    Scene* scene = GEngine::Instance()->GetScene();
    if(scene == nullptr) { return nullptr; }
    return scene->GetFloorTexture(GetOwner()->GetPosition());
}

void Walker::OnUpdate(float deltaTime)
{
    // Debug draw the path.
    /*
    if(mPath.size() > 0)
    {
        Vector3 prev = mPath.back();
        for(int i = static_cast<int>(mPath.size()) - 2; i >= 0; i--)
        {
            Debug::DrawLine(prev, mPath[i], Color32::Orange);
            Debug::DrawLine(mPath[i], mPath[i] + Vector3::UnitY * 10.0f, Color32::Orange);
            prev = mPath[i];
        }
    }
    */

    // Process outstanding walk actions.
    if(mWalkActions.size() > 0)
    {
        // Kind of a HACK: if we're walking, and action manager is skipping, move to end of movement ASAP.
        // Without this, walks during fast-forwards can get stuck and cause the game to freeze.
        //TODO: *Probably* a better way to handle this, with a substantial refactor...
        if(Services::Get<ActionManager>()->IsSkippingCurrentAction())
        {
            SkipToEnd();
        }

        // Increase current action timer.
        mCurrentWalkActionTimer += deltaTime;

        // Stay on the ground.
        mGKOwner->SnapToFloor();

        // Handle the current walk action.
        WalkAction& currentAction = mWalkActions.back();
        if(currentAction.op == WalkOp::FollowPathStart)
        {
            // Possible to finish path while still in the start phase, for very short paths.
            if(AdvancePath())
            {
                // In this case, we want to skip all follow path bits and go right to "follow path end".
                while(mWalkActions.size() > 0 && mWalkActions.back().op != WalkOp::FollowPathEnd)
                {
                    mWalkActions.pop_back();
                }
                NextAction();
            }
            else
            {
                Vector3 toNext = mPath.back() - GetOwner()->GetPosition();
                toNext.y = 0.0f;
                TurnToFace(deltaTime, GetOwner()->GetForward(), toNext.Normalize(), kWalkTurnSpeed);
            }
        }
        else if(currentAction.op == WalkOp::FollowPathStartTurnLeft || currentAction.op == WalkOp::FollowPathStartTurnRight)
        {
            if(!AdvancePath())
            {
                // HACK: Wait until the "turn" anim has the character put its foot down before starting to turn to the target.
                // About 0.66s seems to look OK, but could probably be finessed a bit.
                if(mCurrentWalkActionTimer > 0.66f)
                {
                    Vector3 toNext = mPath.back() - GetOwner()->GetPosition();
                    toNext.y = 0.0f;
                    TurnToFace(deltaTime, GetOwner()->GetForward(), toNext.Normalize(), kWalkTurnSpeed);
                }
            }
        }
        else if(currentAction.op == WalkOp::FollowPath)
        {
            // If we have a "walk to see" target, check whether it has come into view.
            Vector3 facingDir;
            if(IsWalkToSeeTargetInView(facingDir))
            {
                // Make sure final action is a "turn to face" using this facing dir.
                assert(mWalkActions[0].op == WalkOp::TurnToFace);
                mWalkActions[0].facingDir = facingDir;
                
                PopAndNextAction();
            }
            else if(AdvancePath()) // Otherwise, see if finished following path.
            {
                // If we get here and we have a "walk to see" target, it means we got to end of path without ever actully seeing the thing!
                // So, force-set it to something reasonable!
                if(mWalkToSeeTarget != nullptr)
                {
                    assert(mWalkActions[0].op == WalkOp::TurnToFace);
                    Vector3 dir = mWalkToSeeTarget->GetPosition() - mGKOwner->GetPosition();
                    dir.y = 0.0f;
                    mWalkActions[0].facingDir = Vector3::Normalize(dir);
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
                    animParams.fromAutoScript = mFromAutoscript;
                    animParams.finishCallback = std::bind(&Walker::OnWalkAnimFinished, this);
                    GEngine::Instance()->GetScene()->GetAnimator()->Start(animParams);
                }

                // Still following path - turn to face next node in path.
                Vector3 toNext = mPath.back() - GetOwner()->GetPosition();
                toNext.y = 0.0f;
                TurnToFace(deltaTime, GetOwner()->GetForward(), toNext.Normalize(), kWalkTurnSpeed);
            }
        }
        else if(currentAction.op == WalkOp::TurnToFace)
        {
            if(TurnToFace(deltaTime, GetOwner()->GetForward(), currentAction.facingDir, kTurnSpeed))
            {
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
            auto callback = mExitRegionCallback;
            mExitRegionCallback = nullptr;
            callback();
        }
    }
}

void Walker::WalkToInternal(const Vector3& position, const Heading& heading, std::function<void()> finishCallback, bool fromAutoscript)
{
    // Clear continue walk anim flag. Since we're starting a new walk.
    mNeedContinueWalkAnim = false;

    // Save if from autoscript.
    mFromAutoscript = fromAutoscript;

    // It's possible for a "walk to" to be received in the middle of another walk sequence.
    // In that case, the current walk sequence is canceled and a new one is constructed.
    bool wasMidWalk = IsMidWalk();

    // Ok, starting a new walk plan.
    mWalkActions.clear();

    // If heading is specified, save "turn to face" action.
    if(heading.IsValid())
    {
        WalkAction turnAction;
        turnAction.op = WalkOp::TurnToFace;
        turnAction.facingDir = heading.ToDirection();
        mWalkActions.push_back(turnAction);
    }

    // Do we need to walk?
    bool doNextAction = true;
    if(!AtPosition(position))
    {
        // We will walk, so save the "walk end" action.
        WalkAction endWalkAction;
        endWalkAction.op = WalkOp::FollowPathEnd;
        mWalkActions.push_back(endWalkAction);

        // Calculate a path.
        if(!CalculatePath(GetOwner()->GetPosition(), position))
        {
            //TODO: Force to position?
            if(finishCallback != nullptr) { finishCallback(); }
            return;
        }

        // Ok, we have a path (in mPath) that we can follow.
        // One interesting bit we do is to shorten the path if a lot of nodes are off-screen.
        // So if 90% of the path is behind the camera, the walker can just skip to the node right before the camera and walk from there!
        size_t originalPathSize = mPath.size();
        if(!fromAutoscript)
        {
            Vector3 lastPoppedPathPos;
            Camera* camera = Services::GetRenderer()->GetCamera();
            Frustum frustum = camera->GetWorldSpaceViewFrustum();
            while(mPath.size() > 1)
            {
                // Most basic check: skip path node if not within the frustum.
                bool skipPathNode = false;
                if(!frustum.ContainsPoint(mPath.back()))
                {
                    skipPathNode = true;
                }

                // If we think we want to skip this path node (because its outside view frustum), consider some edge cases.
                // First, the path node is at floor level, but walkers can be rather tall.
                // See if the walker's head will be visible in the frustum, despite the path pos being outside it.
                if(skipPathNode)
                {
                    Vector3 headPosAtNode = mPath.back() + Vector3::UnitY * mCharConfig->walkerHeight;
                    if(frustum.ContainsPoint(headPosAtNode))
                    {
                        skipPathNode = false;
                    }
                }

                // If the path node is fairly close to the camera position, don't skip it.
                // This helps to ensure that the walker doesn't "pop in" when coming in from behind the camera.
                if(skipPathNode)
                {
                    const float kDontSkipNodeDist = 75.0f;
                    float distSq = (mPath.back() - camera->GetOwner()->GetPosition()).GetLengthSq();
                    if(distSq < kDontSkipNodeDist * kDontSkipNodeDist)
                    {
                        skipPathNode = false;
                    }
                }

                // Ok, all cases checked!
                // If path node is skipped, get rid of it.
                if(skipPathNode)
                {
                    lastPoppedPathPos = mPath.back();
                    mPath.pop_back();
                }
                else
                {
                    // This path node WAS NOT skipped, stop shortening path!
                    // But push the last popped position back onto the path (so that walker starts walking _just_ off-screen).
                    if(mPath.size() < originalPathSize)
                    {
                        mPath.push_back(lastPoppedPathPos);
                    }
                    break;
                }
            }
        }

        // If path was shortened, warp walker directly to last spot.
        bool warp = mPath.size() < originalPathSize;
        if(warp)
        {
            std::cout << "Warp to " << mPath.back() << std::endl;
            mGKOwner->SetPosition(mPath.back());

            if(mPath.size() > 1)
            {
                Vector3 dir = Vector3::Normalize(mPath[mPath.size() - 2] - mPath.back());
                std::cout << "Warp heading " << dir << ", " << Heading::FromDirection(dir) << std::endl;
                mGKOwner->SetHeading(Heading::FromDirection(dir));
            }
        }

        // Follow the path.
        WalkAction followPathAction;
        followPathAction.op = WalkOp::FollowPath;
        mWalkActions.push_back(followPathAction);

        // If did not warp, we need to determine the "start walk" action.
        if(!warp)
        {
            // If first path node is in front of the walker, no need to play turn anims.
            Vector3 toNext = (mPath.front() - GetOwner()->GetPosition()).Normalize();
            if(Vector3::Dot(GetOwner()->GetForward(), toNext) > -0.5f)
            {
                // Add start walk action, but ONLY IF wasn't previously mid-walk.
                // If was already walking and next node is in same direction...just keep walking!
                if(!wasMidWalk)
                {
                    WalkAction startWalkAction;
                    startWalkAction.op = WalkOp::FollowPathStart;
                    mWalkActions.push_back(startWalkAction);
                }
                else
                {
                    // In this case, we were already walking, and not warping - DO NOT play the next action right away!
                    doNextAction = false;
                }
            }
            else
            {
                // First node IS NOT in front of walker, so gotta turn using fancy anims.

                // So, we need to play start walk anim while also turning to face next node.
                WalkAction startWalkTurnAction;
                startWalkTurnAction.facingDir = toNext;

                // For initial direction to turn, let's use the goal node direction.
                // Just thinking about the real world...you usually turn towards your goal, right?
                Vector3 toLastDir = (mPath.front() - GetOwner()->GetPosition()).Normalize();
                Vector3 cross = Vector3::Cross(GetOwner()->GetForward(), toLastDir);
                if(cross.y > 0)
                {
                    startWalkTurnAction.op = WalkOp::FollowPathStartTurnRight;
                }
                else
                {
                    startWalkTurnAction.op = WalkOp::FollowPathStartTurnLeft;
                }

                // Make sure this actor has animations for turning left/right.
                // A lot of actors actually don't...in which case normal starts after all.
                if(startWalkTurnAction.op == WalkOp::FollowPathStartTurnRight && mWalkStartTurnRightAnim == nullptr)
                {
                    startWalkTurnAction.op = WalkOp::FollowPathStart;
                }
                else if(startWalkTurnAction.op == WalkOp::FollowPathStartTurnLeft && mWalkStartTurnLeftAnim == nullptr)
                {
                    startWalkTurnAction.op = WalkOp::FollowPathStart;
                }

                // Let's do it!
                mWalkActions.push_back(startWalkTurnAction);
            }
        }
    }

    // Save finish callback.
    mFinishedPathCallback = finishCallback;
    
    // OK, walk sequence has been created - let's start doing it!
    //std::cout << "Walk Sequence Begin!" << std::endl;
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
        mPrevWalkOp = mWalkActions.back().op;
        mWalkActions.pop_back();
    }
    
    // Go to next action.
    NextAction();
}

void Walker::NextAction()
{
    mCurrentWalkActionTimer = 0.0f;
    
    // Do next action in sequence, if any.
    if(mWalkActions.size() > 0)
    {
        if(mWalkActions.back().op == WalkOp::FollowPathStart)
        {
            //std::cout << "Follow Path Start" << std::endl;
            
            AnimParams animParams;
            animParams.animation = mWalkStartAnim;
            animParams.allowMove = true;
            animParams.fromAutoScript = mFromAutoscript;
            animParams.finishCallback = std::bind(&Walker::PopAndNextAction, this);
            GEngine::Instance()->GetScene()->GetAnimator()->Start(animParams);
        }
        else if(mWalkActions.back().op == WalkOp::FollowPathStartTurnLeft)
        {
            //std::cout << "Follow Path Start (Turn Left)" << std::endl;
            
            AnimParams animParams;
            animParams.animation = mWalkStartTurnLeftAnim;
            animParams.allowMove = true;
            animParams.fromAutoScript = mFromAutoscript;
            animParams.finishCallback = std::bind(&Walker::PopAndNextAction, this);
            GEngine::Instance()->GetScene()->GetAnimator()->Start(animParams);
        }
        else if(mWalkActions.back().op == WalkOp::FollowPathStartTurnRight)
        {
            //std::cout << "Follow Path Start (Turn Right)" << std::endl;
            
            AnimParams animParams;
            animParams.animation = mWalkStartTurnRightAnim;
            animParams.allowMove = true;
            animParams.fromAutoScript = mFromAutoscript;
            animParams.finishCallback = std::bind(&Walker::PopAndNextAction, this);
            GEngine::Instance()->GetScene()->GetAnimator()->Start(animParams);
        }
        else if(mWalkActions.back().op == WalkOp::FollowPath)
        {
            //std::cout << "Follow Path" << std::endl;
            
            AnimParams animParams;
            animParams.animation = mWalkLoopAnim;
            animParams.allowMove = true;
            animParams.fromAutoScript = mFromAutoscript;
            animParams.finishCallback = std::bind(&Walker::OnWalkAnimFinished, this);
            
            if(mPrevWalkOp == WalkOp::FollowPathStartTurnRight)
            {
                animParams.startFrame = 10;
            }
            
            GEngine::Instance()->GetScene()->GetAnimator()->Start(animParams);
        }
        else if(mWalkActions.back().op == WalkOp::FollowPathEnd)
        {
            //std::cout << "Follow Path End" << std::endl;
            GEngine::Instance()->GetScene()->GetAnimator()->Stop(mWalkStartAnim);
            GEngine::Instance()->GetScene()->GetAnimator()->Stop(mWalkLoopAnim);

            // Play walk stop anim.
            AnimParams animParams;
            animParams.animation = mCharConfig->walkStopAnim;
            animParams.allowMove = true;
            animParams.fromAutoScript = mFromAutoscript;
            animParams.finishCallback = std::bind(&Walker::PopAndNextAction, this);
            GEngine::Instance()->GetScene()->GetAnimator()->Start(animParams);
        }
        else if(mWalkActions.back().op == WalkOp::TurnToFace)
        {
            //std::cout << "Turn To Face" << std::endl;
            // Handled in Update
        }
    }
    else
    {
        //std::cout << "Walk Sequence Done!" << std::endl;
        
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
        auto callback = mFinishedPathCallback;
        mFinishedPathCallback = nullptr;
        callback();
    }
}

bool Walker::IsWalkToSeeTargetInView(Vector3& outTurnToFaceDir)
{
    // Not in view if it doesn't exist!
    if(mWalkToSeeTarget == nullptr) { return false; }

    //TODO: Could maybe cache this AABB to reduce per-frame computation.
    AABB targetAABB = mWalkToSeeTarget->GetAABB();
    //Debug::DrawAABB(targetAABB, Color32::Orange, 5.0f);

    // Create ray from head in direction of target position.
    Vector3 headPos = mGKOwner->GetHeadPosition();
    Vector3 dir = (targetAABB.GetCenter() - headPos).Normalize();
    Ray ray(headPos, dir);
    //Debug::DrawLine(targetAABB.GetCenter(), headPos, Color32::Red);
    
    // If hit the target with the ray, it must be in view.
    SceneCastResult result = GEngine::Instance()->GetScene()->Raycast(ray, false, mGKOwner);
    //std::cout << result.hitInfo.name << " vs " << mWalkToSeeTarget->GetName() << std::endl;

    if(StringUtil::EqualsIgnoreCase(result.hitInfo.name, mWalkToSeeTarget->GetNoun()) ||
       StringUtil::EqualsIgnoreCase(result.hitInfo.name, mWalkToSeeTarget->GetName()))
    {
        // Convert ray direction to a "facing" direction,
        dir.y = 0.0f;
        outTurnToFaceDir = dir.Normalize();
        return true;
    }
    return false;
}

bool Walker::CalculatePath(const Vector3& startPos, const Vector3& endPos)
{
    if(mWalkerBoundary == nullptr) { return false; }
    
    if(mWalkerBoundary->FindPath(startPos, endPos, mPath))
    {
        // Try to reduce the number of nodes in the path.
        // The walker nodes are very close to one another.
        int iterations = 2;
        while(iterations > 0)
        {
            bool getRid = false;
            for(auto it = mPath.end() - 1; it != mPath.begin(); --it)
            {
                if(getRid)
                {
                    it = mPath.erase(it);
                }
                getRid = !getRid;
            }
            --iterations;
        }
        
        // The path generated by A* is likely too "fidgety" for human-like movement.
        // Clean up path by getting rid of excess nodes.
        /*
        for(auto it = mPath.end() - 1; it != mPath.begin(); --it)
        {
            if(it + 1 == mPath.end() || it == mPath.begin()) { continue; }
            
            Vector3 pointBefore = *(it - 1);
            Vector3 point = *(it);
            Vector3 pointAfter = *(it + 1);
            
            Vector3 toAfter = Vector3::Normalize(pointAfter - point);
            Vector3 toBefore = Vector3::Normalize(pointBefore - point);
            
            float dot = Vector3::Dot(toAfter, toBefore);
            if(dot < -0.65f)
            {
                mPath.erase(it);
            }
        }
        */

        // Paths generated by the walker boundary are completely flat, but the scene itself may have some verticality.
        // So, let's factor that into our final path too.
        for(auto& node : mPath)
        {
            node.y = GEngine::Instance()->GetScene()->GetFloorY(node);
        }
    }
    else
    {
        // Couldn't find a path so...let's just walk straight to the spot!
        // This'll probably look broken...but it's the ultimate fallback.
        mPath.clear();
        mPath.push_back(endPos);
        mPath.push_back(startPos);
    }
    return true;
}

bool Walker::AdvancePath()
{
    if(mPath.size() > 0)
    {
        //Debug::DrawLine(GetOwner()->GetPosition(), mPath.back(), Color32::White);
        
        // When walking, we're always mainly interested in getting to the end of the path.
        // Particularly, don't ONLY check distance to last node!
        // If we skipped a node (maybe overshot it), that's OK!
        int atNode = -1;
        for(int i = 0; i < mPath.size(); i++)
        {
            if(AtPosition(mPath[i]))
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
        }
    }
    
    // Return whether path is completed.
    return mPath.empty();
}

bool Walker::TurnToFace(float deltaTime, const Vector3& currentDir, const Vector3& desiredDir, float turnSpeed, int turnDir)
{
    bool doneTurning = false;
    
    // What angle do I need to rotate to face the desired direction?
    float currToDesiredAngle = Math::Acos(Vector3::Dot(currentDir, desiredDir));
    if(currToDesiredAngle > kAtHeadingRadians)
    {
        float rotateDirection = turnDir;
        if(rotateDirection == 0)
        {
            // Which way do I rotate to get to facing direction I want?
            // Can use y-axis of cross product to determine this.
            Vector3 cross = Vector3::Cross(currentDir, desiredDir);
            
            // If y-axis is zero, it means vectors are parallel (either exactly facing or exactly NOT facing).
            // In that case, 1.0f default is fine. Otherwise, we want either 1.0f or -1.0f.
            rotateDirection = cross.y >= 0.0f ? 1 : -1;
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
