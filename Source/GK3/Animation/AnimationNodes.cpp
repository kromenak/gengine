//
// AnimationNodes.cpp
//
// Clark Kromenaker
//
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
#include "Services.h"
#include "Scene.h"
#include "VertexAnimator.h"

void VertexAnimNode::Play(AnimationState* animState)
{
	// Make sure anim state and anim are valid - we need those.
	if(animState == nullptr || animState->animation == nullptr) { return; }
	
	// Make sure we have a vertex anim to play...
	if(vertexAnimation != nullptr)
	{
		// Also we need the object to play the vertex anim on!
		GKProp* obj = GEngine::Instance()->GetScene()->GetSceneObjectByModelName(vertexAnimation->GetModelName());
		if(obj != nullptr)
		{
            VertexAnimParams params;
            params.vertexAnimation = vertexAnimation;
            params.framesPerSecond = animState->animation->GetFramesPerSecond();
            params.startTime = animState->timer;
            params.absolute = absolute;
            if(absolute)
            {
                params.absolutePosition = position - offsetFromOrigin;
                params.absoluteHeading = Heading::FromDegrees(heading - headingFromOrigin);
            }
            params.allowMove = animState->allowMove || absolute; // absolute anims are always "move anims".
            params.fromAutoScript = animState->fromGas;
            
            // Start the anim.
            obj->StartAnimation(params);
		}
	}
}

void VertexAnimNode::Stop()
{
	if(vertexAnimation != nullptr)
	{
		GKProp* obj = GEngine::Instance()->GetScene()->GetSceneObjectByModelName(vertexAnimation->GetModelName());
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
		GKProp* obj = GEngine::Instance()->GetScene()->GetSceneObjectByModelName(vertexAnimation->GetModelName());
		if(obj != nullptr)
		{
            obj->SampleAnimation(vertexAnimation, frame);
		}
	}
}

void SceneTextureAnimNode::Play(AnimationState* animState)
{
	Texture* texture = Services::GetAssets()->LoadTexture(textureName);
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
	GKProp* obj = GEngine::Instance()->GetScene()->GetSceneObjectByModelName(modelName);
	if(obj != nullptr)
	{
		// Grab the material used to render this meshIndex/submeshIndex pair.
		Material* material = obj->GetMeshRenderer()->GetMaterial(meshIndex, submeshIndex);
		if(material != nullptr)
		{
			// Apply the texture to that material.
			Texture* texture = Services::GetAssets()->LoadTexture(textureName);
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
	GKProp* obj = GEngine::Instance()->GetScene()->GetSceneObjectByModelName(modelName);
	if(obj != nullptr)
	{
		//TODO: Not sure if models need to be invisible but still updating in this scenario.
		//For now, I'll just disable or enable the actor entirely.
        obj->SetActive(visible);
	}
}

void SoundAnimNode::Play(AnimationState* animState)
{
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
            GKProp* obj = GEngine::Instance()->GetScene()->GetSceneObjectByModelName(modelName);
            if(obj != nullptr)
            {
                playPosition = obj->GetWorldPosition();
            }
        }
        
        if(animState->isYak)
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
        if(animState->isYak)
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
	std::cout << "PLAY " << soundtrackName << std::endl;
}

void StopSoundtrackAnimNode::Play(AnimationState* animState)
{
	if(soundtrackName.empty())
	{
		std::cout << "STOP ALL SOUNDTRACKS" << std::endl;
	}
	else
	{
		std::cout << "STOP " << soundtrackName << std::endl;
	}
}

void CameraAnimNode::Play(AnimationState* animState)
{
	std::cout << "MOVE CAMERA TO " << cameraPositionName << std::endl;
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
		// The mouth texture names need to have a prefix added, based on 3-letter identifier.
		Texture* mouthTexture = Services::GetAssets()->LoadTexture(actor->GetIdentifier() + "_" + mouthTextureName);
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
	std::cout << "DIALOGUE CUE" << std::endl;
	Services::Get<DialogueManager>()->TriggerDialogueCue();
}
