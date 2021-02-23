//
// Walker.cpp
//
// Clark Kromenaker
//
#include "Walker.h"

#include "Actor.h"
#include "Animator.h"
#include "CharacterManager.h"
#include "Debug.h"
#include "GEngine.h"
#include "GKActor.h"
#include "Heading.h"
#include "Matrix4.h"
#include "MeshRenderer.h"
#include "Quaternion.h"
#include "Scene.h"
#include "Services.h"
#include "StringUtil.h"
#include "Vector3.h"
#include "WalkerBoundary.h"

TYPE_DEF_CHILD(Component, Walker);

Walker::Walker(Actor* owner) : Component(owner),
    mGKOwner(static_cast<GKActor*>(owner))
{
    
}

void Walker::SnapToFloor()
{
    Scene* scene = GEngine::Instance()->GetScene();
    if(scene != nullptr)
    {
        Vector3 pos = GetOwner()->GetPosition();
        pos.y = scene->GetFloorY(pos) + mCharConfig->shoeThickness;
        GetOwner()->SetPosition(pos);
    }
}

void Walker::WalkTo(const Vector3& position, WalkerBoundary* walkerBoundary, std::function<void()> finishCallback)
{
    WalkTo(position, Heading::None, walkerBoundary, finishCallback);
}

Vector3 turnAnimDir;
float turnAnimTimer = 0.0f;
float turnAngle = 0.0f;

void Walker::WalkTo(const Vector3& position, const Heading& heading, WalkerBoundary* walkerBoundary, std::function<void()> finishCallback)
{
    // DEBUG: visualize goal world position and position on A* node map.
    //Debug::DrawLine(position, position + Vector3::UnitY * 10, Color32::Red, 10.0f);
    //Vector3 converted = walkerBoundary->TexturePosToWorldPos(walkerBoundary->WorldPosToTexturePos(position));
    //Debug::DrawLine(converted, converted + Vector3::UnitY * 10, Color32::Green, 10.0f);
    
    // Save walker boundary.
    mWalkerBoundary = walkerBoundary;
    
    // Save finish callback.
    mFinishedPathCallback = finishCallback;
    
    // It's possible for a "walk to" to be received in the middle of another walk sequence.
    // In that case, the current walk sequence is canceled and a new one is constructed.
    bool wasMidWalk = mWalkActions.size() > 0 && mWalkActions.back().op == WalkOp::FollowPath;
    //TODO: Clear/finish current sequence.
    mWalkActions.clear();
    
    // If heading is specified, save "turn to face" action.
    if(heading.IsValid())
    {
        WalkAction turnAction;
        turnAction.op = WalkOp::TurnToFace;
        turnAction.facingDir = heading.ToVector();
        mWalkActions.push_back(turnAction);
    }
    
    // Do we need to walk?
    if((GetOwner()->GetPosition() - position).GetLengthSq() > kAtNodeDistSq)
    {
        // We will walk, so save the "walk end" action.
        WalkAction endWalkAction;
        endWalkAction.op = WalkOp::FollowPathEnd;
        mWalkActions.push_back(endWalkAction);
        
        // Calculate a path.
        CalculatePath(GetOwner()->GetPosition(), position);
        
        // Follow the path.
        WalkAction followPathAction;
        followPathAction.op = WalkOp::FollowPath;
        mWalkActions.push_back(followPathAction);
        
        // Determine start walk action. This depends on whether the next node is right in front of us, behind, etc.
        // If first path node is in front of the walker, no need to play turn anims.
        Vector3 toNext = (mPath.back() - GetOwner()->GetPosition()).Normalize();
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
        }
        else
        {
            // First node IS NOT in front of walker, so gotta turn using fancy anims.
            
            // So, we need to play start walk anim while also turning to face next node.
            WalkAction startWalkTurnAction;
            startWalkTurnAction.facingDir = toNext;
            
            // For initial direction to turn, let's use the goal node direction.
            // Just thinking about the real world...you usually turn towards your goal, right?
            Vector3 toLastDir = (mPath.back() - GetOwner()->GetPosition()).Normalize();
            Vector3 cross = Vector3::Cross(GetOwner()->GetForward(), toLastDir);
            if(cross.y > 0)
            {
                startWalkTurnAction.op = WalkOp::FollowPathStartTurnRight;
            }
            else
            {
                startWalkTurnAction.op = WalkOp::FollowPathStartTurnLeft;
            }
            
            // The "start turn" anims turn the actor 135 degrees to the left or right.
            // But what if out actual desired facing is more or less than that?
            // Well, we can make this one anim work for any rotation by stealthily applying more or less rotation during the start of the anim!
            turnAnimDir = Quaternion(Vector3::UnitY, Math::ToRadians(135.0f)).Rotate(GetOwner()->GetForward());
            Vector3 toNextDir = (mPath.back() - GetOwner()->GetPosition()).Normalize();
            
            // Get diff between where turn anim will face by default and where we actually need to face.
            turnAngle = Math::Acos(Vector3::Dot(turnAnimDir, toNextDir));
            
            // Apply that turn to current facing.
            
            turnAnimTimer = 0.0f;
            
            mWalkActions.push_back(startWalkTurnAction);
        }
    }
    
    // OK, walk sequence has been created - let's start doing it!
    std::cout << "Walk Sequence Begin!" << std::endl;
    NextAction();
}

void Walker::WalkToSee(const std::string& targetName, const Vector3& targetPosition, WalkerBoundary* walkerBoundary, std::function<void()> finishCallback)
{
    mWalkToSeeTarget = targetName;
    mWalkToSeeTargetPosition = targetPosition;
    
    // Check whether thing is already in view.
    // If so, we don't even need to walk (but may need to turn-to-face).
    Vector3 facingDir;
    if(IsWalkToSeeTargetInView(facingDir))
    {
        // Be sure to save finish callback in this case - it usually happens in walk to.
        mFinishedPathCallback = finishCallback;
        
        //TODO: Clear/finish current sequence.
        
        // No need to walk, but do turn to face the thing.
        WalkAction turnAction;
        turnAction.op = WalkOp::TurnToFace;
        turnAction.facingDir = facingDir;
        mWalkActions.push_back(turnAction);
    }
    else
    {
        WalkTo(targetPosition, walkerBoundary, finishCallback);
    }
    
    // OK, walk sequence has been created - let's start doing it!
    NextAction();
}

void Walker::OnUpdate(float deltaTime)
{
    // Debug draw the path.
    if(mPath.size() > 0)
    {
        Vector3 prev = mPath.back();
        for(int i = static_cast<int>(mPath.size()) - 2; i >= 0; i--)
        {
            Debug::DrawLine(prev, mPath[i], Color32::Orange);
            prev = mPath[i];
        }
    }
    
    if(mWalkActions.size() > 0)
    {
        WalkAction& currentAction = mWalkActions.back();
        if(currentAction.op == WalkOp::FollowPathStart)
        {
            if(!AdvancePath())
            {
                Vector3 toNextDir = (mPath.back() - GetOwner()->GetPosition()).Normalize();
                TurnToFace(deltaTime, GetOwner()->GetForward(), toNextDir, kWalkTurnSpeed);
            }
        }
        else if(currentAction.op == WalkOp::FollowPathStartTurnLeft || currentAction.op == WalkOp::FollowPathStartTurnRight)
        {
            if(!AdvancePath())
            {
                turnAnimTimer += deltaTime;
                
                int shoeMeshIndex = currentAction.op == WalkOp::FollowPathStartTurnLeft ? mCharConfig->leftShoeAxesMeshIndex : mCharConfig->rightShoeAxesMeshIndex;
                int shoeSubmeshIndex = currentAction.op == WalkOp::FollowPathStartTurnLeft ? mCharConfig->leftShoeAxesGroupIndex : mCharConfig->rightShoeAxesGroupIndex;
                int shoePointIndex = currentAction.op == WalkOp::FollowPathStartTurnLeft ? mCharConfig->leftShoeAxesPointIndex : mCharConfig->rightShoeAxesPointIndex;
                
                MeshRenderer* meshRenderer = mGKOwner->GetMeshRenderer();
                Matrix4 localToWorldMatrix = meshRenderer->GetOwner()->GetTransform()->GetLocalToWorldMatrix();
                Matrix4 meshToLocalMatrix = meshRenderer->GetMesh(shoeMeshIndex)->GetMeshToLocalMatrix();
                Matrix4 meshToWorldMatrix = localToWorldMatrix * meshToLocalMatrix;
                
                Vector3 footDir = -meshToWorldMatrix.GetZAxis();
                footDir.y = 0.0f;
                footDir.Normalize();
                
                Vector3 footPos = meshToWorldMatrix.TransformPoint(meshRenderer->GetMesh(shoeMeshIndex)->GetSubmesh(shoeSubmeshIndex)->GetVertexPosition(shoePointIndex));
                Debug::DrawLine(footPos, footPos + footDir * 10.0f, Color32::Magenta);
                
                Debug::DrawLine(footPos, mPath.back(), Color32::Yellow);
                //Vector3 toNextDir = (mPath.back() - footPos).Normalize();
                //Quaternion turnAmount = GetTurnAmount(deltaTime, footDir, toNextDir, kWalkTurnSpeed, 0);
                //meshRenderer->GetOwner()->GetTransform()->RotateAround(footPos, turnAmount);
                
                GetOwner()->SetPosition(footPos);
                mGKOwner->SetHeading(Heading::FromDirection(footDir));

                
                /*
                if(turnAnimTimer < 1.0f)
                {
                    Debug::DrawLine(footPos, mPath.back(), Color32::Yellow);
                    Vector3 toNextDir = (mPath.back() - footPos).Normalize();
                    
                    // So, we can turn the model during the anim, but we need to keep track of what the final facing will be.
                    int turnDir = currentAction.op == WalkOp::FollowPathStartTurnLeft ? -1 : 1;
                    Quaternion turnAmount = GetTurnAmount(deltaTime, footDir, toNextDir, kWalkTurnSpeed, turnDir);
                    
                    meshRenderer->GetOwner()->GetTransform()->RotateAround(footPos, turnAmount);
                }
                */
                
                /*
                if(turnAnimTimer < 0.6f)
                {
                    Vector3 toNextDir = (mPath.back() - GetOwner()->GetPosition()).Normalize();
                    
                    // Here's the idea with "turning to face" for these start anims.
                    // The anim is authored with a 135 degree turn in mind.
                    // But obviously the actual direction the walker is turning will not always be exactly that!
                    
                    // So, we can turn the model during the anim, but we need to keep track of what the final facing will be.
                    int turnDir = currentAction.op == WalkOp::FollowPathStartTurnLeft ? -1 : 1;
                    Quaternion turnAmount = GetTurnAmount(deltaTime, turnAnimDir, toNextDir, kWalkTurnSpeed, turnDir);
                    
                    // Rotate the turn anim dir.
                    turnAnimDir = turnAmount.Rotate(turnAnimDir);
                    
                    // Rotate the model too!
                    Transform* meshTransform = mGKOwner->GetMeshRenderer()->GetOwner()->GetTransform();
                    meshTransform->RotateAround(GetOwner()->GetPosition(), turnAmount);
                }
                */
            }
        }
        else if(currentAction.op == WalkOp::FollowPath)
        {
            /*
            // If we have a "walk to see" target, check whether it has come into view.
            bool stopWalkPrematurely = false;
            if(!mWalkToSeeTarget.empty())
            {
                Vector3 facingDir;
                if(IsWalkToSeeTargetInView(facingDir))
                {
                    stopWalkPrematurely = true;
                    
                    // When doing a "walk to see," we want the actor to turn to face.
                    mHasDesiredFacingDir = true;
                    mDesiredFacingDir = facingDir;
                }
            }
            */
            
            if(AdvancePath())
            {
                // Path is finished - move on to next step in sequence.
                PopAndNextAction();
            }
            else
            {
                // Still following path - turn to face next node in path.
                Vector3 toNextDir = (mPath.back() - GetOwner()->GetPosition()).Normalize();
                TurnToFace(deltaTime, GetOwner()->GetForward(), toNextDir, kWalkTurnSpeed);
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
}

void Walker::PopAndNextAction()
{
    // Pop action from sequence.
    if(mWalkActions.size() > 0)
    {
        mWalkActions.pop_back();
    }
    
    // Go to next action.
    NextAction();
}

void Walker::NextAction()
{
    // Do next action in sequence, if any.
    if(mWalkActions.size() > 0)
    {
        if(mWalkActions.back().op == WalkOp::FollowPathStart)
        {
            std::cout << "Follow Path Start" << std::endl;
            GEngine::Instance()->GetScene()->GetAnimator()->Start(mCharConfig->walkStartAnim, true, false,
                                                                  std::bind(&Walker::PopAndNextAction, this));
        }
        else if(mWalkActions.back().op == WalkOp::FollowPathStartTurnLeft)
        {
            std::cout << "Follow Path Start (Turn Left)" << std::endl;
            GEngine::Instance()->GetScene()->GetAnimator()->Start(mCharConfig->walkStartTurnLeftAnim, false, false,
                                                                  std::bind(&Walker::PopAndNextAction, this));
        }
        else if(mWalkActions.back().op == WalkOp::FollowPathStartTurnRight)
        {
            std::cout << "Follow Path Start (Turn Right)" << std::endl;
            GEngine::Instance()->GetScene()->GetAnimator()->Start(mCharConfig->walkStartTurnRightAnim, false, false,
                                                                  std::bind(&Walker::PopAndNextAction, this));
        }
        else if(mWalkActions.back().op == WalkOp::FollowPath)
        {
            std::cout << "Follow Path" << std::endl;
            GEngine::Instance()->GetScene()->GetAnimator()->Start(mCharConfig->walkLoopAnim, true, false,
                                                                  std::bind(&Walker::ContinueWalk, this));
            // Handled in Update
        }
        else if(mWalkActions.back().op == WalkOp::FollowPathEnd)
        {
            std::cout << "Follow Path End" << std::endl;
            GEngine::Instance()->GetScene()->GetAnimator()->Stop(mCharConfig->walkStartAnim);
            GEngine::Instance()->GetScene()->GetAnimator()->Stop(mCharConfig->walkLoopAnim);
            GEngine::Instance()->GetScene()->GetAnimator()->Start(mCharConfig->walkStopAnim, true, false,
                                                                  std::bind(&Walker::PopAndNextAction, this));
        }
        else if(mWalkActions.back().op == WalkOp::TurnToFace)
        {
            std::cout << "Turn To Face" << std::endl;
            // Handled in Update
        }
    }
    else
    {
        std::cout << "Walk Sequence Done!" << std::endl;
        
        // No actions left in sequence => walk is finished.
        OnWalkToFinished();
    }
}


void Walker::ContinueWalk()
{
    // This function is just a helper to loop the "walk loop" anim.
    // Just make sure the current action is still "follow path" and if so we can play the loop anim one more time.
    if(mWalkActions.size() > 0 && mWalkActions.back().op == WalkOp::FollowPath)
    {
        GEngine::Instance()->GetScene()->GetAnimator()->Start(mCharConfig->walkLoopAnim, true, false, std::bind(&Walker::ContinueWalk, this));
    }
}

void Walker::OnWalkToFinished()
{
    // Make sure state variables are cleared.
    mPath.clear();
    mWalkToSeeTarget.clear();
    
    // Call finished callback.
    if(mFinishedPathCallback != nullptr)
    {
        mFinishedPathCallback();
        mFinishedPathCallback = nullptr;
    }
}

bool Walker::IsWalkToSeeTargetInView(Vector3& outTurnToFaceDir)
{
    Vector3 headPos = mGKOwner->GetHeadPosition();
    Debug::DrawLine(headPos, mWalkToSeeTargetPosition, Color32::Magenta);
    
    // Create ray from head in direction of target position.
    Vector3 dir = (mWalkToSeeTargetPosition - headPos).Normalize();
    Ray ray(headPos, dir);
    
    // If hit the target with the ray, it must be in view.
    SceneCastResult result = GEngine::Instance()->GetScene()->Raycast(ray, false, mGKOwner);
    if(StringUtil::EqualsIgnoreCase(result.hitInfo.name, mWalkToSeeTarget))
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
        // Get rid of any nodes that are already too close to the walker.
        /*
        for(int i = mPath.size() - 1; i >= 0; --i)
        {
            if((mPath.back() - mGKOwner->GetPosition()).GetLengthSq() > kAtNodeDistSq * 2.0f)
            {
                break;
            }
            mPath.pop_back();
        }
        */
        
        // Try to reduce the number of nodes in the path.
        // The walker nodes are very close to one another.
        /*
        int iterations = 2;
        while(iterations > 0)
        {
            bool getRid = false;
            for(auto it = mPath.end(); it != mPath.begin(); --it)
            {
                if(getRid)
                {
                    mPath.erase(it);
                }
                getRid = !getRid;
            }
            --iterations;
        }
        */
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
        Debug::DrawLine(GetOwner()->GetPosition(), mPath.back(), Color32::White);
        
        // See if close enough to next node.
        Vector3 toNext = mPath.back() - mGKOwner->GetPosition();
        if(toNext.GetLengthSq() < kAtNodeDistSq)
        {
            // We're at this node - pop it off the path.
            mPath.pop_back();
        }
    }
    
    // Return whether path is completed.
    return mPath.size() == 0;
}

bool Walker::TurnToFace(float deltaTime, const Vector3& currentDir, const Vector3& desiredDir, float turnSpeed, int turnDir, bool aboutOwnerPos)
{
    bool doneTurning = false;
    
    // What angle do I need to rotate to face the desired direction?
    float angle = Math::Acos(Vector3::Dot(currentDir, desiredDir));
    if(angle > kAtHeadingRadians)
    {
        float rotateDirection = turnDir;
        if(turnDir == 0)
        {
            // Which way do I rotate to get to facing direction I want?
            // Can use y-axis of cross product to determine this.
            Vector3 cross = Vector3::Cross(currentDir, desiredDir);
            
            // If y-axis is zero, it means vectors are parallel (either exactly facing or exactly NOT facing).
            // In that case, 1.0f default is fine. Otherwise, we want either 1.0f or -1.0f.
            rotateDirection = 1.0f;
            if(!Math::IsZero(cross.y))
            {
                rotateDirection = cross.y / Math::Abs(cross.y);
            }
        }
        
        // Determine how much we'll rotate this frame.
        // If it's enough to where our angle will be less than the "at heading" rotation, just set heading exactly.
        float angleOfRotation = turnSpeed * deltaTime;
        if(angle - angleOfRotation <= kAtHeadingRadians)
        {
            angleOfRotation = angle;
            doneTurning = true;
        }
        
        // Factor direction into angle.
        angleOfRotation *= rotateDirection;
        
        // Rotate actor's mesh to face the desired direction, rotating around the actor's position.
        // Remember, the GKActor follows the mesh's position during animations (based on hip bone placement). But the mesh itself may have a vastly different origin b/c of playing animation.
        Transform* meshTransform = mGKOwner->GetMeshRenderer()->GetOwner()->GetTransform();
        if(aboutOwnerPos)
        {
            meshTransform->RotateAround(mGKOwner->GetPosition(), Vector3::UnitY, angleOfRotation);
        }
        else
        {
            meshTransform->Rotate(Vector3::UnitY, angleOfRotation);
        }
    }
    else
    {
        doneTurning = true;
    }
    return doneTurning;
}

Quaternion Walker::GetTurnAmount(float deltaTime, const Vector3& currentDir, const Vector3& desiredDir, float turnSpeed, int turnDir)
{
    // Determine angle between current and desired.
    float angle = Math::Acos(Vector3::Dot(currentDir, desiredDir));
    std::cout << "Current angle: " << Math::ToDegrees(angle) << std::endl;
    
    // If angle is less than the "at heading radians" already, no need to turn at all.
    if(angle <= kAtHeadingRadians)
    {
        return Quaternion::Identity;
    }
    
    // Determine how much to rotate this frame.
    // If it's enough to where angle is less than the "at heading" rotation, just set heading exactly.
    float rotateAngle = turnSpeed * deltaTime;
    if(angle - rotateAngle <= kAtHeadingRadians)
    {
        rotateAngle = angle;
    }
    
    // Determine which way to turn.
    // Sometimes the caller already knows the desired turn direction, in which case we'll just use that.
    // But otherwise, use cross product method to determine clockwise/counter-clockwise.
    if(turnDir == 0)
    {
        // If y-axis is zero, it means vectors are parallel (either exactly facing or exactly NOT facing).
        // In that case, 1.0f default is fine. Otherwise, we want either 1.0f or -1.0f.
        Vector3 cross = Vector3::Cross(currentDir, desiredDir);
        turnDir = cross.y >= 0.0f ? 1 : -1;
    }
    
    // Factor direction into angle.
    rotateAngle *= turnDir;
    
    // Create quat and return it.
    return Quaternion(Vector3::UnitY, rotateAngle);
}
