#include "SceneData.h"

#include "ActionManager.h"
#include "BSP.h"
#include "Services.h"
#include "StringUtil.h"
#include "Texture.h"
#include "WalkerBoundary.h"

SceneData::SceneData(const std::string& location, const std::string& timeblock) : mTimeblock(timeblock)
{
	// Load general and specific SIF assets.
	mGeneralSIF = Services::GetAssets()->LoadSIF(location);
	mSpecificSIF = Services::GetAssets()->LoadSIF(location + timeblock);
}

SceneData::~SceneData()
{
    if(mBSP != nullptr)
    {
        Services::GetAssets()->UnloadBSP(mBSP);
    }
}

const SceneActor* SceneData::DetermineWhoEgoWillBe() const
{
    // Try to ascertain who is ego from the specific timeblock SIF.
    const SceneActor* ego = nullptr;
    if(mSpecificSIF != nullptr)
    {
        ego = mSpecificSIF->FindCurrentEgo();
    }

    // If no specific timeblock SIF, or it just didn't contain any ego info, fall back to general SIF.
    if(ego == nullptr && mGeneralSIF != nullptr)
    {
        ego = mGeneralSIF->FindCurrentEgo();
    }
    return ego;
}

void SceneData::ResolveSceneData()
{
	// GENERAL
    // Take general block from general SIF to start.
    if(mGeneralSIF != nullptr)
    {
        mGeneralSettings = mGeneralSIF->FindCurrentGeneralBlock();
    }
	
	// If there's a specific SIF, also get that general block and merge with the other one.
	// Specific SIF settings override general SIF settings, if set.
	if(mSpecificSIF != nullptr)
	{
		GeneralBlock specificBlock = mSpecificSIF->FindCurrentGeneralBlock();
		mGeneralSettings.TakeOverridesFrom(specificBlock);
	}
	
	// Load the desired scene asset - chosen based on settings block.
	mSceneAsset = Services::GetAssets()->LoadSceneAsset(mGeneralSettings.sceneAssetName);
	
	// Load the BSP data, which is specified by the scene model.
	// If this is null, the game will still work...but there's no BSP geometry!
	if(mSceneAsset != nullptr)
	{
		mBSP = Services::GetAssets()->LoadBSP(mSceneAsset->GetBSPName());
	}
    else
    {
        mBSP = Services::GetAssets()->LoadBSP("DEFAULT.BSP");
    }
    
    // Load BSP lightmap data.
    mBSPLightmap = Services::GetAssets()->LoadBSPLightmap(mGeneralSettings.sceneAssetName);
    
    // Configure BSP, if we have one.
    if(mBSP != nullptr)
    {
        // Apply lightmap to BSP.
        if(mBSPLightmap != nullptr)
        {
            mBSP->ApplyLightmap(*mBSPLightmap);
        }

        // Save floor name in BSP. This enables easier querying of floor data.
        mBSP->SetFloorObjectName(mGeneralSettings.floorModelName);
    }
	
	// Figure out if we have a skybox, and set it to be rendered.
	// The skybox can be defined in any SIF or in the SceneAsset.
	// We'll give the SceneAsset priority, since most seem to be defined there.
	if(mSceneAsset != nullptr)
	{
		mSkybox = mSceneAsset->GetSkybox();
	}
	if(mSkybox == nullptr)
	{
		mSkybox = mGeneralSettings.CreateSkybox();
	}
	
	// Also figure out whether we have a walker boundary - if so, create one.
	if(!mGeneralSettings.walkerBoundaryTextureName.empty())
	{
        // Small thing, but since Texture class automatically uses magenta as a transparent color, clear that in this case.
        Texture* walkerTexture = Services::GetAssets()->LoadTexture(mGeneralSettings.walkerBoundaryTextureName);
        walkerTexture->ClearTransparentColor();

		mWalkerBoundary = new WalkerBoundary();
		mWalkerBoundary->SetTexture(walkerTexture);
		mWalkerBoundary->SetSize(mGeneralSettings.walkerBoundarySize);
		mWalkerBoundary->SetOffset(mGeneralSettings.walkerBoundaryOffset);
	}
	
	// Build list of actors to use in the scene based on contents of the two SIFs.
    if(mGeneralSIF != nullptr)
    {
        AddActorBlocks(mGeneralSIF->GetActorBlocks());
    }
	if(mSpecificSIF != nullptr)
	{
		AddActorBlocks(mSpecificSIF->GetActorBlocks());
	}
	
	// Build list of models to use in the scene based on contents of the two SIFS.
    if(mGeneralSIF != nullptr)
    {
        AddModelBlocks(mGeneralSIF->GetModelBlocks());
    }
	if(mSpecificSIF != nullptr)
	{
		AddModelBlocks(mSpecificSIF->GetModelBlocks());
	}
	
	// And so on...
    if(mGeneralSIF != nullptr)
    {
        AddPositionBlocks(mGeneralSIF->GetPositionBlocks());
    }
	if(mSpecificSIF != nullptr)
	{
		AddPositionBlocks(mSpecificSIF->GetPositionBlocks());
	}

    if(mGeneralSIF != nullptr)
    {
        AddInspectCameraBlocks(mGeneralSIF->GetInspectCameraBlocks());
    }
	if(mSpecificSIF != nullptr)
	{
		AddInspectCameraBlocks(mSpecificSIF->GetInspectCameraBlocks());
	}

    if(mGeneralSIF != nullptr)
    {
        AddRoomCameraBlocks(mGeneralSIF->GetRoomCameraBlocks());
    }
	if(mSpecificSIF != nullptr)
	{
		AddRoomCameraBlocks(mSpecificSIF->GetRoomCameraBlocks());
	}

    if(mGeneralSIF != nullptr)
    {
        AddCinematicCameraBlocks(mGeneralSIF->GetCinematicCameraBlocks());
    }
	if(mSpecificSIF != nullptr)
	{
		AddCinematicCameraBlocks(mSpecificSIF->GetCinematicCameraBlocks());
	}

    if(mGeneralSIF != nullptr)
    {
        AddDialogueCameraBlocks(mGeneralSIF->GetDialogueCameraBlocks());
    }
	if(mSpecificSIF != nullptr)
	{
		AddDialogueCameraBlocks(mSpecificSIF->GetDialogueCameraBlocks());
	}

    if(mGeneralSIF != nullptr)
    {
        AddTriggerBlocks(mGeneralSIF->GetTriggerBlocks());
    }
    if(mSpecificSIF != nullptr)
    {
        AddTriggerBlocks(mSpecificSIF->GetTriggerBlocks());
    }

    if(mGeneralSIF != nullptr)
    {
        AddSoundtrackBlocks(mGeneralSIF->GetSoundtrackBlocks());
    }
	if(mSpecificSIF != nullptr)
	{
		AddSoundtrackBlocks(mSpecificSIF->GetSoundtrackBlocks());
	}

    if(mGeneralSIF != nullptr)
    {
        AddConversationBlocks(mGeneralSIF->GetConversationBlocks());
    }
    if(mSpecificSIF != nullptr)
    {
        AddConversationBlocks(mSpecificSIF->GetConversationBlocks());
    }
	
	// Clear actions from previous scene - we're about to populate here!
	Services::Get<ActionManager>()->ClearActionSets();
	
	// Add inventory action sets first (global-to-specific).
	Services::Get<ActionManager>()->AddInventoryActionSets(mTimeblock);
	
	// Add current scene general SIF action sets conditionally (in order defined in SIF file).
    if(mGeneralSIF != nullptr)
    {
        AddActionBlocks(mGeneralSIF->GetActionBlocks(), true);
    }
	
	// Add current scene specific SIFs action sets unconditionally (in order defined in SIF file).
	if(mSpecificSIF != nullptr)
	{
		AddActionBlocks(mSpecificSIF->GetActionBlocks(), false);
	}
	
	// Add global action sets (global-to-specific).
	Services::Get<ActionManager>()->AddGlobalActionSets(mTimeblock);
}

const ScenePosition* SceneData::GetScenePosition(const std::string& positionName) const
{
    for(int i = 0; i < mPositions.size(); i++)
    {
        if(StringUtil::EqualsIgnoreCase(mPositions[i]->label, positionName))
        {
            return mPositions[i];
        }
    }
    return nullptr;
}

const SceneCamera* SceneData::GetInspectCamera(const std::string& nounOrModel) const
{
    for(const SceneCamera* camera : mInspectCameras)
    {
        if(StringUtil::EqualsIgnoreCase(camera->label, nounOrModel))
        {
            return camera;
        }
    }
    return nullptr;
}

const RoomSceneCamera* SceneData::GetRoomCamera(const std::string& cameraName) const
{
	for(int i = 0; i < mRoomCameras.size(); i++)
	{
		if(StringUtil::EqualsIgnoreCase(mRoomCameras[i]->label, cameraName))
		{
			return mRoomCameras[i];
		}
	}
	return nullptr;
}

const SceneCamera* SceneData::GetCinematicCamera(const std::string& cameraName) const
{
	for(int i = 0; i < mCinematicCameras.size(); i++)
	{
		if(StringUtil::EqualsIgnoreCase(mCinematicCameras[i]->label, cameraName))
		{
			return mCinematicCameras[i];
		}
	}
	return nullptr;
}

const DialogueSceneCamera* SceneData::GetDialogueCamera(const std::string& cameraName) const
{
	for(int i = 0; i < mDialogueCameras.size(); i++)
	{
		if(StringUtil::EqualsIgnoreCase(mDialogueCameras[i]->label, cameraName))
		{
			return mDialogueCameras[i];
		}
	}
	return nullptr;
}

const DialogueSceneCamera* SceneData::GetInitialDialogueCameraForConversation(const std::string& conversationName) const
{
    for(int i = 0; i < mDialogueCameras.size(); i++)
    {
        if(mDialogueCameras[i]->isInitial && StringUtil::EqualsIgnoreCase(mDialogueCameras[i]->dialogueName, conversationName))
        {
            return mDialogueCameras[i];
        }
    }
    return nullptr;
}

const DialogueSceneCamera* SceneData::GetFinalDialogueCameraForConversation(const std::string& conversationName) const
{
    for(int i = 0; i < mDialogueCameras.size(); i++)
    {
        if(mDialogueCameras[i]->isFinal && StringUtil::EqualsIgnoreCase(mDialogueCameras[i]->dialogueName, conversationName))
        {
            return mDialogueCameras[i];
        }
    }
    return nullptr;
}

std::vector<const SceneConversation*> SceneData::GetConversationSettings(const std::string& conversationName) const
{
    std::vector<const SceneConversation*> settings;
    for(int i = 0; i < mConversations.size(); i++)
    {
        if(StringUtil::EqualsIgnoreCase(mConversations[i]->name, conversationName))
        {
            settings.push_back(mConversations[i]);
        }
    }
    return settings;
}

void SceneData::AddActorBlocks(const std::vector<ConditionalBlock<SceneActor>>& actorBlocks)
{
	for(auto& block : actorBlocks)
	{
		if(Services::GetSheep()->Evaluate(block.condition))
		{
			for(auto& actor : block.items)
			{
				mActors.push_back(&actor);
			}
		}
	}
}

void SceneData::AddModelBlocks(const std::vector<ConditionalBlock<SceneModel>>& modelBlocks)
{
	for(auto& block : modelBlocks)
	{
		if(Services::GetSheep()->Evaluate(block.condition))
		{
			for(auto& model : block.items)
			{
				mModels.push_back(&model);
			}
		}
	}
}

void SceneData::AddPositionBlocks(const std::vector<ConditionalBlock<ScenePosition>>& positionBlocks)
{
	for(auto& block : positionBlocks)
	{
		if(Services::GetSheep()->Evaluate(block.condition))
		{
			for(auto& position : block.items)
			{
				mPositions.push_back(&position);
			}
		}
	}
}

void SceneData::AddInspectCameraBlocks(const std::vector<ConditionalBlock<SceneCamera>>& cameraBlocks)
{
	for(auto& block : cameraBlocks)
	{
		if(Services::GetSheep()->Evaluate(block.condition))
		{
			for(auto& camera : block.items)
			{
				mInspectCameras.push_back(&camera);
			}
		}
	}
}

void SceneData::AddRoomCameraBlocks(const std::vector<ConditionalBlock<RoomSceneCamera>>& cameraBlocks)
{
	for(auto& block : cameraBlocks)
	{
		if(Services::GetSheep()->Evaluate(block.condition))
		{
			for(auto& camera : block.items)
			{
				// Since we're iterating cameras anyway, save the default camera if we come across it.
				// If multiple defaults are specified, the last one specified wins out.
				if(camera.isDefault || mRoomCameras.size() == 0)
				{
					mDefaultRoomCamera = &camera;
				}
				mRoomCameras.push_back(&camera);
			}
		}
	}
}

void SceneData::AddCinematicCameraBlocks(const std::vector<ConditionalBlock<SceneCamera>>& cameraBlocks)
{
	for(auto& block : cameraBlocks)
	{
		if(Services::GetSheep()->Evaluate(block.condition))
		{
			for(auto& camera : block.items)
			{
				mCinematicCameras.push_back(&camera);
			}
		}
	}
}

void SceneData::AddDialogueCameraBlocks(const std::vector<ConditionalBlock<DialogueSceneCamera>>& cameraBlocks)
{
	for(auto& block : cameraBlocks)
	{
		if(Services::GetSheep()->Evaluate(block.condition))
		{
			for(auto& camera : block.items)
			{
				mDialogueCameras.push_back(&camera);
			}
		}
	}
}

void SceneData::AddTriggerBlocks(const std::vector<ConditionalBlock<SceneRegionOrTrigger>>& triggerBlocks)
{
    for(auto& block : triggerBlocks)
    {
        if(Services::GetSheep()->Evaluate(block.condition))
        {
            for(auto& trigger : block.items)
            {
                mTriggers.push_back(&trigger);
            }
        }
    }
}

void SceneData::AddSoundtrackBlocks(const std::vector<ConditionalBlock<Soundtrack*>>& soundtrackBlocks)
{
	for(auto& block : soundtrackBlocks)
	{
		if(Services::GetSheep()->Evaluate(block.condition))
		{
			mSoundtracks.insert(mSoundtracks.end(), block.items.begin(), block.items.end());
		}
	}
}

void SceneData::AddConversationBlocks(const std::vector<ConditionalBlock<SceneConversation>>& conversationBlocks)
{
    for(auto& block : conversationBlocks)
    {
        if(Services::GetSheep()->Evaluate(block.condition))
        {
            for(auto& conversation : block.items)
            {
                mConversations.push_back(&conversation);
            }
        }
    }
}

void SceneData::AddActionBlocks(const std::vector<ConditionalBlock<NVC*>>& actionSetBlocks, bool performNameCheck)
{
	for(auto& block : actionSetBlocks)
	{
		if(Services::GetSheep()->Evaluate(block.condition))
		{
			for(auto& nvc : block.items)
			{
				if(performNameCheck)
				{
					Services::Get<ActionManager>()->AddActionSetIfForTimeblock(nvc->GetName(), mTimeblock);
				}
				else
				{
					Services::Get<ActionManager>()->AddActionSet(nvc->GetName());
				}
			}
		}
		else
		{
			Services::GetReports()->Log("Generic", StringUtil::Format("Skipping header `%s`.", block.conditionText.c_str()));
		}
	}
}
