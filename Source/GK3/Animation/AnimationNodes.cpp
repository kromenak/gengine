#include "AnimationNodes.h"

#include "ActionManager.h"
#include "Animation.h"
#include "Animator.h"
#include "AssetManager.h"
#include "CharacterManager.h"
#include "DialogueManager.h"
#include "FaceController.h"
#include "FootstepManager.h"
#include "GK3UI.h"
#include "GKActor.h"
#include "Heading.h"
#include "MeshRenderer.h"
#include "SceneManager.h"
#include "SoundtrackPlayer.h"
#include "Texture.h"
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
		GKObject* obj = gSceneManager.GetScene()->GetSceneObjectByModelName(vertexAnimation->GetModelName());
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

            // If not an absolute anim, see if the animation's name is prefixed by the name of a model in the scene.
            // If so...we will treat that model as our parent (so, we move/rotate to match them).
            // (This feels like a HACK, but the original game also does this parenting, but the assets themselves have no flags/indicators about it.)
            // (So, they must do it in some similar way to this, unless I'm missing something?)
            if(!absolute)
            {
                // e.g. Grab the "ROX" from "ROX_SPRAYANDWIPE2.ANM".
                std::string prefix = animState->params.animation->GetName().substr(0, 3);

                // If a parent exists, and it isn't ourselves, use it!
                GKObject* parent = gSceneManager.GetScene()->GetSceneObjectByModelName(prefix);
                if(parent != nullptr && parent != obj)
                {
                    params.parent = parent->GetMeshRenderer()->GetOwner();
                    //printf("Found parent %s for %s\n", parent->GetName().c_str(), obj->GetName().c_str());
                }
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

void VertexAnimNode::Sample(int frame)
{
    if(vertexAnimation != nullptr)
    {
        GKObject* obj = gSceneManager.GetScene()->GetSceneObjectByModelName(vertexAnimation->GetModelName());
        if(obj != nullptr)
        {
            VertexAnimParams params;
            params.vertexAnimation = vertexAnimation;
            //params.framesPerSecond?

            // If this is an absolute anim, calculate the position/heading to set the model actor to when the anim plays.
            params.absolute = absolute;
            if(absolute)
            {
                params.absolutePosition = CalcAbsolutePosition();
                params.absoluteHeading = Heading::FromDegrees(absoluteWorldToModelHeading - absoluteModelToActorHeading);
            }

            // Move anims allow the actor associated with the model to stay in its final position when the animation ends, instead of reverting.
            // Absolute anims are always "move anims".
            params.allowMove = absolute;

            // Sample the anim.
            obj->SampleAnimation(params, frame);
        }
    }
}

void VertexAnimNode::Stop()
{
	if(vertexAnimation != nullptr)
	{
		GKObject* obj = gSceneManager.GetScene()->GetSceneObjectByModelName(vertexAnimation->GetModelName());
		if(obj != nullptr)
		{
            obj->StopAnimation(vertexAnimation);
		}
	}
}

bool VertexAnimNode::AppliesToModel(const std::string& modelName)
{
    return vertexAnimation != nullptr && StringUtil::EqualsIgnoreCase(modelName, vertexAnimation->GetModelName());
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
	Texture* texture = gAssetManager.LoadSceneTexture(textureName, AssetScope::Scene);
	if(texture != nullptr)
	{
		//TODO: Ensure sceneName matches loaded scene name?
		gSceneManager.GetScene()->ApplyTextureToSceneModel(sceneModelName, texture);
	}
}

void SceneTextureAnimNode::Sample(int frame)
{
    Play(nullptr);
}

void SceneModelVisibilityAnimNode::Play(AnimationState* animState)
{
	//TODO: Ensure sceneName matches loaded scene name?
	gSceneManager.GetScene()->SetSceneModelVisibility(sceneModelName, visible);
}

void SceneModelVisibilityAnimNode::Sample(int frame)
{
    Play(nullptr);
}

void ModelTextureAnimNode::Play(AnimationState* animState)
{
	// Get actor by model name.
	GKObject* obj = gSceneManager.GetScene()->GetSceneObjectByModelName(modelName);
	if(obj != nullptr)
	{
		// Grab the material used to render this meshIndex/submeshIndex pair.
		Material* material = obj->GetMeshRenderer()->GetMaterial(meshIndex, submeshIndex);
		if(material != nullptr)
		{
			// Apply the texture to that material.
			Texture* texture = gAssetManager.LoadSceneTexture(textureName, AssetScope::Scene);
			if(texture != nullptr)
			{
				material->SetDiffuseTexture(texture);
			}
		}
	}
}

void ModelTextureAnimNode::Sample(int frame)
{
    Play(nullptr);
}

void ModelVisibilityAnimNode::Play(AnimationState* animState)
{
	// Get actor by model name.
	GKObject* obj = gSceneManager.GetScene()->GetSceneObjectByModelName(modelName);
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

void ModelVisibilityAnimNode::Sample(int frame)
{
    Play(nullptr);
}

void SoundAnimNode::Play(AnimationState* animState)
{
    // Don't play new sounds during action skip.
    if(gActionManager.IsSkippingCurrentAction()) { return; }

    // Create play audio params struct.
    PlayAudioParams playParams;
    playParams.audio = audio;
    playParams.audioType = animState->params.isYak ? AudioType::VO : AudioType::SFX;

    // Volume is specified as 0-100, but audio system expects 0.0-1.0.
    playParams.volume = volume * 0.01f;

    // If 3D, do a bit more work to determine position.
    playParams.is3d = is3d;
    if(is3d)
    {
        // Use specified position by default.
        Vector3 playPosition = position;

        // If position is based on model name, find the model and set position.
        if(!modelName.empty())
        {
            GKObject* obj = gSceneManager.GetScene()->GetSceneObjectByModelName(modelName);
            if(obj != nullptr)
            {
                playPosition = obj->GetAudioPosition();
            }
        }

        playParams.position = playPosition;
        playParams.minDist = minDistance;
        playParams.maxDist = maxDistance;
    }

    gAudioManager.Play(playParams);
}

namespace
{
    void PlayFootSound(bool scuff, GKActor* actor)
    {
        // Get the actor's shoe type.
        std::string shoeType = actor->GetShoeType();

        // Query the texture used on the floor where the actor is walking.
        Texture* floorTexture = actor->GetFloorTypeWalkingOn();
        std::string floorTextureName = floorTexture != nullptr ? floorTexture->GetNameNoExtension() : "carpet1";

        // Get the footstep sound.
        Audio* footAudio = scuff ? gFootstepManager.GetFootscuff(shoeType, floorTextureName)
            : gFootstepManager.GetFootstep(shoeType, floorTextureName);
        if(footAudio != nullptr)
        {
            // Play the sound at the actor's world position, which is near their feet anyways.
            // The min/max distances for footsteps are derived from experimenting with the original game.
            const float kMinFootstepDist = 60.0f;
            const float kMaxFootstepDist = 800.0f;

            PlayAudioParams playParams;
            playParams.audio = footAudio;
            playParams.audioType = AudioType::SFX;

            playParams.is3d = true;
            playParams.position = actor->GetWorldPosition();
            playParams.minDist = kMinFootstepDist;
            playParams.maxDist = kMaxFootstepDist;
            gAudioManager.Play(playParams);
        }
    }
}

void FootstepAnimNode::Play(AnimationState* animState)
{
    if(gActionManager.IsSkippingCurrentAction()) { return; }

	// Get actor using the specified noun.
	GKActor* actor = gSceneManager.GetScene()->GetActorByNoun(actorNoun);
	if(actor != nullptr)
	{
        // Play a footstep.
        PlayFootSound(false, actor);
	}
}

void FootscuffAnimNode::Play(AnimationState* animState)
{
    if(gActionManager.IsSkippingCurrentAction()) { return; }

	// Get actor using the specified noun.
	GKActor* actor = gSceneManager.GetScene()->GetActorByNoun(actorNoun);
	if(actor != nullptr)
	{
        // Play a foot scuff.
        PlayFootSound(true, actor);
	}
}

void PlaySoundtrackAnimNode::Play(AnimationState* animState)
{
    Scene* scene = gSceneManager.GetScene();
    if(scene == nullptr) { return; }

    SoundtrackPlayer* soundtrackPlayer = scene->GetSoundtrackPlayer();
    if(soundtrackPlayer == nullptr) { return; }

    Soundtrack* soundtrack = gAssetManager.LoadSoundtrack(soundtrackName, AssetScope::Scene);
    if(soundtrack == nullptr) { return; }
    soundtrackPlayer->Play(soundtrack);
}

void StopSoundtrackAnimNode::Play(AnimationState* animState)
{
    Scene* scene = gSceneManager.GetScene();
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
    if(glide)
    {
        gSceneManager.GetScene()->GlideToCameraPosition(cameraPositionName, nullptr);
    }
    else
    {
        gSceneManager.GetScene()->SetCameraPosition(cameraPositionName);
    }
}

void CameraAnimNode::Sample(int frame)
{
    Play(nullptr);
}

void FaceTexAnimNode::Play(AnimationState* animState)
{
	// Get actor using the specified noun.
	GKActor* actor = gSceneManager.GetScene()->GetActorByNoun(actorNoun);
	if(actor != nullptr)
	{
		// In this case, the texture name is what it is.
		Texture* texture = gAssetManager.LoadTexture(textureName);
		if(texture != nullptr)
		{
			actor->GetFaceController()->Set(faceElement, texture);
		}
	}
}

void FaceTexAnimNode::Sample(int frame)
{
    Play(nullptr);
}

void UnFaceTexAnimNode::Play(AnimationState* animState)
{
	// Get actor using the specified noun.
	GKActor* actor = gSceneManager.GetScene()->GetActorByNoun(actorNoun);
	if(actor != nullptr)
	{
		actor->GetFaceController()->Clear(faceElement);
	}
}

void UnFaceTexAnimNode::Sample(int frame)
{
    Play(nullptr);
}

void LipSyncAnimNode::Play(AnimationState* animState)
{
	// Get actor using the specified noun.
	GKActor* actor = gSceneManager.GetScene()->GetActorByNoun(actorNoun);
	if(actor != nullptr)
	{
		// The mouth texture name is based on the current face config for the character.
		Texture* mouthTexture = gAssetManager.LoadTexture(actor->GetConfig()->faceConfig->identifier + "_" + mouthTextureName);
		if(mouthTexture != nullptr)
		{
			actor->GetFaceController()->SetMouth(mouthTexture);
		}
	}
}

void LipSyncAnimNode::Sample(int frame)
{
    Play(nullptr);
}

void GlanceAnimNode::Play(AnimationState* animState)
{
	std::cout << actorNoun << " GLANCE AT " << position << std::endl;
}

void GlanceAnimNode::Sample(int frame)
{
    Play(nullptr);
}

void MoodAnimNode::Play(AnimationState* animState)
{
    GKActor* actor = gSceneManager.GetScene()->GetActorByNoun(actorNoun);
    if(actor != nullptr)
    {
        actor->GetFaceController()->SetMood(moodName);
    }
}

void MoodAnimNode::Sample(int frame)
{
    Play(nullptr);
}

void ExpressionAnimNode::Play(AnimationState* animState)
{
    GKActor* actor = gSceneManager.GetScene()->GetActorByNoun(actorNoun);
    if(actor != nullptr)
    {
        actor->GetFaceController()->DoExpression(expressionName);
    }
}

void ExpressionAnimNode::Sample(int frame)
{
    Play(nullptr);
}

void SpeakerAnimNode::Play(AnimationState* animState)
{
	gDialogueManager.SetSpeaker(actorNoun);
}

void SpeakerAnimNode::Sample(int frame)
{
    Play(nullptr);
}

void CaptionAnimNode::Play(AnimationState* animState)
{
    gGK3UI.AddCaption(caption, gDialogueManager.GetSpeaker());
}

void CaptionAnimNode::Sample(int frame)
{
    Play(nullptr);
}

void SpeakerCaptionAnimNode::Play(AnimationState* animState)
{
    // Add the caption.
    gGK3UI.AddCaption(caption, speaker);

    // Calculate duration in seconds.
    // We know how many frames the caption should be up, so multiply that by seconds per frame.
    int frameCount = endFrameNumber - frameNumber;
    float duration = frameCount * animState->params.animation->GetFrameDuration();
    gGK3UI.FinishCaption(duration);
}

void SpeakerCaptionAnimNode::Sample(int frame)
{
    Play(nullptr);
}

void DialogueCueAnimNode::Play(AnimationState* animState)
{
	gDialogueManager.TriggerDialogueCue();
    gGK3UI.FinishCaption();
}

void DialogueCueAnimNode::Sample(int frame)
{
    Play(nullptr);
}

void DialogueAnimNode::Play(AnimationState* animState)
{
    //TODO: Unsure if "numLines" and "playFidgets" are correct here.
    gDialogueManager.StartDialogue(licensePlate, 1, false, nullptr);
}

void DialogueAnimNode::Sample(int frame)
{
    Play(nullptr);
}