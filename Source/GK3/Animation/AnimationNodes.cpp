#include "AnimationNodes.h"

#include "Animation.h"
#include "Animator.h"
#include "DialogueManager.h"
#include "FaceController.h"
#include "FootstepManager.h"
#include "GKActor.h"
#include "GKActor.h"
#include "Heading.h"
#include "MeshRenderer.h"
#include "Scene.h"
#include "Services.h"
#include "SoundtrackPlayer.h"
#include "VertexAnimation.h"
#include "VertexAnimator.h"

void VertexAnimNode::Play(AnimationState* animState)
{
	// Make sure anim state and anim are valid - we need those.
	if(animState == nullptr || animState->params.animation == nullptr) { return; }
	
	// Make sure we have a vertex anim to play...
	if(vertexAnimation != nullptr)
	{
		// Also we need the object to play the vertex anim on!
		GKObject* obj = GEngine::Instance()->GetScene()->GetSceneObjectByModelName(vertexAnimation->GetModelName());
		if(obj != nullptr)
		{
            VertexAnimParams params;
            params.vertexAnimation = vertexAnimation;
            params.framesPerSecond = animState->params.animation->GetFramesPerSecond();
            
            // This logic is a bit tricky/complicated, but it is needed to support starting anims at different times.
            // Usually, we start at t=0, but if executing frame 0, but we are on frame 20, we need to "catch up" by setting starting time for frame 20.
            params.startTime = 0.0f;
            if(animState->executingFrame < animState->currentFrame)
            {
                params.startTime = static_cast<float>(animState->currentFrame - animState->executingFrame) / params.framesPerSecond;
            }
            
            // The animator may update not exactly at the time interval this frame should have executed.
            // If we're already a fraction of time into the current frame, take that into account for smoother animations.
            params.startTime += animState->timer;

            // If this is an absolute anim, calculate the position/heading to set the model actor to when the anim plays.
            params.absolute = absolute;
            if(absolute)
            {
                params.absolutePosition = CalcAbsolutePosition();
                params.absoluteHeading = Heading::FromDegrees(absoluteWorldToModelHeading - absoluteModelToActorHeading);
            }

            // Move anims allow the actor associated with the model to stay in its final position when the animation ends, instead of reverting.
            // Absolute anims are always "move anims".
            params.allowMove = animState->params.allowMove || absolute;

            // Keep track of whether this is an autoscript anim.
            // This is mainly b/c autoscript anims are lower priority than other anims.
            params.fromAutoScript = animState->params.fromAutoScript;
            
            // Start the anim.
            obj->StartAnimation(params);
		}
	}
}

void VertexAnimNode::Stop()
{
	if(vertexAnimation != nullptr)
	{
		GKObject* obj = GEngine::Instance()->GetScene()->GetSceneObjectByModelName(vertexAnimation->GetModelName());
		if(obj != nullptr)
		{
            obj->StopAnimation(vertexAnimation);
		}
	}
}

void VertexAnimNode::Sample(Animation* anim, int frame)
{
	if(vertexAnimation != nullptr)
	{
        GKObject* obj = GEngine::Instance()->GetScene()->GetSceneObjectByModelName(vertexAnimation->GetModelName());
		if(obj != nullptr)
		{
            VertexAnimParams params;
            params.vertexAnimation = vertexAnimation;
            obj->SampleAnimation(params, frame);
		}
	}
}

Vector3 VertexAnimNode::CalcAbsolutePosition()
{
    // Remember, when playing an absolute animation, the model actor's origin IS NOT necessarily equal to the model's position!
    // This depends on how the animation was authored.
    //
    // To calculate the absolute position, start at world origin, add "world to model" offset.
    // Then, use the "world to model" heading to ROTATE the "model to actor" offset, and then add that to the previous position.
    // And...you got your spot!
    Quaternion modelToActorRot(Vector3::UnitY, Math::ToRadians(absoluteWorldToModelHeading));
    return absoluteWorldToModelOffset + modelToActorRot.Rotate(absoluteModelToActorOffset);
}

void SceneTextureAnimNode::Play(AnimationState* animState)
{
	Texture* texture = Services::GetAssets()->LoadSceneTexture(textureName);
	if(texture != nullptr)
	{
		//TODO: Ensure sceneName matches loaded scene name?
		GEngine::Instance()->GetScene()->ApplyTextureToSceneModel(sceneModelName, texture);
	}
}

void SceneModelVisibilityAnimNode::Play(AnimationState* animState)
{
	//TODO: Ensure sceneName matches loaded scene name?
	GEngine::Instance()->GetScene()->SetSceneModelVisibility(sceneModelName, visible);
}

void ModelTextureAnimNode::Play(AnimationState* animState)
{
	// Get actor by model name.
	GKObject* obj = GEngine::Instance()->GetScene()->GetSceneObjectByModelName(modelName);
	if(obj != nullptr)
	{
		// Grab the material used to render this meshIndex/submeshIndex pair.
		Material* material = obj->GetMeshRenderer()->GetMaterial(meshIndex, submeshIndex);
		if(material != nullptr)
		{
			// Apply the texture to that material.
			Texture* texture = Services::GetAssets()->LoadSceneTexture(textureName);
			if(texture != nullptr)
			{
				material->SetDiffuseTexture(texture);
			}
		}
	}
}

void ModelVisibilityAnimNode::Play(AnimationState* animState)
{
	// Get actor by model name.
	GKObject* obj = GEngine::Instance()->GetScene()->GetSceneObjectByModelName(modelName);
	if(obj != nullptr)
	{
        MeshRenderer* meshRenderer = obj->GetMeshRenderer();
        if(meshRenderer != nullptr)
        {
            if(meshIndex >= 0 && submeshIndex >= 0)
            {
                // Toggle specific submesh visibility.
                meshRenderer->SetVisibility(meshIndex, submeshIndex, visible);

                // If we're toggling specific submeshes on/off, let's assume we want the object as a whole enabled...
                meshRenderer->SetEnabled(true);
                obj->SetActive(true);
            }
            else
            {
                obj->SetActive(visible);
            }
        }
	}
}

void SoundAnimNode::Play(AnimationState* animState)
{
    if(Services::Get<ActionManager>()->IsSkippingCurrentAction()) { return; }

    // This will hold playing sound instance.
    PlayingSoundHandle soundInstance;
    
    // If 3D, do a bit more work to determine position.
    if(is3D)
    {
        // Use specified position by default.
        Vector3 playPosition = position;
        
        // If position is based on model name, find the model and set position.
        if(!modelName.empty())
        {
            GKObject* obj = GEngine::Instance()->GetScene()->GetSceneObjectByModelName(modelName);
            if(obj != nullptr)
            {
                playPosition = obj->GetWorldPosition();
            }
        }
        
        if(animState->params.isYak)
        {
            soundInstance = Services::GetAudio()->PlayVO3D(audio, playPosition, minDistance, maxDistance);
        }
        else
        {
            soundInstance = Services::GetAudio()->PlaySFX3D(audio, playPosition, minDistance, maxDistance);
        }
    }
    else
    {
        if(animState->params.isYak)
        {
            soundInstance = Services::GetAudio()->PlayVO(audio);
        }
        else
        {
            soundInstance = Services::GetAudio()->PlaySFX(audio);
        }
    }
    
    // Set volume after sound is created.
    // Volume is 0-100, but audio system expects 0.0-1.0.
    soundInstance.SetVolume(volume * 0.01f);
}

void FootstepAnimNode::Play(AnimationState* animState)
{
    if(Services::Get<ActionManager>()->IsSkippingCurrentAction()) { return; }

	// Get actor using the specified noun.
	GKActor* actor = GEngine::Instance()->GetScene()->GetActorByNoun(actorNoun);
	if(actor != nullptr)
	{
		// Get the actor's shoe type.
		std::string shoeType = "Male Leather";
		
		// Query the texture used on the floor where the actor is walking.
		std::string floorTextureName = "carpet1";
		
		// Get the footstep sound.
		Audio* footstepAudio = Services::Get<FootstepManager>()->GetFootstep(shoeType, floorTextureName);
		if(footstepAudio != nullptr)
		{
            //TODO: May want to play the sound at the actor's foot position by querying model.
            Services::GetAudio()->PlaySFX3D(footstepAudio, actor->GetWorldPosition());
		}
	}
}

void FootscuffAnimNode::Play(AnimationState* animState)
{
    if(Services::Get<ActionManager>()->IsSkippingCurrentAction()) { return; }

    //TODO: Almost identical to Footstep node, so maybe they can be combined somehow.
	// Get actor using the specified noun.
	GKActor* actor = GEngine::Instance()->GetScene()->GetActorByNoun(actorNoun);
	if(actor != nullptr)
	{
		// Get the actor's shoe type.
		std::string shoeType = "Male Leather";
		
		// Query the texture used on the floor where the actor is walking.
		std::string floorTextureName = "carpet1";
		
		// Get the scuff sound.
		Audio* footscuffAudio = Services::Get<FootstepManager>()->GetFootscuff(shoeType, floorTextureName);
		if(footscuffAudio != nullptr)
		{
            //TODO: May want to play the sound at the actor's foot position by querying model.
            Services::GetAudio()->PlaySFX3D(footscuffAudio, actor->GetWorldPosition());
        }
	}
}

void PlaySoundtrackAnimNode::Play(AnimationState* animState)
{
    Scene* scene = GEngine::Instance()->GetScene();
    if(scene == nullptr) { return; }

    SoundtrackPlayer* soundtrackPlayer = scene->GetSoundtrackPlayer();
    if(soundtrackPlayer == nullptr) { return; }

    Soundtrack* soundtrack = Services::GetAssets()->LoadSoundtrack(soundtrackName);
    if(soundtrack == nullptr) { return; }
    soundtrackPlayer->Play(soundtrack);
}

void StopSoundtrackAnimNode::Play(AnimationState* animState)
{
    Scene* scene = GEngine::Instance()->GetScene();
    if(scene == nullptr) { return; }

    SoundtrackPlayer* soundtrackPlayer = scene->GetSoundtrackPlayer();
    if(soundtrackPlayer == nullptr) { return; }

    // Either stop all soundtracks, or a specific one.
	if(soundtrackName.empty())
	{
        soundtrackPlayer->StopAll();
	}
	else
	{
        soundtrackPlayer->Stop(soundtrackName);
	}
}

void CameraAnimNode::Play(AnimationState* animState)
{
    GEngine::Instance()->GetScene()->SetCameraPosition(cameraPositionName);
}

void FaceTexAnimNode::Play(AnimationState* animState)
{
	// Get actor using the specified noun.
	GKActor* actor = GEngine::Instance()->GetScene()->GetActorByNoun(actorNoun);
	if(actor != nullptr)
	{
		// In this case, the texture name is what it is.
		Texture* texture = Services::GetAssets()->LoadTexture(textureName);
		if(texture != nullptr)
		{
			actor->GetFaceController()->Set(faceElement, texture);
		}
	}
}

void UnFaceTexAnimNode::Play(AnimationState* animState)
{
	// Get actor using the specified noun.
	GKActor* actor = GEngine::Instance()->GetScene()->GetActorByNoun(actorNoun);
	if(actor != nullptr)
	{
		actor->GetFaceController()->Clear(faceElement);
	}
}

void LipSyncAnimNode::Play(AnimationState* animState)
{
	// Get actor using the specified noun.
	GKActor* actor = GEngine::Instance()->GetScene()->GetActorByNoun(actorNoun);
	if(actor != nullptr)
	{
		// The mouth texture names need to have a prefix added, based on 3-letter model name.
		Texture* mouthTexture = Services::GetAssets()->LoadTexture(actor->GetMeshRenderer()->GetModelName() + "_" + mouthTextureName);
		if(mouthTexture != nullptr)
		{
			actor->GetFaceController()->SetMouth(mouthTexture);
		}
	}
}

void GlanceAnimNode::Play(AnimationState* animState)
{
	std::cout << actorNoun << " GLANCE AT " << position << std::endl;
}

void MoodAnimNode::Play(AnimationState* animState)
{
	std::cout << actorNoun << " IN MOOD " << moodName << std::endl;
}

void ExpressionAnimNode::Play(AnimationState* animState)
{
    std::cout << actorNoun << " HAS EXPRESSION " << expressionName << std::endl;
}

void SpeakerAnimNode::Play(AnimationState* animState)
{
	std::cout << "SPEAKER IS NOW " << actorNoun << std::endl;
	Services::Get<DialogueManager>()->SetSpeaker(actorNoun);
}

void CaptionAnimNode::Play(AnimationState* animState)
{
	std::cout << "CAPTION: " << caption << std::endl;
}

void SpeakerCaptionAnimNode::Play(AnimationState* animState)
{
	std::cout << "SPEAKER " << actorNoun << " w/ CAPTION: " << caption << std::endl;
}

void DialogueCueAnimNode::Play(AnimationState* animState)
{
	//std::cout << "DIALOGUE CUE" << std::endl;
	Services::Get<DialogueManager>()->TriggerDialogueCue();
}

void DialogueAnimNode::Play(AnimationState* animState)
{
    //TODO: Unsure if "numLines" and "useFidgets" are correct here.
    Services::Get<DialogueManager>()->StartDialogue(licensePlate, 1, false, nullptr);
}