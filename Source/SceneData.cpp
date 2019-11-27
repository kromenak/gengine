//
// SceneData.cpp
//
// Clark Kromenaker
//
#include "SceneData.h"

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
	
	AddActionBlocks(mGeneralSIF->GetActionBlocks(), true);
	if(mSpecificSIF != nullptr)
	{
		AddActionBlocks(mSpecificSIF->GetActionBlocks(), false);
	}
	
	// ALSO, always include global action sets...
	//TODO: Maybe we should load and store this elsewhere? NVCManager anyone?
	//TODO: Though, the AssetManager makes sure we only have one in memory anyway, so maybe this is fine...
	mActionSets.push_back(Services::GetAssets()->LoadNVC("GLB_ALL.NVC"));
	
	//TODO: Handle also loading "GLB" (global) NVCs for specific days and timeblocks.
	//TODO: e.g. GLB_23ALL should be loaded if the day is 2 or 3
	//TODO: e.g. GLB_210A should be loaded on day 2 during 10AM timeblock
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
			return mRoomCameras[i];
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

std::vector<const Action*> SceneData::GetActions(const std::string& noun, GKActor* ego) const
{
	// As we iterate, we'll use this to keep track of what verbs are in use.
	// We don't want verb repeats - a new item with the same verb will overwrite the old item.
	std::unordered_map<std::string, const Action*> verbsToActions;
	for(auto& nvc : mActionSets)
	{
		const std::vector<Action>& allActions = nvc->GetActions(noun);
		for(auto& action : allActions)
		{
			if(nvc->IsCaseMet(&action, ego))
			{
				verbsToActions[action.verb] = &action;
			}
		}
	}
	
	// Finally, convert our map to a vector to return.
	std::vector<const Action*> viableActions;
	for(auto entry : verbsToActions)
	{
		viableActions.push_back(entry.second);
	}
	return viableActions;
}

const Action* SceneData::GetAction(const std::string& noun, const std::string& verb, GKActor* ego) const
{
	// Cycle through NVCs, trying to find a valid action.
	// Again, any later match will overwrite an earlier match.
	const Action* action = nullptr;
	for(auto& nvc : mActionSets)
	{
		const Action* possibleAction = nvc->GetAction(noun, verb);
		if(possibleAction != nullptr && nvc->IsCaseMet(possibleAction, ego))
		{
			action = possibleAction;
		}
	}
	return action;
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
			// If performing a name check (occurs in general SIF), we ONLY want to load action sets if the name of the asset
			// implies that it should be used for the current day or timeblock.
			if(performNameCheck)
			{
				for(auto& nvc : block.items)
				{
					// For the general SIF, a naming convention indicates whether we should or shouldn't use the NVC.
					// For example, "loc_all" is always used. "loc_12all" is used for days 1 & 2.
					std::string nvcName = nvc->GetNameNoExtension();
					StringUtil::ToLower(nvcName);
					
					// First three letters are always the location code.
					// Arguably, we could care that the location code matches the current location, but that's kind of a given.
					// So, we'll just ignore it.
					std::size_t curIndex = 3;
					
					// Next, there mayyy be an underscore, but maybe not.
					// Skip the underscore, in any case.
					if(nvcName[curIndex] == '_')
					{
						++curIndex;
					}
					
					// See if "all" is in the name.
					// If so, it indicates that the actions are used for all timeblocks on one or more days.
					std::size_t allPos = nvcName.find("all", curIndex);
					if(allPos != std::string::npos)
					{
						// If "all" is at the current index, it means there's no day constraint - just ALWAYS load this one!
						if(allPos == curIndex)
						{
							mActionSets.push_back(nvc);
						}
						else
						{
							// "all" is later in the string, meaning intermediate characters indicate which
							// days are OK to use this NVC. So, see if the current day is included!
							for(std::size_t i = curIndex; i < allPos; ++i)
							{
								if(std::isdigit(nvcName[i]))
								{
									int day = std::stoi(std::string(1, nvcName[i]));
									if(day == mTimeblock.GetDay())
									{
										mActionSets.push_back(nvc);
										break;
									}
								}
							}
						}
					}
					else
					{
						// If "all" did not appear, we assume this is an action set for a specific timeblock ONLY!
						// See if it's the current timeblock!
						std::string currentTimeblock = mTimeblock.ToString();
						StringUtil::ToLower(currentTimeblock);
						
						if(nvcName.find(currentTimeblock) != std::string::npos)
						{
							mActionSets.push_back(nvc);
						}
					}
				}
			}
			else
			{
				// No name check = just insert them all!
				mActionSets.insert(mActionSets.end(), block.items.begin(), block.items.end());
			}
		}
	}
}
