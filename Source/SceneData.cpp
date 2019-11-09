//
// SceneData.cpp
//
// Clark Kromenaker
//
#include "SceneData.h"

#include "Services.h"
#include "StringUtil.h"

SceneData::SceneData(const std::string& location, const std::string& timeblock)
{
	// Load general and specific SIF assets.
	mGeneralSIF = Services::GetAssets()->LoadSIF(location);
	mSpecificSIF = Services::GetAssets()->LoadSIF(location + timeblock);
	
	// Load scene model asset. Only *one* is needed, so one defined
	// in the specific SIF will override one from the general SIF.
	std::string sceneAssetName;
	if(mSpecificSIF != nullptr && !mSpecificSIF->GetSceneAssetName().empty())
	{
		sceneAssetName = mSpecificSIF->GetSceneAssetName();
	}
	else if(mGeneralSIF != nullptr)
	{
		sceneAssetName = mGeneralSIF->GetSceneAssetName();
	}
	mSceneAsset = Services::GetAssets()->LoadSceneAsset(sceneAssetName);
	
	// Load the BSP data, which is specified by the scene model.
	// If this is null, the game will still work...but there's no BSP geometry!
	if(mSceneAsset != nullptr)
	{
		mBSP = Services::GetAssets()->LoadBSP(mSceneAsset->GetBSPName());
	}

	// Figure out if we have a skybox, and set it to be rendered.
	// The skybox can be defined in several places. Go down priority list and find one!
	if(mSceneAsset != nullptr)
	{
		mSkybox = mSceneAsset->GetSkybox();
	}
	if(mSkybox == nullptr && mSpecificSIF != nullptr)
	{
		mSkybox = mSpecificSIF->GetSkybox();
	}
	if(mSkybox == nullptr && mGeneralSIF != nullptr)
	{
		mSkybox = mGeneralSIF->GetSkybox();
	}
	
	// FROM HERE, the lack of SIF files proves fatal, so just ignore if we don't have.
	if(mGeneralSIF == nullptr) { return; }
	
	// Collect actor definitions from general and specific SIFs.
	mSceneActors = mGeneralSIF->GetSceneActors();
	if(mSpecificSIF != nullptr)
	{
		std::vector<SceneActor*> sceneActorDatas = mSpecificSIF->GetSceneActors();
		mSceneActors.insert(mSceneActors.end(), sceneActorDatas.begin(), sceneActorDatas.end());
		
		//TODO: If there is more than one "ego" actor in the list, ignore all but the latest one.
	}
	
	// Collect model definitions from general and specific SIFs.
	mSceneModels = mGeneralSIF->GetSceneModels();
	if(mSpecificSIF != nullptr)
	{
		std::vector<SceneModel*> sceneModelDatas = mSpecificSIF->GetSceneModels();
		mSceneModels.insert(mSceneModels.end(), sceneModelDatas.begin(), sceneModelDatas.end());
	}
	
	// Collect noun/verb/case sets from general and specific SIFs.
	// The logic for the general SIF is a bit more complicated!
	std::vector<NVC*> nounVerbCases = mGeneralSIF->GetActionSets();
	for(auto& nvc : nounVerbCases)
	{
		// For the general SIF, a naming convention indicates whether we should or shouldn't use the NVC.
		// For example, "loc_all" is always used. "loc_12all" is used for days 1 & 2.
		std::string nvcName = nvc->GetNameNoExtension();
		StringUtil::ToLower(nvcName);
		
		// The naming convention always has an underscore followed by "all".
		std::size_t allPos = nvcName.find("all");
		std::size_t underscorePos = nvcName.rfind("_", allPos);
		
		// If "all" or underscore don't exist, use the file by default.
		// Also, if the underscore is just one spot before "all", use the file, since it should be loaded ALWAYS.
		if(allPos == std::string::npos || underscorePos == std::string::npos || underscorePos == allPos - 1)
		{
			//std::cout << "Using NVC " << nvcName << std::endl;
			mActionSets.push_back(nvc);
		}
		else
		{
			// So, there's some space between the underscore and the "all".
			// Those values should be digits indicating which days to load the NVC on.
			// If the digit matches the first digit of our timeblock, use it!
			std::size_t checkPos = underscorePos + 1;
			while(checkPos < allPos)
			{
				char val = nvcName[checkPos];
				if(val == timeblock[0])
				{
					//std::cout << "Using NVC " << nvcName << std::endl;
					mActionSets.push_back(nvc);
					break;
				}
				checkPos++;
			}
		}
	}
	
	// The specific SIF NVCs is much simpler: just use them all!
	if(mSpecificSIF != nullptr)
	{
		nounVerbCases = mSpecificSIF->GetActionSets();
		mActionSets.insert(mActionSets.end(), nounVerbCases.begin(), nounVerbCases.end());
	}
	
	// ALSO, always include the global NVC!
	//TODO: Maybe we should load and store this elsewhere? NVCManager anyone?
	//TODO: Though, the AssetManager makes sure we only have one in memory anyway, so maybe this is fine...
	mActionSets.push_back(Services::GetAssets()->LoadNVC("GLB_ALL.NVC"));
	
	//TODO: Handle also loading "GLB" (global) NVCs for specific days and timeblocks.
	//TODO: e.g. GLB_23ALL should be loaded if the day is 2 or 3
	//TODO: e.g. GLB_210A should be loaded on day 2 during 10AM timeblock
	
	// Determine which soundtrack to use. Give priority to specific SIF version, fall back on general one.
	std::vector<Soundtrack*> soundtracks;
	if(mSpecificSIF != nullptr)
	{
		soundtracks = mSpecificSIF->GetSoundtracks();
	}
	if(soundtracks.size() == 0)
	{
		soundtracks = mGeneralSIF->GetSoundtracks();
	}
	if(soundtracks.size() > 0)
	{
		mSoundtrack = soundtracks[0];
	}
}

const SceneCamera* SceneData::GetDefaultRoomCamera() const
{
	const SceneCamera* sceneCameraData = nullptr;
	if(mSpecificSIF != nullptr)
	{
		sceneCameraData = mSpecificSIF->GetDefaultRoomCamera();
	}
	if(sceneCameraData == nullptr && mGeneralSIF != nullptr)
	{
		sceneCameraData = mGeneralSIF->GetDefaultRoomCamera();
	}
	return sceneCameraData;
}

const SceneCamera* SceneData::GetRoomCamera(const std::string& cameraName) const
{
	const SceneCamera* sceneCameraData = nullptr;
	if(mSpecificSIF != nullptr)
	{
		sceneCameraData = mSpecificSIF->GetRoomCamera(cameraName);
	}
	if(sceneCameraData == nullptr)
	{
		sceneCameraData = mGeneralSIF->GetRoomCamera(cameraName);
	}
	return sceneCameraData;
}

const ScenePosition* SceneData::GetScenePosition(const std::string& positionName) const
{
	const ScenePosition* position = nullptr;
	if(mSpecificSIF != nullptr)
	{
		position = mSpecificSIF->GetPosition(positionName);
	}
	if(position == nullptr)
	{
		position = mGeneralSIF->GetPosition(positionName);
	}
	return position;
}

const std::string& SceneData::GetFloorModelName() const
{
	if(mSpecificSIF != nullptr &&
	   !mSpecificSIF->GetFloorModelName().empty())
	{
		return mSpecificSIF->GetFloorModelName();
	}
	return mGeneralSIF->GetFloorModelName();
}

WalkerBoundary* SceneData::GetWalkerBoundary() const
{
	WalkerBoundary* walkerBoundary = nullptr;
	if(mSpecificSIF != nullptr)
	{
		walkerBoundary = mSpecificSIF->GetWalkerBoundary();
	}
	if(walkerBoundary == nullptr && mGeneralSIF != nullptr)
	{
		walkerBoundary = mGeneralSIF->GetWalkerBoundary();
	}
	return walkerBoundary;
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


