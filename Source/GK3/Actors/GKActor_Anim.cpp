#include "GKActor.h"

#include "CharacterManager.h"
#include "Debug.h"
#include "GasPlayer.h"
#include "MeshRenderer.h"
#include "VertexAnimator.h"

void GKActor::StartAnimation(VertexAnimation* anim, int framesPerSecond, bool allowMove, float time, bool fromGas)
{
    // Don't let a GAS anim override a non-GAS anim.
    if(fromGas && mVertexAnimator->IsPlaying()) { return; }
    
    // If this is not a GAS anim, pause any running GAS.
    if(!fromGas)
    {
        mGasPlayer->Pause();
    }
    
    // Stop any playing animation before starting a new one.
    if(mVertexAnimator->IsPlaying())
    {
        mVertexAnimator->Stop(nullptr);
    }
    
    // Start the animation.
    // Note that this will sample the first frame of the animation, updating the model's positions/rotations.
    mVertexAnimator->Start(anim, framesPerSecond, std::bind(&GKActor::OnVertexAnimationStopped, this), time);
    
    // Animations start from actor's position/rotation.
    // So, make sure the mesh is aligned to that transform.
    SyncMeshTransformToActor(anim);
    
    // In GK3, a "move" anim is one that is allowed to move the actor. This is like "root motion" in modern engines.
    // When "move" anim ends, the actor/mesh stay where they are. For "non-move" anims, actor/mesh revert to initial pos/rot.
    // Interestingly, the actor still moves with the model during non-move anims...it just reverts at the end.
    mVertexAnimAllowMove = allowMove;
    if(!allowMove)
    {
        // Save start pos/rot for the actor, so it can be reverted.
        mStartVertexAnimPosition = GetPosition();
        mStartVertexAnimRotation = GetRotation();
        
        // Save start pos/rot for the mesh, so it can be reverted.
        mStartVertexAnimMeshPos = GetMeshPosition();
        mStartVertexAnimMeshRotation = GetMeshRotation();
        
        //
        mLastMeshPos = mStartVertexAnimMeshPos;
        mLastMeshRotation = mStartVertexAnimMeshRotation;
    }
}

void GKActor::StartAbsoluteAnimation(VertexAnimation* anim, int framesPerSecond, Vector3 pos, Heading heading, float time, bool fromGas)
{
    // Don't let a GAS anim override a non-GAS anim.
    if(fromGas && mVertexAnimator->IsPlaying()) { return; }
    
    // Absolute anims are always move anims?
    mVertexAnimAllowMove = true;
    
    // Set the 3D model's position and heading.
    mMeshActor->SetPosition(pos);
    mMeshActor->SetRotation(heading.ToQuaternion());
    
    // If this is not a GAS anim, pause any running GAS.
    if(!fromGas)
    {
        mGasPlayer->Pause();
    }
    
    // Start the animation.
    mVertexAnimator->Start(anim, framesPerSecond, std::bind(&GKActor::OnVertexAnimationStopped, this), time);
}

void GKActor::StopAnimation(VertexAnimation* anim)
{
    // Vertex animator will only stop if specified anim is currently playing.
    // But keep in mind passing null will stop any anim!
    mVertexAnimator->Stop(anim);
}

void GKActor::SampleAnimation(VertexAnimation* anim, int frame)
{
    mVertexAnimator->Sample(anim, frame);
}

void GKActor::OnVertexAnimationStopped()
{
    // Make sure actor is at mesh position at end of animation.
    SyncActorTransformToMesh();
    
    // On anim stop, if vertex anim is not allowed to move actor position,
    // we must revert actor back to position when anim started.
    if(!mVertexAnimAllowMove)
    {
        // Move actor back to start position/rotation.
        SetPosition(mStartVertexAnimPosition);
        SetRotation(mStartVertexAnimRotation);
        
        // Do the same for the mesh, though the mesh is a bit harder.
        // Basically, determine diff from start of anim and revert that.
        Vector3 currentToStart = mStartVertexAnimMeshPos - GetMeshPosition();
        mMeshActor->GetTransform()->Translate(currentToStart);
        
        Quaternion meshRotation = GetMeshRotation();
        Quaternion diff = Quaternion::Diff(mStartVertexAnimMeshRotation, meshRotation);
        mMeshActor->GetTransform()->RotateAround(GetPosition(), diff);
        
        if(mWalkerDOR != nullptr)
        {
            mWalkerDOR->SetPosition(mMeshActor->GetPosition());
            mWalkerDOR->SetRotation(mMeshActor->GetRotation());
        }
    }
    
    mLastMeshPos = GetMeshPosition();
    mLastMeshRotation = GetMeshRotation();
}

void GKActor::SyncMeshTransformToActor(VertexAnimation* anim)
{
    // If there's no character config, we can't do any crazy hip-related math.
    // So, pretty straightforward in that case.
    if(mCharConfig == nullptr)
    {
        mMeshActor->SetPosition(GetPosition());
    }
    else
    {
        // If an anim is provided, sample the hip position/matrix from that animation.
        // Otherwise, just grab the hip position/matrix from the mesh renderer.
        Vector3 hipMeshPos;
        Matrix4 meshToLocalMatrix;
        if(anim != nullptr)
        {
            // Note that the "frames per second" (set to 15 here) is pretty meaningless b/c we're only sampling time = 0.0f anyway.
            hipMeshPos = anim->SampleVertexPosition(0.0f, 15, mCharConfig->hipAxesMeshIndex, mCharConfig->hipAxesGroupIndex, mCharConfig->hipAxesPointIndex);
            meshToLocalMatrix = anim->SampleTransformPose(0.0f, 15, mCharConfig->hipAxesMeshIndex).GetMeshToLocalMatrix();
        }
        else
        {
            hipMeshPos = mMeshRenderer->GetMesh(mCharConfig->hipAxesMeshIndex)->GetSubmesh(mCharConfig->hipAxesGroupIndex)->GetVertexPosition(mCharConfig->hipAxesPointIndex);
            meshToLocalMatrix = mMeshRenderer->GetMesh(mCharConfig->hipAxesMeshIndex)->GetMeshToLocalMatrix();
        }
        
        // Convert hip pos to world space. Transform point using mesh->world matrix.
        Vector3 worldHipPos = (mMeshActor->GetTransform()->GetLocalToWorldMatrix() * meshToLocalMatrix).TransformPoint(hipMeshPos);
        
        // The hips are not right at the mesh actor's position. So, calculate offset from hips to mesh actor position.
        // Y-component is zeroed out because we don't want this to change the height, just X/Z plane position.
        Vector3 hipPosToActor = mMeshActor->GetPosition() - worldHipPos;
        hipPosToActor.y = 0.0f;
        
        // Set mesh actor to the actor's position.
        // HOWEVER, again, mesh's actual position (represented by hip pos) may be offset (by quite a large amount) from the mesh actor's position!
        // So that's what we apply the hipToActor offset.
        mMeshActor->SetPosition(GetPosition() + hipPosToActor);
    }
    
    // Move mesh to actor's rotation.
    Quaternion rotation = GetRotation();
    if(mActorType == ActorType::Actor)
    {
        rotation *= Quaternion(Vector3::UnitY, Math::kPi);
    }
    mMeshActor->SetRotation(rotation);
    
    // 
    if(mWalkerDOR != nullptr)
    {
        mWalkerDOR->SetPosition(mMeshActor->GetPosition());
        mWalkerDOR->SetRotation(mMeshActor->GetRotation());
    }
}

void GKActor::SyncActorTransformToMesh()
{
    Vector3 meshPosition = GetMeshPosition();
    Vector3 meshPositionChange = meshPosition - mLastMeshPos;
    GetTransform()->Translate(meshPositionChange);
    mLastMeshPos = GetMeshPosition();
    
    Quaternion meshRotation = GetMeshRotation();
    Quaternion meshRotationChange = Quaternion::Diff(meshRotation, mLastMeshRotation);
    GetTransform()->Rotate(meshRotationChange);
    mLastMeshRotation = meshRotation;
    
    /*
    // Update actor's position to match the mesh's position.
    // Mesh's position is the mesh's hip position, but with height value removed/zero'd.
    Vector3 meshPos = GetMeshPosition();
    SetPosition(meshPos);
    
    // Update actor's rotation to match the mesh's rotation.
    // Use hip direction, but zero out any y-component.
    Quaternion meshRotation = GetMeshRotation();
    if(mActorType == ActorType::Actor)
    {
        meshRotation *= Quaternion(Vector3::UnitY, Math::kPi);
    }
    SetRotation(meshRotation);
    */
}

Vector3 GKActor::GetMeshPosition()
{
    if(mCharConfig == nullptr) { return mMeshActor->GetPosition(); }
    
    // Get hip pos and convert to world space.
    // To do this, multiply the mesh->local with local->world to get a mesh->world matrix for transforming.
    Vector3 meshHipPos = mMeshRenderer->GetMesh(mCharConfig->hipAxesMeshIndex)->GetSubmesh(mCharConfig->hipAxesGroupIndex)->GetVertexPosition(mCharConfig->hipAxesPointIndex);
    Vector3 worldHipPos = (mMeshActor->GetTransform()->GetLocalToWorldMatrix() * mMeshRenderer->GetMesh(mCharConfig->hipAxesMeshIndex)->GetMeshToLocalMatrix()).TransformPoint(meshHipPos);
    worldHipPos.y = mMeshActor->GetTransform()->GetLocalToWorldMatrix().TransformPoint(Vector3::Zero).y;
    return worldHipPos;
}

Quaternion GKActor::GetMeshRotation()
{
    if(mCharConfig == nullptr) { return mMeshActor->GetRotation(); }
    Matrix4 hipMeshToWorldMatrix = mMeshActor->GetTransform()->GetLocalToWorldMatrix() * mMeshRenderer->GetMesh(mCharConfig->hipAxesMeshIndex)->GetMeshToLocalMatrix();
    return hipMeshToWorldMatrix.GetRotation();
}
