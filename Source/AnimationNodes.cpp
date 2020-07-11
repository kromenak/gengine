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

void VertexAnimNode::Play(AnimationState* animState)
{
	// Make sure anim state and anim are valid - we need those.
	if(animState == nullptr || animState->animation == nullptr) { return; }
	
	// Make sure we have a vertex anim to play...
	if(vertexAnimation != nullptr)
	{
		// Also we need the object to play the vertex anim on!
		GKActor* actor = GEngine::Instance()->GetScene()->GetSceneObjectByModelName(vertexAnimation->GetModelName());
		if(actor != nullptr)
		{
			// Start absolute or relative anim.
			if(absolute)
			{
				actor->StartAbsoluteAnimation(vertexAnimation, animState->animation->GetFramesPerSecond(), position - offsetFromOrigin, Heading::FromDegrees(heading - headingFromOrigin), animState->timer, animState->fromGas);
			}
			else
			{
				actor->StartAnimation(vertexAnimation, animState->animation->GetFramesPerSecond(), animState->allowMove, animState->timer, animState->fromGas);
			}
		}
	}
}

void VertexAnimNode::Stop()
{
	if(vertexAnimation != nullptr)
	{
		GKActor* actor = GEngine::Instance()->GetScene()->GetSceneObjectByModelName(vertexAnimation->GetModelName());
		if(actor != nullptr)
		{
			actor->StopAnimation(vertexAnimation);
		}
	}
}

void VertexAnimNode::Sample(Animation* anim, int frame)
{
	if(vertexAnimation != nullptr)
	{
		GKActor* actor = GEngine::Instance()->GetScene()->GetSceneObjectByModelName(vertexAnimation->GetModelName());
		if(actor != nullptr)
		{
			actor->SampleAnimation(vertexAnimation, frame);
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
	GKActor* object = GEngine::Instance()->GetScene()->GetSceneObjectByModelName(modelName);
	if(object != nullptr)
	{
		// Grab the material used to render this meshIndex/submeshIndex pair.
		Material* material = object->GetMeshRenderer()->GetMaterial(meshIndex, submeshIndex);
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
	GKActor* object = GEngine::Instance()->GetScene()->GetSceneObjectByModelName(modelName);
	if(object != nullptr)
	{
		//TODO: Not sure if models need to be invisible but still updating in this scenario.
		//For now, I'll just disable or enable the actor entirely.
		object->SetActive(visible);
	}
}

void SoundAnimNode::Play(AnimationState* animState)
{
	//TODO: Flesh this out
	if(audio != nullptr)
	{
		Services::GetAudio()->Play(audio);
	}
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
			//TODO: Play at correct 3D position in the scene.
			Services::GetAudio()->Play(footstepAudio);
		}
	}
}

void FootscuffAnimNode::Play(AnimationState* animState)
{
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
			//TODO: Play at correct 3D position in the scene.
			Services::GetAudio()->Play(footscuffAudio);
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
