#include "SceneInitFile.h"

#include "AssetManager.h"
#include "IniParser.h"
#include "Model.h"
#include "SheepManager.h"
#include "Skybox.h"
#include "StringUtil.h"

Skybox* GeneralBlock::CreateSkybox()
{
	// If we have any of the six skybox sides, we can make a skybox!
	bool hasSkyboxData = !skyboxLeftTextureName.empty() || !skyboxRightTextureName.empty()
    || !skyboxBackTextureName.empty() || !skyboxFrontTextureName.empty()
	|| !skyboxDownTextureName.empty() || !skyboxUpTextureName.empty();
	if(!hasSkyboxData) { return nullptr; }
	
	Skybox* skybox = new Skybox();
	skybox->SetLeftTexture(gAssetManager.LoadSceneTexture(skyboxLeftTextureName, AssetScope::Scene));
	skybox->SetRightTexture(gAssetManager.LoadSceneTexture(skyboxRightTextureName, AssetScope::Scene));
	skybox->SetBackTexture(gAssetManager.LoadSceneTexture(skyboxBackTextureName, AssetScope::Scene));
	skybox->SetFrontTexture(gAssetManager.LoadSceneTexture(skyboxFrontTextureName, AssetScope::Scene));
	skybox->SetDownTexture(gAssetManager.LoadSceneTexture(skyboxDownTextureName, AssetScope::Scene));
	skybox->SetUpTexture(gAssetManager.LoadSceneTexture(skyboxUpTextureName, AssetScope::Scene));
	return skybox;
}

void GeneralBlock::TakeOverridesFrom(const GeneralBlock& other)
{
	// Go through each property and use it if not empty/default.
	if(!other.sceneAssetName.empty())
	{
		sceneAssetName = other.sceneAssetName;
	}
	
	if(!other.floorModelName.empty())
	{
		floorModelName = other.floorModelName;
	}
	if(!other.walkerBoundaryTextureName.empty())
	{
		walkerBoundaryTextureName = other.walkerBoundaryTextureName;
	}
	if(other.walkerBoundarySize != Vector2::Zero)
	{
		walkerBoundarySize = other.walkerBoundarySize;
	}
	if(other.walkerBoundaryOffset != Vector2::Zero)
	{
		walkerBoundaryOffset = other.walkerBoundaryOffset;
	}

    // Unlike others here, "overriding" camera bounds models is additive!
    cameraBoundsModelNames.insert(cameraBoundsModelNames.end(), other.cameraBoundsModelNames.begin(), other.cameraBoundsModelNames.end());
	
	if(other.globalLightPosition != Vector3::Zero)
	{
		globalLightPosition = other.globalLightPosition;
	}
	if(other.globalLightAmbient != Vector3::Zero)
	{
		globalLightAmbient = other.globalLightAmbient;
	}
	
	if(!other.skyboxLeftTextureName.empty())
	{
		skyboxLeftTextureName = other.skyboxLeftTextureName;
	}
	if(!other.skyboxRightTextureName.empty())
	{
		skyboxRightTextureName = other.skyboxRightTextureName;
	}
	if(!other.skyboxBackTextureName.empty())
	{
		skyboxBackTextureName = other.skyboxBackTextureName;
	}
	if(!other.skyboxFrontTextureName.empty())
	{
		skyboxFrontTextureName = other.skyboxFrontTextureName;
	}
	if(!other.skyboxDownTextureName.empty())
	{
		skyboxDownTextureName = other.skyboxDownTextureName;
	}
	if(!other.skyboxUpTextureName.empty())
	{
		skyboxUpTextureName = other.skyboxUpTextureName;
	}
}

SceneInitFile::~SceneInitFile()
{
	//TODO: delete any block conditions - we own them after compiling!
}

void SceneInitFile::Load(uint8_t* data, uint32_t dataLength)
{
    ParseFromData(data, dataLength);
}

const SceneActor* SceneInitFile::FindCurrentEgo() const
{
	// Though rare, it's possible for multiple egos to be specified in a single SIF.
	// And it's possible for one to be in a block with a conditional.
	// Our rule for picking will be the one furthest down the file that has a passing condition.
	// Since actor block list is naturally ordered, it's really just the last one we find!
	const SceneActor* bestPick = nullptr;
	for(auto& actorBlock : mActors)
	{
		// Find any egos in this block.
		// In the off-chance (and probably error) there are two, just use the last one.
		const SceneActor* egoInBlock = nullptr;
		for(auto& actor : actorBlock.items)
		{
			if(actor.ego)
			{
				egoInBlock = &actor;
			}
		}
		
		// If we found an ego, just make sure the block condition passes. If so, we can use it.
		if(egoInBlock != nullptr)
		{
			if(gSheepManager.Evaluate(actorBlock.condition))
			{
				bestPick = egoInBlock;
			}
		}
	}
	return bestPick;
}

GeneralBlock SceneInitFile::FindCurrentGeneralBlock() const
{
	// General blocks have a unique property where each block in a SIF can specify individual properties and exclude others,
	// with later blocks overriding the values set in previous blocks, if any.
	
	// So, what we do here is start with a default block, iterate over all blocks, and fill in our working block
	// with any data specified if the condition for the block is met.
	GeneralBlock block;
	for(auto& gb : mGeneralBlocks)
	{
		// Only use this block if the condition passes.
		if(gSheepManager.Evaluate(gb.condition))
		{
			block.TakeOverridesFrom(gb);
		}
	}
	
	// Return the constructed/filled in block.
	return block;
}

void SceneInitFile::ParseFromData(uint8_t* data, uint32_t dataLength)
{
    IniParser parser(data, dataLength);
    parser.ParseAll();
    
    // Read in general section.
    std::vector<IniSection> generals = parser.GetSections("GENERAL");
    for(auto& section : generals)
    {
		mGeneralBlocks.emplace_back();
		GeneralBlock& general = mGeneralBlocks.back();
		
        // Compile and save condition.
        if(!section.condition.empty())
        {
			// Why is this called "Int Evaluation"? Not sure - but testing in GK3 seems to suggest it is...
			general.conditionText = section.condition;
			general.condition = gSheepManager.Compile("Int Evaluation", section.condition);
        }
        
		// Handle all key/value pairs in this block.
        for(auto& line : section.lines)
        {
			IniKeyValue& first = line.entries.front();
			if(StringUtil::EqualsIgnoreCase(first.key, "scene"))
            {
				general.sceneAssetName = first.value;
            }
            else if(StringUtil::EqualsIgnoreCase(first.key, "floor"))
            {
				general.floorModelName = first.value;
            }
            else if(StringUtil::EqualsIgnoreCase(first.key, "boundary"))
            {
				// First value is name of a texture defining walk bounds.
				general.walkerBoundaryTextureName = first.value;
				
				// Remaining key/values are size/offset of the texture in 3D space.
				// This is used as a 2D overlay on the X/Z plane to determine walkable area.
				for(size_t i = 1; i < line.entries.size(); ++i)
				{
					IniKeyValue& keyValue = line.entries[i];
					if(StringUtil::EqualsIgnoreCase(keyValue.key, "size"))
                    {
						general.walkerBoundarySize = keyValue.GetValueAsVector2();
                    }
                    else if(StringUtil::EqualsIgnoreCase(keyValue.key, "offset"))
                    {
						general.walkerBoundaryOffset = keyValue.GetValueAsVector2();
                    }
				}
            }
            else if(StringUtil::EqualsIgnoreCase(first.key, "cameraBounds"))
            {
				// Add to bounds model names.
                general.cameraBoundsModelNames.push_back(first.value);
				
				// One possible option: a type.
				if(line.entries.size() > 1)
				{
					IniKeyValue& keyValue = line.entries[1];
					
					//TODO: Based on search of assets, this behavior seems never to be used (even though it is documented). So...get rid of it?
					if(StringUtil::EqualsIgnoreCase(keyValue.key, "type"))
					{
						general.cameraBoundsDynamic = (StringUtil::EqualsIgnoreCase(keyValue.value, "dynamic"));
					}
				}
            }
            else if(StringUtil::EqualsIgnoreCase(first.key, "globalLight"))
            {
				for(size_t i = 1; i < line.entries.size(); ++i)
				{
					IniKeyValue& keyValue = line.entries[i];
					if(StringUtil::EqualsIgnoreCase(keyValue.key, "pos"))
                    {
                        general.globalLightPosition = keyValue.GetValueAsVector3();
                    }
                    else if(StringUtil::EqualsIgnoreCase(keyValue.key, "ambient"))
                    {
						//TODO: I think ambient is probably meant to be a color, so we should save as Color32 I think?
                        general.globalLightAmbient = keyValue.GetValueAsVector3();
                    }
				}
            }
            else if(StringUtil::EqualsIgnoreCase(first.key, "skybox"))
            {
				for(size_t i = 1; i < line.entries.size(); ++i)
				{
					IniKeyValue& keyValue = line.entries[i];
                    if(StringUtil::EqualsIgnoreCase(keyValue.key, "left"))
                    {
						general.skyboxLeftTextureName = keyValue.value;
                    }
                    else if(StringUtil::EqualsIgnoreCase(keyValue.key, "right"))
                    {
                        general.skyboxRightTextureName = keyValue.value;
                    }
                    else if(StringUtil::EqualsIgnoreCase(keyValue.key, "front"))
                    {
                        general.skyboxFrontTextureName = keyValue.value;
                    }
                    else if(StringUtil::EqualsIgnoreCase(keyValue.key, "back"))
                    {
                        general.skyboxBackTextureName = keyValue.value;
                    }
                    else if(StringUtil::EqualsIgnoreCase(keyValue.key, "up"))
                    {
                        general.skyboxUpTextureName = keyValue.value;
                    }
                    else if(StringUtil::EqualsIgnoreCase(keyValue.key, "down"))
                    {
                        general.skyboxDownTextureName = keyValue.value;
                    }
				}
            }
        }
    }
    
    // Read in cameras.
    std::vector<IniSection> inspectCameras = parser.GetSections("INSPECT_CAMERAS");
    for(auto& section : inspectCameras)
    {
		mInspectCameras.emplace_back();
		ConditionalBlock<SceneCamera>& cameraBlock = mInspectCameras.back();
		
        // Compile and save condition.
        if(!section.condition.empty())
        {
			cameraBlock.conditionText = section.condition;
			cameraBlock.condition = gSheepManager.Compile("Int Evaluation", section.condition);
        }
        
		// Handle creation of each camera in this block.
        for(auto& line : section.lines)
        {
			cameraBlock.items.emplace_back();
			SceneCamera& camera = cameraBlock.items.back();
			
			for(auto& keyValue : line.entries)
			{
				if(StringUtil::EqualsIgnoreCase(keyValue.key, "noun") || StringUtil::EqualsIgnoreCase(keyValue.key, "model"))
                {
                    camera.label = keyValue.value;
                }
                else if(StringUtil::EqualsIgnoreCase(keyValue.key, "pos"))
                {
                    camera.position = keyValue.GetValueAsVector3();
                }
                else if(StringUtil::EqualsIgnoreCase(keyValue.key, "angle"))
                {
                    // Angle will be in degrees, but we want it in radians for internal use.
					Vector2 angleDeg = keyValue.GetValueAsVector2();
                    camera.angle.x = Math::ToRadians(angleDeg.x);
					camera.angle.y = Math::ToRadians(angleDeg.y);
                }
                else if(StringUtil::EqualsIgnoreCase(keyValue.key, "fov"))
                {
                    camera.fov = keyValue.GetValueAsFloat();
                }
			}
        }
    }
    
    std::vector<IniSection> roomCameras = parser.GetSections("ROOM_CAMERAS");
    for(auto& section : roomCameras)
    {
		mRoomCameras.emplace_back();
		ConditionalBlock<RoomSceneCamera>& cameraBlock = mRoomCameras.back();
		
        // Compile and save condition.
        if(!section.condition.empty())
        {
			cameraBlock.conditionText = section.condition;
			cameraBlock.condition = gSheepManager.Compile("Int Evaluation", section.condition);
        }
        
		// Handle creation of each camera in this block.
        for(auto& line : section.lines)
        {
			cameraBlock.items.emplace_back();
			RoomSceneCamera& camera = cameraBlock.items.back();
			
			// First keyword for room cameras is always the camera name.
			IniKeyValue& first = line.entries.front();
			camera.label = first.key;
			
			// Followed by one or more optional attributes.
			for(size_t i = 1; i < line.entries.size(); ++i)
			{
				IniKeyValue& keyValue = line.entries[i];
				if(StringUtil::EqualsIgnoreCase(keyValue.key, "pos"))
                {
                    camera.position = keyValue.GetValueAsVector3();
                }
                else if(StringUtil::EqualsIgnoreCase(keyValue.key, "angle"))
                {
					// Angle is in degrees, but we use radians internally.
					Vector2 angleDeg = keyValue.GetValueAsVector2();
                    camera.angle.x = Math::ToRadians(angleDeg.x);
                    camera.angle.y = Math::ToRadians(angleDeg.y);
                }
                else if(StringUtil::EqualsIgnoreCase(keyValue.key, "fov"))
                {
                    camera.fov = keyValue.GetValueAsFloat();
                }
                else if(StringUtil::EqualsIgnoreCase(keyValue.key, "default"))
                {
					camera.isDefault = true;
					// Save index for default room camera. Equals size, since about to add to list.
					//mDefaultRoomCameraIndex = (int)mRoomCameras.size();
                }
			}
        }
    }
    
    std::vector<IniSection> cinematicCameras = parser.GetSections("CINEMATIC_CAMERAS");
    for(auto& section : cinematicCameras)
    {
		mCinematicCameras.emplace_back();
		ConditionalBlock<SceneCamera>& cameraBlock = mCinematicCameras.back();
		
        // Compile and save condition.
        if(!section.condition.empty())
        {
			cameraBlock.conditionText = section.condition;
			cameraBlock.condition = gSheepManager.Compile("Int Evaluation", section.condition);
        }
        
		// Handle creation of each camera in this block.
        for(auto& line : section.lines)
        {
			cameraBlock.items.emplace_back();
			SceneCamera& camera = cameraBlock.items.back();
			
			// First keyword for cinematic cameras is always the camera name.
			IniKeyValue& first = line.entries.front();
			camera.label = first.key;
            
			// Followed by one or more optional attributes.
			for(size_t i = 1; i < line.entries.size(); ++i)
			{
				IniKeyValue& keyValue = line.entries[i];
				if(StringUtil::EqualsIgnoreCase(keyValue.key, "pos"))
                {
                    camera.position = keyValue.GetValueAsVector3();
                }
                else if(StringUtil::EqualsIgnoreCase(keyValue.key, "angle"))
                {
					// Angle is in degrees, but we use radians internally.
					Vector2 angleDeg = keyValue.GetValueAsVector2();
                    camera.angle.x = Math::ToRadians(angleDeg.x);
                    camera.angle.y = Math::ToRadians(angleDeg.y);
                }
                else if(StringUtil::EqualsIgnoreCase(keyValue.key, "fov"))
                {
                    camera.fov = keyValue.GetValueAsFloat();
                }
			}
        }
    }
    
    std::vector<IniSection> DialogueSceneCameras = parser.GetSections("DIALOGUE_CAMERAS");
    for(auto& section : DialogueSceneCameras)
    {
		mDialogueCameras.emplace_back();
		ConditionalBlock<DialogueSceneCamera>& cameraBlock = mDialogueCameras.back();
		
        // Compile and save condition.
        if(!section.condition.empty())
        {
			cameraBlock.conditionText = section.condition;
			cameraBlock.condition = gSheepManager.Compile("Int Evaluation", section.condition);
        }
        
		// Create each camera in this block.
        for(auto& line : section.lines)
        {
            cameraBlock.items.emplace_back();
			DialogueSceneCamera& camera = cameraBlock.items.back();
			
			// First keyword for dialogue cameras is always the camera name.
			IniKeyValue& first = line.entries.front();
			camera.label = first.key;
            
			// Followed by one or more optional attributes.
			for(size_t i = 1; i < line.entries.size(); ++i)
			{
				IniKeyValue& keyValue = line.entries[i];
				if(StringUtil::EqualsIgnoreCase(keyValue.key, "pos"))
                {
                    camera.position = keyValue.GetValueAsVector3();
                }
                else if(StringUtil::EqualsIgnoreCase(keyValue.key, "angle"))
                {
					// Angle is in degrees, but we use radians internally.
					Vector2 angleDeg = keyValue.GetValueAsVector2();
                    camera.angle.x = Math::ToRadians(angleDeg.x);
                    camera.angle.y = Math::ToRadians(angleDeg.y);
                }
                else if(StringUtil::EqualsIgnoreCase(keyValue.key, "fov"))
                {
                    camera.fov = keyValue.GetValueAsFloat();
                }
				else if(StringUtil::EqualsIgnoreCase(keyValue.key, "dialogue"))
                {
                    camera.dialogueName = keyValue.value;
                }
                else if(StringUtil::EqualsIgnoreCase(keyValue.key, "set"))
                {
                    camera.setName = keyValue.value;
                }
				else if(StringUtil::EqualsIgnoreCase(keyValue.key, "show"))
                {
                    camera.showInToolbar = true;
                }
                else if(StringUtil::EqualsIgnoreCase(keyValue.key, "initial"))
                {
                    camera.isInitial = true;
                }
                else if(StringUtil::EqualsIgnoreCase(keyValue.key, "final"))
                {
                    camera.isFinal = true;
                }
                //TODO: FOV?
			}
        }
    }
    
    // Read in positions.
    std::vector<IniSection> positionSections = parser.GetSections("POSITIONS");
    for(auto& section : positionSections)
    {
		mPositions.emplace_back();
		ConditionalBlock<ScenePosition>& positionBlock = mPositions.back();
		
        // Compile and save condition.
        if(!section.condition.empty())
        {
			positionBlock.conditionText = section.condition;
			positionBlock.condition = gSheepManager.Compile("Int Evaluation", section.condition);
        }
        
		// Create each scene position.
        for(auto& line : section.lines)
        {
			positionBlock.items.emplace_back();
			ScenePosition& position = positionBlock.items.back();
			
			// First pair is always the identifier.
			IniKeyValue& first = line.entries.front();
			position.label = first.key;
            
			// Followed by one or more optional attributes.
			for(size_t i = 1; i < line.entries.size(); ++i)
			{
				IniKeyValue& keyValue = line.entries[i];
				if(StringUtil::EqualsIgnoreCase(keyValue.key, "pos"))
                {
                    position.position = keyValue.GetValueAsVector3();
                }
                else if(StringUtil::EqualsIgnoreCase(keyValue.key, "heading"))
                {
					position.heading = Heading::FromDegrees(keyValue.GetValueAsFloat());
                }
                else if(StringUtil::EqualsIgnoreCase(keyValue.key, "camera"))
                {
					position.cameraName = keyValue.value;
                }
			}
        }
    }
    
    // Read in actors.
    std::vector<IniSection> actorSections = parser.GetSections("ACTORS");
    for(auto& section : actorSections)
    {
		mActors.emplace_back();
		ConditionalBlock<SceneActor>& actorBlock = mActors.back();
		
        // Compile and save condition.
        if(!section.condition.empty())
        {
			actorBlock.conditionText = section.condition;
			actorBlock.condition = gSheepManager.Compile("Int Evaluation", section.condition);
        }
        
		// Create each actor defined in the block.
        for(auto& line : section.lines)
        {
			actorBlock.items.emplace_back();
			SceneActor& actor = actorBlock.items.back();
			
			for(auto& keyValue : line.entries)
			{
				if(StringUtil::EqualsIgnoreCase(keyValue.key, "model"))
                {
                    actor.model = gAssetManager.LoadModel(keyValue.value, GetScope());
                }
                else if(StringUtil::EqualsIgnoreCase(keyValue.key, "noun"))
                {
                    actor.noun = keyValue.value;
					StringUtil::ToUpper(actor.noun);
                }
                else if(StringUtil::EqualsIgnoreCase(keyValue.key, "pos"))
                {
					actor.positionName = keyValue.value;
                }
                else if(StringUtil::EqualsIgnoreCase(keyValue.key, "idle"))
                {
                    actor.idleGas = gAssetManager.LoadGAS(keyValue.value);
                }
                else if(StringUtil::EqualsIgnoreCase(keyValue.key, "talk"))
                {
                    actor.talkGas = gAssetManager.LoadGAS(keyValue.value);
                }
                else if(StringUtil::EqualsIgnoreCase(keyValue.key, "listen"))
                {
                    actor.listenGas = gAssetManager.LoadGAS(keyValue.value);
                }
                else if(StringUtil::EqualsIgnoreCase(keyValue.key, "initAnim"))
                {
                    actor.initAnim = gAssetManager.LoadAnimation(keyValue.value, GetScope());
                }
                else if(StringUtil::EqualsIgnoreCase(keyValue.key, "hidden"))
                {
                    actor.hidden = true;
                }
                else if(StringUtil::EqualsIgnoreCase(keyValue.key, "ego"))
                {
                    actor.ego = true;
                }
			}
        }
    }
    
    // Read in models.
    std::vector<IniSection> modelSections = parser.GetSections("MODELS");
    for(auto& section : modelSections)
    {
		mModels.emplace_back();
		ConditionalBlock<SceneModel>& modelBlock = mModels.back();
		
        // Compile and save condition.
        if(!section.condition.empty())
        {
			modelBlock.conditionText = section.condition;
            modelBlock.condition = gSheepManager.Compile("Int Evaluation", section.condition);
        }
        
		// Create each model defined in block.
        for(auto& line : section.lines)
        {
			modelBlock.items.emplace_back();
			SceneModel& model = modelBlock.items.back();
            
			for(auto& keyValue : line.entries)
			{
				if(StringUtil::EqualsIgnoreCase(keyValue.key, "model"))
                {
                    model.name = keyValue.value;
                }
                else if(StringUtil::EqualsIgnoreCase(keyValue.key, "noun"))
                {
                    model.noun = keyValue.value;
					StringUtil::ToUpper(model.noun);
                }
                else if(StringUtil::EqualsIgnoreCase(keyValue.key, "type"))
                {
                    if(StringUtil::EqualsIgnoreCase(keyValue.value, "scene"))
                    {
                        model.type = SceneModel::Type::Scene;
                    }
                    else if(StringUtil::EqualsIgnoreCase(keyValue.value, "prop"))
                    {
                        model.type = SceneModel::Type::Prop;
                    }
                    else if(StringUtil::EqualsIgnoreCase(keyValue.value, "hittest"))
                    {
                        model.type = SceneModel::Type::HitTest;
                    }
                    else if(StringUtil::EqualsIgnoreCase(keyValue.value, "gasprop"))
                    {
                        model.type = SceneModel::Type::GasProp;
                    }
                }
                else if(StringUtil::EqualsIgnoreCase(keyValue.key, "verb"))
                {
                    model.verb = keyValue.value;
                }
                else if(StringUtil::EqualsIgnoreCase(keyValue.key, "initanim"))
                {
                    model.initAnim = gAssetManager.LoadAnimation(keyValue.value, GetScope());
                }
                else if(StringUtil::EqualsIgnoreCase(keyValue.key, "hidden"))
                {
                    model.hidden = true;
                }
                else if(StringUtil::EqualsIgnoreCase(keyValue.key, "gas"))
                {
                    model.gas = gAssetManager.LoadGAS(keyValue.value);
                }
			}
            
            // After parsing all the data, if this is a prop, load the model.
            // For non-props, we don't load a model - the model is baked into the BSP.
            if(!model.name.empty() &&
               (model.type == SceneModel::Type::Prop ||
                model.type == SceneModel::Type::GasProp))
            {
                model.model = gAssetManager.LoadModel(model.name, GetScope());
            }
        }
    }
    
    // Read in regions and triggers.
    std::vector<IniSection> regionSections = parser.GetSections("REGIONS");
    for(auto& section : regionSections)
    {
		mRegions.emplace_back();
		ConditionalBlock<SceneRegionOrTrigger>& regionBlock = mRegions.back();
		
        // Compile and save condition.
        if(!section.condition.empty())
        {
			regionBlock.conditionText = section.condition;
            regionBlock.condition = gSheepManager.Compile("Int Evaluation", section.condition);
        }
        
		// Create each region.
        for(auto& line : section.lines)
        {
			regionBlock.items.emplace_back();
			SceneRegionOrTrigger& region = regionBlock.items.back();
			
			// First pair is always the identifier.
			IniKeyValue& first = line.entries.front();
			region.label = first.key;
            
			// Followed by one or more optional attributes.
			for(size_t i = 1; i < line.entries.size(); ++i)
			{
				IniKeyValue& keyValue = line.entries[i];
				if(StringUtil::EqualsIgnoreCase(keyValue.key, "rect"))
                {
					region.rect = keyValue.GetValueAsRect();
                }
			}
        }
    }
    
    std::vector<IniSection> triggerSections = parser.GetSections("TRIGGERS");
    for(auto& section : triggerSections)
    {
		mTriggers.emplace_back();
		ConditionalBlock<SceneRegionOrTrigger>& triggerBlock = mTriggers.back();
		
        // Compile and save condition.
        if(!section.condition.empty())
        {
			triggerBlock.conditionText = section.condition;
            triggerBlock.condition = gSheepManager.Compile("Int Evaluation", section.condition);
        }
        
		// Create each trigger defined.
        for(auto& line : section.lines)
        {
			triggerBlock.items.emplace_back();
			SceneRegionOrTrigger& region = triggerBlock.items.back();
            
			for(auto& keyValue : line.entries)
			{
				if(StringUtil::EqualsIgnoreCase(keyValue.key, "noun"))
                {
                    region.label = keyValue.value;
                }
                else if(StringUtil::EqualsIgnoreCase(keyValue.key, "rect"))
                {
                    region.rect = keyValue.GetValueAsRect();
                }
			}
        }
    }
    
    std::vector<IniSection> ambientSections = parser.GetSections("AMBIENT");
    for(auto& section : ambientSections)
    {
		mSoundtracks.emplace_back();
		ConditionalBlock<Soundtrack*>& soundtrackBlock = mSoundtracks.back();
		
        // Compile and save condition.
        if(!section.condition.empty())
        {
			soundtrackBlock.conditionText = section.condition;
            soundtrackBlock.condition = gSheepManager.Compile("Int Evaluation", section.condition);
        }
        
		// Add soundtracks.
        for(auto& line : section.lines)
        {
            Soundtrack* soundtrack = gAssetManager.LoadSoundtrack(line.entries[0].key, GetScope());
            if(soundtrack != nullptr)
            {
				soundtrackBlock.items.push_back(soundtrack);
            }
        }
    }

    std::vector<IniSection> conversationSections = parser.GetSections("LISTENERS");
    for(auto& section : conversationSections)
    {
        mConversations.emplace_back();
        ConditionalBlock<SceneConversation>& conversationBlock = mConversations.back();

        // Compile and save condition.
        if(!section.condition.empty())
        {
            conversationBlock.conditionText = section.condition;
            conversationBlock.condition = gSheepManager.Compile("Int Evaluation", section.condition);
        }

        // Add conversation settings.
        for(auto& line : section.lines)
        {
            conversationBlock.items.emplace_back();
            SceneConversation& convo = conversationBlock.items.back();

            for(auto& keyValue : line.entries)
            {
                if(StringUtil::EqualsIgnoreCase(keyValue.key, "dialogue"))
                {
                    convo.name = keyValue.value;
                }
                else if(StringUtil::EqualsIgnoreCase(keyValue.key, "actor"))
                {
                    convo.actorName = keyValue.value;
                }
                else if(StringUtil::EqualsIgnoreCase(keyValue.key, "talk"))
                {
                    convo.talkGas = gAssetManager.LoadGAS(keyValue.value);
                }
                else if(StringUtil::EqualsIgnoreCase(keyValue.key, "listen"))
                {
                    convo.listenGas = gAssetManager.LoadGAS(keyValue.value);
                }
                else if(StringUtil::EqualsIgnoreCase(keyValue.key, "enter"))
                {
                    convo.enterAnim = gAssetManager.LoadAnimation(keyValue.value, GetScope());
                }
                else if(StringUtil::EqualsIgnoreCase(keyValue.key, "exit"))
                {
                    convo.exitAnim = gAssetManager.LoadAnimation(keyValue.value, GetScope());
                }
            }
        }
    }
    
    std::vector<IniSection> actionSections = parser.GetSections("ACTIONS");
    for(auto& section : actionSections)
    {
		mActions.emplace_back();
		ConditionalBlock<NVC*>& actionBlock = mActions.back();
		
        // Compile and save condition.
        if(!section.condition.empty())
        {
			actionBlock.conditionText = section.condition;
            actionBlock.condition = gSheepManager.Compile("Int Evaluation", section.condition);
        }
        
        for(auto& line : section.lines)
        {
            NVC* nvc = gAssetManager.LoadNVC(line.entries[0].key, GetScope());
            if(nvc != nullptr)
            {
                actionBlock.items.push_back(nvc);
            }
        }
    }
}
