//
// SceneData.cpp
//
// Clark Kromenaker
//
#include "SceneData.h"

#include "ActionManager.h"
#include "Services.h"
#include "StringUtil.h"
#include "WalkerBoundary.h"

SceneData::SceneData(const std::string& location, const std::string& timeblock) : mTimeblock(timeblock)
{
	// Load general and specific SIF assets.
	mGeneralSIF = Services::GetAssets()->LoadSIF(location);
	mSpecificSIF = Services::GetAssets()->LoadSIF(location + timeblock);
	
	//TODO: If no general SIF...that's not good!
}

const SceneActor* SceneData::DetermineWhoEgoWillBe() const
{
	// If there's a specific SIF, it will override ego choice - check it first.
	const SceneActor* ego = mSpecificSIF != nullptr ? mSpecificSIF->FindCurrentEgo() : nullptr;
	
	// If couldn't find ego in specific SIF, we'll have to use the general SIF.
	return ego != nullptr ? ego : mGeneralSIF->FindCurrentEgo();
}

void SceneData::ResolveSceneData()
{
	// We need a SIF, at least.
	if(mGeneralSIF == nullptr) { return; }
	
	// GENERAL
	// Take general block from general SIF to start.
	mGeneralSettings = mGeneralSIF->FindCurrentGeneralBlock();
	
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
		mWalkerBoundary = new WalkerBoundary();
		mWalkerBoundary->SetTexture(Services::GetAssets()->LoadTexture(mGeneralSettings.walkerBoundaryTextureName));
		mWalkerBoundary->SetSize(mGeneralSettings.walkerBoundarySize);
		mWalkerBoundary->SetOffset(mGeneralSettings.walkerBoundaryOffset);
	}
	
	// Build list of actors to use in the scene based on contents of the two SIFs.
	AddActorBlocks(mGeneralSIF->GetActorBlocks());
	if(mSpecificSIF != nullptr)
	{
		AddActorBlocks(mSpecificSIF->GetActorBlocks());
	}
	
	// Build list of models to use in the scene based on contents of the two SIFS.
	AddModelBlocks(mGeneralSIF->GetModelBlocks());
	if(mSpecificSIF != nullptr)
	{
		AddModelBlocks(mSpecificSIF->GetModelBlocks());
	}
	
	// And so on...
	AddPositionBlocks(mGeneralSIF->GetPositionBlocks());
	if(mSpecificSIF != nullptr)
	{
		AddPositionBlocks(mSpecificSIF->GetPositionBlocks());
	}
	
	AddInspectCameraBlocks(mGeneralSIF->GetInspectCameraBlocks());
	if(mSpecificSIF != nullptr)
	{
		AddInspectCameraBlocks(mSpecificSIF->GetInspectCameraBlocks());
	}
	
	AddRoomCameraBlocks(mGeneralSIF->GetRoomCameraBlocks());
	if(mSpecificSIF != nullptr)
	{
		AddRoomCameraBlocks(mSpecificSIF->GetRoomCameraBlocks());
	}
	
	AddCinematicCameraBlocks(mGeneralSIF->GetCinematicCameraBlocks());
	if(mSpecificSIF != nullptr)
	{
		AddCinematicCameraBlocks(mSpecificSIF->GetCinematicCameraBlocks());
	}
	
	AddDialogueCameraBlocks(mGeneralSIF->GetDialogueCameraBlocks());
	if(mSpecificSIF != nullptr)
	{
		AddDialogueCameraBlocks(mSpecificSIF->GetDialogueCameraBlocks());
	}
	
	AddSoundtrackBlocks(mGeneralSIF->GetSoundtrackBlocks());
	if(mSpecificSIF != nullptr)
	{
		AddSoundtrackBlocks(mSpecificSIF->GetSoundtrackBlocks());
	}
	
	// Clear actions from previous scene - we're about to populate here!
	Services::Get<ActionManager>()->ClearActionSets();
	
	// Always load action sets that are global or for inventory.
	Services::Get<ActionManager>()->AddGlobalAndInventoryActionSets(mTimeblock);
	
	// Populate actions for current scene.
	// General SIF contains action sets that should only be conditionally loaded based on current timeblock.
	// We should always load action sets for specific SIF.
	AddActionBlocks(mGeneralSIF->GetActionBlocks(), true);
	if(mSpecificSIF != nullptr)
	{
		AddActionBlocks(mSpecificSIF->GetActionBlocks(), false);
	}
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
	}
}
