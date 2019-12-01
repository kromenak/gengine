//
// AnimationNodes.cpp
//
// Clark Kromenaker
//
#include "AnimationNodes.h"

#include "Animation.h"
#include "FaceController.h"
#include "FootstepManager.h"
#include "GKActor.h"
#include "GKObject.h"
#include "Heading.h"
#include "MeshRenderer.h"
#include "Services.h"
#include "Scene.h"

void VertexAnimNode::Play(Animation* anim)
{
	if(vertexAnimation != nullptr)
	{
		GKObject* object = GEngine::inst->GetScene()->GetSceneObjectByModelName(vertexAnimation->GetModelName());
		if(object != nullptr)
		{
			// Absolute anims must set the object's position and heading before playing.
			if(absolute)
			{
				object->SetPosition(position - offsetFromOrigin);
				object->SetHeading(Heading::FromDegrees(heading - headingFromOrigin));
			}
			
			if(anim != nullptr)
			{
				object->PlayAnimation(vertexAnimation, anim->GetFramesPerSecond());
			}
			else
			{
				object->PlayAnimation(vertexAnimation);
			}
		}
	}
}

void VertexAnimNode::Sample(Animation* anim, int frame)
{
	if(vertexAnimation != nullptr)
	{
		GKObject* object = GEngine::inst->GetScene()->GetSceneObjectByModelName(vertexAnimation->GetModelName());
		if(object != nullptr)
		{
			object->SampleAnimation(vertexAnimation, frame);
		}
	}
}

void SceneTextureAnimNode::Play(Animation* anim)
{
	Texture* texture = Services::GetAssets()->LoadTexture(textureName);
	if(texture != nullptr)
	{
		//TODO: Ensure sceneName matches loaded scene name?
		GEngine::inst->GetScene()->ApplyTextureToSceneModel(sceneModelName, texture);
	}
}

void SceneModelVisibilityAnimNode::Play(Animation* anim)
{
	//TODO: Ensure sceneName matches loaded scene name?
	GEngine::inst->GetScene()->SetSceneModelVisibility(sceneModelName, visible);
}

void ModelTextureAnimNode::Play(Animation* anim)
{
	// Get actor by model name.
	GKObject* object = GEngine::inst->GetScene()->GetSceneObjectByModelName(modelName);
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

void ModelVisibilityAnimNode::Play(Animation* anim)
{
	// Get actor by model name.
	GKObject* object = GEngine::inst->GetScene()->GetSceneObjectByModelName(modelName);
	if(object != nullptr)
	{
		//TODO: Not sure if models need to be invisible but still updating in this scenario.
		//For now, I'll just disable or enable the actor entirely.
		object->SetActive(visible);
	}
}

void SoundAnimNode::Play(Animation* anim)
{
	//TODO: Flesh this out
	if(audio != nullptr)
	{
		Services::GetAudio()->Play(audio);
	}
}

void FootstepAnimNode::Play(Animation* anim)
{
	// Get actor using the specified noun.
	GKActor* actor = GEngine::inst->GetScene()->GetActorByNoun(actorNoun);
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

void FootscuffAnimNode::Play(Animation* anim)
{
	// Get actor using the specified noun.
	GKActor* actor = GEngine::inst->GetScene()->GetActorByNoun(actorNoun);
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

void PlaySoundtrackAnimNode::Play(Animation* anim)
{
	std::cout << "PLAY " << soundtrackName << std::endl;
}

void StopSoundtrackAnimNode::Play(Animation* anim)
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

void CameraAnimNode::Play(Animation* anim)
{
	std::cout << "MOVE CAMERA TO " << cameraPositionName << std::endl;
}

void FaceTexAnimNode::Play(Animation* anim)
{
	// Get actor using the specified noun.
	GKActor* actor = GEngine::inst->GetScene()->GetActorByNoun(actorNoun);
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

void UnFaceTexAnimNode::Play(Animation* anim)
{
	// Get actor using the specified noun.
	GKActor* actor = GEngine::inst->GetScene()->GetActorByNoun(actorNoun);
	if(actor != nullptr)
	{
		actor->GetFaceController()->Clear(faceElement);
	}
}

void LipSyncAnimNode::Play(Animation* anim)
{
	// Get actor using the specified noun.
	GKActor* actor = GEngine::inst->GetScene()->GetActorByNoun(actorNoun);
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

void GlanceAnimNode::Play(Animation* anim)
{
	std::cout << actorNoun << " GLANCE AT " << position << std::endl;
}

void MoodAnimNode::Play(Animation* anim)
{
	std::cout << actorNoun << " IN MOOD " << moodName << std::endl;
}
