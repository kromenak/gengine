//
// SceneData.cpp
//
// Clark Kromenaker
//
#include "SceneData.h"

#include "Services.h"
#include "StringUtil.h"

SceneData::SceneData(std::string location, std::string timeblock)
{
	std::string specificName = location + timeblock;
	
	// Load general and specific SIF assets.
	mGeneralSIF = Services::GetAssets()->LoadSIF(location);
	assert(mGeneralSIF != nullptr);
	mSpecificSIF = Services::GetAssets()->LoadSIF(specificName);
	
	// Load scene model asset. Only *one* is needed, so one defined
	// in the specific SIF will override one from the general SIF.
	std::string sceneModelName;
	if(mSpecificSIF != nullptr && !mSpecificSIF->GetSceneModelName().empty())
	{
		sceneModelName = mSpecificSIF->GetSceneModelName();
	}
	else
	{
		sceneModelName = mGeneralSIF->GetSceneModelName();
	}
	mSceneModel = Services::GetAssets()->LoadSceneModel(sceneModelName);
	assert(mSceneModel != nullptr);
	
	// Load the BSP data, which is specified by the scene model.
	// If this is null, the game will still work...but there's no BSP geometry!
	mBSP = Services::GetAssets()->LoadBSP(mSceneModel->GetBSPName());

	// Figure out if we have a skybox, and set it to be rendered.
	// The skybox can be defined in several places. Go down priority list and find one!
	mSkybox = mSceneModel->GetSkybox();
	if(mSkybox == nullptr && mSpecificSIF != nullptr)
	{
		mSkybox = mSpecificSIF->GetSkybox();
	}
	if(mSkybox == nullptr)
	{
		mSkybox = mGeneralSIF->GetSkybox();
	}
	
	// Collect actor definitions from general and specific SIFs.
	mSceneActorDatas = mGeneralSIF->GetSceneActorDatas();
	if(mSpecificSIF != nullptr)
	{
		std::vector<SceneActorData*> sceneActorDatas = mSpecificSIF->GetSceneActorDatas();
		mSceneActorDatas.insert(mSceneActorDatas.end(), sceneActorDatas.begin(), sceneActorDatas.end());
		
		//TODO: If there is more than one "ego" actor in the list, ignore all but the latest one.
	}
	
	// Collect model definitions from general and specific SIFs.
	mSceneModelDatas = mGeneralSIF->GetSceneModelDatas();
	if(mSpecificSIF != nullptr)
	{
		std::vector<SceneModelData*> sceneModelDatas = mSpecificSIF->GetSceneModelDatas();
		mSceneModelDatas.insert(mSceneModelDatas.end(), sceneModelDatas.begin(), sceneModelDatas.end());
	}
	
	// Collect noun/verb/case sets from general and specific SIFs.
	// The logic for the general SIF is a bit more complicated!
	std::vector<NVC*> nounVerbCases = mGeneralSIF->GetNounVerbCases();
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
			std::cout << "Using NVC " << nvcName << std::endl;
			mNounVerbCaseSets.push_back(nvc);
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
					std::cout << "Using NVC " << nvcName << std::endl;
					mNounVerbCaseSets.push_back(nvc);
					break;
				}
				checkPos++;
			}
		}
	}
	
	// The specific SIF NVCs is much simpler: just use them all!
	if(mSpecificSIF != nullptr)
	{
		nounVerbCases = mSpecificSIF->GetNounVerbCases();
		mNounVerbCaseSets.insert(mNounVerbCaseSets.end(), nounVerbCases.begin(), nounVerbCases.end());
	}
	
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

std::string SceneData::GetFloorModelName() const
{
	std::string floorModelName;
	if(mSpecificSIF != nullptr)
	{
		floorModelName = mSpecificSIF->GetFloorBspModelName();
	}
	if(floorModelName.empty())
	{
		floorModelName = mGeneralSIF->GetFloorBspModelName();
	}
	return floorModelName;
}

SceneCameraData* SceneData::GetDefaultRoomCamera() const
{
	SceneCameraData* sceneCameraData = nullptr;
	if(mSpecificSIF != nullptr)
	{
		sceneCameraData = mSpecificSIF->GetDefaultRoomCamera();
	}
	if(sceneCameraData == nullptr)
	{
		sceneCameraData = mGeneralSIF->GetDefaultRoomCamera();
	}
	return sceneCameraData;
}

SceneCameraData* SceneData::GetRoomCamera(std::string cameraName) const
{
	SceneCameraData* sceneCameraData = nullptr;
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

ScenePositionData* SceneData::GetScenePosition(std::string positionName) const
{
	ScenePositionData* position = nullptr;
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

Color32 SceneData::GetWalkBoundaryColor(Vector3 position) const
{
	if(mSpecificSIF != nullptr && mSpecificSIF->GetWalkBoundaryTexture() != nullptr)
	{
		return mSpecificSIF->GetWalkBoundaryColor(position);
	}
	return mGeneralSIF->GetWalkBoundaryColor(position);
}

std::vector<const NVCItem*> SceneData::GetViableVerbsForNoun(std::string noun, GKActor* ego) const
{
	// As we iterate, we'll use this to keep track of what verbs are in use.
	// We don't want verb repeats - a new item with the same verb will overwrite the old item.
	std::unordered_map<std::string, const NVCItem*> verbsToActions;
	for(auto& nvc : mNounVerbCaseSets)
	{
		const std::vector<NVCItem>& allActions = nvc->GetActionsForNoun(noun);
		for(auto& action : allActions)
		{
			if(nvc->IsCaseMet(&action, ego))
			{
				verbsToActions[action.verb] = &action;
			}
		}
	}
	
	// Finally, convert our map to a vector to return.
	std::vector<const NVCItem*> viableActions;
	for(auto entry : verbsToActions)
	{
		viableActions.push_back(entry.second);
	}
	return viableActions;
}

const NVCItem* SceneData::GetNounVerbAction(std::string noun, std::string verb, GKActor* ego) const
{
	// Cycle through NVCs, trying to find a valid action.
	// Again, any later match will overwrite an earlier match.
	const NVCItem* action = nullptr;
	for(auto& nvc : mNounVerbCaseSets)
	{
		const NVCItem* possibleAction = nvc->GetAction(noun, verb);
		if(possibleAction != nullptr && nvc->IsCaseMet(possibleAction, ego))
		{
			action = possibleAction;
		}
	}
	return action;
}


