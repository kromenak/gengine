//
// SIF.cpp
//
// Clark Kromenaker
//
#include "SIF.h"

#include <iostream>

#include "IniParser.h"
#include "Services.h"
#include "Skybox.h"
#include "StringUtil.h"
#include "WalkerBoundary.h"

SIF::SIF(std::string name, char* data, int dataLength) : Asset(name)
{
    ParseFromData(data, dataLength);
}

SIF::~SIF()
{
	delete mSkybox;
}

const SceneCamera* SIF::GetRoomCamera(const std::string& cameraName) const
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

const ScenePosition* SIF::GetPosition(const std::string& positionName) const
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

void SIF::ParseFromData(char *data, int dataLength)
{
    IniParser parser(data, dataLength);
    parser.ParseAll();
    
    // Read in general section.
    std::vector<IniSection> generals = parser.GetSections("GENERAL");
    for(auto& section : generals)
    {
        // Check condition and early out maybe.
        if(!section.condition.empty())
        {
            SheepScript* sheep = Services::GetSheep()->Compile(section.condition);
            if(!Services::GetSheep()->Evaluate(sheep))
            {
                continue;
            }
        }
        
        for(auto& line : section.lines)
        {
			IniKeyValue& first = line.entries.front();
			if(StringUtil::EqualsIgnoreCase(first.key, "scene"))
            {
                mSceneModelName = first.value;
            }
            else if(StringUtil::EqualsIgnoreCase(first.key, "floor"))
            {
                mFloorModelName = first.value;
            }
            else if(StringUtil::EqualsIgnoreCase(first.key, "boundary"))
            {
				if(mWalkerBoundary == nullptr)
				{
					mWalkerBoundary = new WalkerBoundary();
				}
				
				// First value is name of a texture defining walk bounds.
				mWalkerBoundary->SetTexture(Services::GetAssets()->LoadTexture(first.value));
				
				// Remaining key/values are size/offset of the texture in 3D space.
				// This is used as a 2D overlay on the X/Z plane to determine walkable area.
				for(int i = 1; i < line.entries.size(); ++i)
				{
					IniKeyValue& keyValue = line.entries[i];
					if(StringUtil::EqualsIgnoreCase(keyValue.key, "size"))
                    {
						mWalkerBoundary->SetSize(keyValue.GetValueAsVector2());
                    }
                    else if(StringUtil::EqualsIgnoreCase(keyValue.key, "offset"))
                    {
						mWalkerBoundary->SetOffset(keyValue.GetValueAsVector2());
                    }
				}
            }
            else if(StringUtil::EqualsIgnoreCase(first.key, "cameraBounds"))
            {
                mCameraBoundsModelName = first.value;
				
				// One possible option: a type.
				if(line.entries.size() > 1)
				{
					IniKeyValue& keyValue = line.entries[1];
					if(StringUtil::EqualsIgnoreCase(keyValue.key, "type"))
					{
						mCameraBoundsDynamic = (StringUtil::EqualsIgnoreCase(keyValue.value, "dynamic"));
					}
				}
            }
            else if(StringUtil::EqualsIgnoreCase(first.key, "globalLight"))
            {
				for(int i = 1; i < section.lines.size(); ++i)
				{
					IniKeyValue& keyValue = line.entries[i];
					if(StringUtil::EqualsIgnoreCase(keyValue.key, "pos"))
                    {
                        mGlobalLightPosition = keyValue.GetValueAsVector3();
                    }
                    else if(StringUtil::EqualsIgnoreCase(keyValue.key, "ambient"))
                    {
                        mGlobalLightAmbient = keyValue.GetValueAsVector3();
                    }
				}
            }
            else if(StringUtil::EqualsIgnoreCase(first.key, "skybox"))
            {
				mSkybox = new Skybox();
				
				for(int i = 1; i < section.lines.size(); ++i)
				{
					IniKeyValue& keyValue = line.entries[i];
					
					Texture* texture = Services::GetAssets()->LoadTexture(keyValue.value);
                    if(StringUtil::EqualsIgnoreCase(keyValue.key, "left"))
                    {
                        mSkybox->SetLeftTexture(texture);
                    }
                    else if(StringUtil::EqualsIgnoreCase(keyValue.key, "right"))
                    {
                        mSkybox->SetRightTexture(texture);
                    }
                    else if(StringUtil::EqualsIgnoreCase(keyValue.key, "front"))
                    {
                        mSkybox->SetFrontTexture(texture);
                    }
                    else if(StringUtil::EqualsIgnoreCase(keyValue.key, "back"))
                    {
                        mSkybox->SetBackTexture(texture);
                    }
                    else if(StringUtil::EqualsIgnoreCase(keyValue.key, "up"))
                    {
                        mSkybox->SetUpTexture(texture);
                    }
                    else if(StringUtil::EqualsIgnoreCase(keyValue.key, "down"))
                    {
                        mSkybox->SetDownTexture(texture);
                    }
					
				}
            }
        }
    }
    
    // Read in cameras.
    std::vector<IniSection> inspectCameras = parser.GetSections("INSPECT_CAMERAS");
    for(auto& section : inspectCameras)
    {
        // Check condition and early out maybe.
        if(!section.condition.empty())
        {
            SheepScript* sheep = Services::GetSheep()->Compile(section.condition);
            if(!Services::GetSheep()->Evaluate(sheep))
            {
                continue;
            }
        }
        
        for(auto& line : section.lines)
        {
            SceneCamera* camera = new SceneCamera();
			for(auto& keyValue : line.entries)
			{
				if(StringUtil::EqualsIgnoreCase(keyValue.key, "noun") || StringUtil::EqualsIgnoreCase(keyValue.key, "model"))
                {
                    camera->label = keyValue.value;
                }
                else if(StringUtil::EqualsIgnoreCase(keyValue.key, "pos"))
                {
                    camera->position = keyValue.GetValueAsVector3();
                }
                else if(StringUtil::EqualsIgnoreCase(keyValue.key, "angle"))
                {
                    // Angle will be in degrees, but we want it in radians for internal use.
					Vector2 angleDeg = keyValue.GetValueAsVector2();
                    camera->angle.SetX(Math::ToRadians(angleDeg.GetX()));
                    camera->angle.SetY(Math::ToRadians(angleDeg.GetY()));
                }
			}
			
			// Add to inspect cameras.
			mInspectCameras.push_back(camera);
        }
    }
    
    std::vector<IniSection> roomCameras = parser.GetSections("ROOM_CAMERAS");
    for(auto& section : roomCameras)
    {
        // Check condition and early out maybe.
        if(!section.condition.empty())
        {
            SheepScript* sheep = Services::GetSheep()->Compile(section.condition);
            if(!Services::GetSheep()->Evaluate(sheep))
            {
                continue;
            }
        }
        
        for(auto& line : section.lines)
        {
            SceneCamera* camera = new SceneCamera();
			
			// First keyword for room cameras is always the camera name.
			IniKeyValue& first = line.entries.front();
			camera->label = first.key;
			
			// Followed by one or more optional attributes.
			for(int i = 1; i < line.entries.size(); ++i)
			{
				IniKeyValue& keyValue = line.entries[i];
				if(StringUtil::EqualsIgnoreCase(keyValue.key, "pos"))
                {
                    camera->position = keyValue.GetValueAsVector3();
                }
                else if(StringUtil::EqualsIgnoreCase(keyValue.key, "angle"))
                {
					// Angle is in degrees, but we use radians internally.
					Vector2 angleDeg = keyValue.GetValueAsVector2();
                    camera->angle.SetX(Math::ToRadians(angleDeg.GetX()));
                    camera->angle.SetY(Math::ToRadians(angleDeg.GetY()));
                }
                else if(StringUtil::EqualsIgnoreCase(keyValue.key, "default"))
                {
                    // Save index for default room camera. Equals size, since about to add to list.
                    mDefaultRoomCameraIndex = (int)mRoomCameras.size();
                }
			}
			
			// Add to room cameras.
			mRoomCameras.push_back(camera);
        }
    }
    
    std::vector<IniSection> cinematicCameras = parser.GetSections("CINEMATIC_CAMERAS");
    for(auto& section : cinematicCameras)
    {
        // Check condition and early out maybe.
        if(!section.condition.empty())
        {
            SheepScript* sheep = Services::GetSheep()->Compile(section.condition);
            if(!Services::GetSheep()->Evaluate(sheep))
            {
                continue;
            }
        }
        
        for(auto& line : section.lines)
        {
            SceneCamera* camera = new SceneCamera();
			
			// First keyword for cinematic cameras is always the camera name.
			IniKeyValue& first = line.entries.front();
			camera->label = first.key;
            
			// Followed by one or more optional attributes.
			for(int i = 1; i < line.entries.size(); ++i)
			{
				IniKeyValue& keyValue = line.entries[i];
				if(StringUtil::EqualsIgnoreCase(keyValue.key, "pos"))
                {
                    camera->position = keyValue.GetValueAsVector3();
                }
                else if(StringUtil::EqualsIgnoreCase(keyValue.key, "angle"))
                {
					// Angle is in degrees, but we use radians internally.
					Vector2 angleDeg = keyValue.GetValueAsVector2();
                    camera->angle.SetX(Math::ToRadians(angleDeg.GetX()));
                    camera->angle.SetY(Math::ToRadians(angleDeg.GetY()));
                }
			}
			
			// Add to cinematic cameras.
            mCinematicCameras.push_back(camera);
        }
    }
    
    std::vector<IniSection> DialogueSceneCameras = parser.GetSections("DIALOGUE_CAMERAS");
    for(auto& section : DialogueSceneCameras)
    {
        // Check condition and early out maybe.
        if(!section.condition.empty())
        {
            SheepScript* sheep = Services::GetSheep()->Compile(section.condition);
            if(!Services::GetSheep()->Evaluate(sheep))
            {
                continue;
            }
        }
        
        for(auto& line : section.lines)
        {
            DialogueSceneCamera* camera = new DialogueSceneCamera();
			
			// First keyword for dialogue cameras is always the camera name.
			IniKeyValue& first = line.entries.front();
			camera->label = first.key;
            
			// Followed by one or more optional attributes.
			for(int i = 1; i < line.entries.size(); ++i)
			{
				IniKeyValue& keyValue = line.entries[i];
				if(StringUtil::EqualsIgnoreCase(keyValue.key, "pos"))
                {
                    camera->position = keyValue.GetValueAsVector3();
                }
                else if(StringUtil::EqualsIgnoreCase(keyValue.key, "angle"))
                {
					// Angle is in degrees, but we use radians internally.
					Vector2 angleDeg = keyValue.GetValueAsVector2();
                    camera->angle.SetX(Math::ToRadians(angleDeg.GetX()));
                    camera->angle.SetY(Math::ToRadians(angleDeg.GetY()));
                }
				else if(StringUtil::EqualsIgnoreCase(keyValue.key, "dialogue"))
                {
                    camera->dialogueName = keyValue.value;
                }
                else if(StringUtil::EqualsIgnoreCase(keyValue.key, "set"))
                {
                    camera->setName = keyValue.value;
                }
				else if(StringUtil::EqualsIgnoreCase(keyValue.key, "show"))
                {
                    camera->showInToolbar = true;
                }
                else if(StringUtil::EqualsIgnoreCase(keyValue.key, "final"))
                {
                    camera->isFinal = true;
                }
			}
			
			// Add to dialogue cameras.
			mDialogueSceneCameras.push_back(camera);
        }
    }
    
    // Read in positions.
    std::vector<IniSection> positionSections = parser.GetSections("POSITIONS");
    for(auto& section : positionSections)
    {
        // Check condition and early out maybe.
        if(!section.condition.empty())
        {
            SheepScript* sheep = Services::GetSheep()->Compile(section.condition);
            if(!Services::GetSheep()->Evaluate(sheep))
            {
                continue;
            }
        }
        
        for(auto& line : section.lines)
        {
            ScenePosition* position = new ScenePosition();
			
			// First pair is always the identifier.
			IniKeyValue& first = line.entries.front();
			position->label = first.key;
            
			// Followed by one or more optional attributes.
			for(int i = 1; i < line.entries.size(); ++i)
			{
				IniKeyValue& keyValue = line.entries[i];
				if(StringUtil::EqualsIgnoreCase(keyValue.key, "pos"))
                {
                    position->position = keyValue.GetValueAsVector3();
                }
                else if(StringUtil::EqualsIgnoreCase(keyValue.key, "heading"))
                {
					position->heading = Heading::FromDegrees(keyValue.GetValueAsFloat());
                }
                else if(StringUtil::EqualsIgnoreCase(keyValue.key, "camera"))
                {
                    for(auto& cam : mRoomCameras)
                    {
                        if(StringUtil::EqualsIgnoreCase(cam->label, keyValue.value))
                        {
                            position->camera = cam;
                            break;
                        }
                    }
                }
			}
			
			// Add to positions.
            mPositions.push_back(position);
        }
    }
    
    // Read in actors.
    std::vector<IniSection> actorSections = parser.GetSections("ACTORS");
    for(auto& section : actorSections)
    {
        // Check condition and early out maybe.
        if(!section.condition.empty())
        {
            SheepScript* sheep = Services::GetSheep()->Compile(section.condition);
            if(!Services::GetSheep()->Evaluate(sheep))
            {
                continue;
            }
        }
        
        for(auto& line : section.lines)
        {
            SceneActor* actor = new SceneActor();
			
			for(auto& keyValue : line.entries)
			{
				if(StringUtil::EqualsIgnoreCase(keyValue.key, "model"))
                {
                    actor->model = Services::GetAssets()->LoadModel(keyValue.value);
                }
                else if(StringUtil::EqualsIgnoreCase(keyValue.key, "noun"))
                {
                    actor->noun = keyValue.value;
                }
                else if(StringUtil::EqualsIgnoreCase(keyValue.key, "pos"))
                {
                    for(auto& position : mPositions)
                    {
                        if(StringUtil::EqualsIgnoreCase(position->label, keyValue.value))
                        {
                            actor->position = position;
                            break;
                        }
                    }
                }
                else if(StringUtil::EqualsIgnoreCase(keyValue.key, "idle"))
                {
                    actor->idleGas = Services::GetAssets()->LoadGAS(keyValue.value);
                }
                else if(StringUtil::EqualsIgnoreCase(keyValue.key, "talk"))
                {
                    actor->talkGas = Services::GetAssets()->LoadGAS(keyValue.value);
                }
                else if(StringUtil::EqualsIgnoreCase(keyValue.key, "listen"))
                {
                    actor->listenGas = Services::GetAssets()->LoadGAS(keyValue.value);
                }
                else if(StringUtil::EqualsIgnoreCase(keyValue.key, "initAnim"))
                {
                    actor->initAnim = Services::GetAssets()->LoadAnimation(keyValue.value);
                }
                else if(StringUtil::EqualsIgnoreCase(keyValue.key, "hidden"))
                {
                    actor->hidden = true;
                }
                else if(StringUtil::EqualsIgnoreCase(keyValue.key, "ego"))
                {
                    actor->ego = true;
                }
			}
            
            // If no position was set, try a default for now.
            if(actor->position == nullptr && mPositions.size() > 0)
            {
                actor->position = mPositions[0];
            }
            
            // If no GAS files were loaded, try to use defaults.
            if(actor->idleGas == nullptr)
            {
                actor->idleGas = Services::GetAssets()->LoadGAS(actor->model->GetNameNoExtension() + "Idle");
            }
            if(actor->talkGas == nullptr)
            {
                actor->talkGas = Services::GetAssets()->LoadGAS(actor->model->GetNameNoExtension() + "Talk");
            }
            if(actor->listenGas == nullptr)
            {
                actor->listenGas = actor->talkGas;
            }
			
			// Add to actors.
            mSceneActors.push_back(actor);
        }
    }
    
    // Read in models.
    std::vector<IniSection> modelSections = parser.GetSections("MODELS");
    for(auto& section : modelSections)
    {
        // Check condition and early out maybe.
        if(!section.condition.empty())
        {
            SheepScript* sheep = Services::GetSheep()->Compile(section.condition);
            if(!Services::GetSheep()->Evaluate(sheep))
            {
                //std::cout << "Condition was NOT true: " << section.condition << std::endl;
                continue;
            }
            //std::cout << "Condition was true: " << section.condition << std::endl;
        }
        
        for(auto& line : section.lines)
        {
            SceneModel* model = new SceneModel();
            
			for(auto& keyValue : line.entries)
			{
				if(StringUtil::EqualsIgnoreCase(keyValue.key, "model"))
                {
                    model->name = keyValue.value;
                }
                else if(StringUtil::EqualsIgnoreCase(keyValue.key, "noun"))
                {
                    model->noun = keyValue.value;
                }
                else if(StringUtil::EqualsIgnoreCase(keyValue.key, "type"))
                {
                    if(StringUtil::EqualsIgnoreCase(keyValue.value, "scene"))
                    {
                        model->type = SceneModel::Type::Scene;
                    }
                    else if(StringUtil::EqualsIgnoreCase(keyValue.value, "prop"))
                    {
                        model->type = SceneModel::Type::Prop;
                    }
                    else if(StringUtil::EqualsIgnoreCase(keyValue.value, "hittest"))
                    {
                        model->type = SceneModel::Type::HitTest;
                    }
                    else if(StringUtil::EqualsIgnoreCase(keyValue.value, "gasprop"))
                    {
                        model->type = SceneModel::Type::GasProp;
                    }
                }
                else if(StringUtil::EqualsIgnoreCase(keyValue.key, "verb"))
                {
                    model->verb = keyValue.value;
                }
                else if(StringUtil::EqualsIgnoreCase(keyValue.key, "initanim"))
                {
                    model->initAnim = Services::GetAssets()->LoadAnimation(keyValue.value);
                }
                else if(StringUtil::EqualsIgnoreCase(keyValue.key, "hidden"))
                {
                    model->hidden = true;
                }
                else if(StringUtil::EqualsIgnoreCase(keyValue.key, "gas"))
                {
                    model->gas = Services::GetAssets()->LoadGAS(keyValue.value);
                }
				
			}
            
            // After parsing all the data, if this is a prop, load the model.
            // For non-props, we don't load a model - the model is baked into the BSP.
            if(!model->name.empty() &&
               (model->type == SceneModel::Type::Prop ||
                model->type == SceneModel::Type::GasProp))
            {
                model->model = Services::GetAssets()->LoadModel(model->name);
            }
            mSceneModels.push_back(model);
        }
    }
    
    // Read in regions and triggers.
    std::vector<IniSection> regionSections = parser.GetSections("REGIONS");
    for(auto& section : regionSections)
    {
        // Check condition and early out maybe.
        if(!section.condition.empty())
        {
            SheepScript* sheep = Services::GetSheep()->Compile(section.condition);
            if(!Services::GetSheep()->Evaluate(sheep))
            {
                continue;
            }
        }
        
        for(auto& line : section.lines)
        {
            SceneRegionOrTrigger* region = new SceneRegionOrTrigger();
			
			// First pair is always the identifier.
			IniKeyValue& first = line.entries.front();
			region->label = first.key;
            
			// Followed by one or more optional attributes.
			for(int i = 1; i < line.entries.size(); ++i)
			{
				IniKeyValue& keyValue = line.entries[i];
				if(StringUtil::EqualsIgnoreCase(keyValue.key, "rect"))
                {
                    //TODO: read in rect.
                }
			}
				
			// Add to regions.
            mRegions.push_back(region);
        }
    }
    
    std::vector<IniSection> triggerSections = parser.GetSections("TRIGGERS");
    for(auto& section : triggerSections)
    {
        // Check condition and early out maybe.
        if(!section.condition.empty())
        {
            SheepScript* sheep = Services::GetSheep()->Compile(section.condition);
            if(!Services::GetSheep()->Evaluate(sheep))
            {
                continue;
            }
        }
        
        for(auto& line : section.lines)
        {
            SceneRegionOrTrigger* region = new SceneRegionOrTrigger();
            
			for(auto& keyValue : line.entries)
			{
				if(StringUtil::EqualsIgnoreCase(keyValue.key, "noun"))
                {
                    region->label = keyValue.value;
                }
                if(StringUtil::EqualsIgnoreCase(keyValue.key, "rect"))
                {
                    //TODO: read in rect.
                }
			}
			
			// Add to triggers.
            mTriggers.push_back(region);
        }
    }
    
    std::vector<IniSection> ambientSections = parser.GetSections("AMBIENT");
    for(auto& section : ambientSections)
    {
        // Check condition and early out maybe.
        if(!section.condition.empty())
        {
            SheepScript* sheep = Services::GetSheep()->Compile(section.condition);
            if(!Services::GetSheep()->Evaluate(sheep))
            {
                continue;
            }
        }
        
        for(auto& line : section.lines)
        {
            Soundtrack* soundtrack = Services::GetAssets()->LoadSoundtrack(line.entries[0].key);
            if(soundtrack != nullptr)
            {
                mSoundtracks.push_back(soundtrack);
            }
        }
    }
    
    std::vector<IniSection> actionSections = parser.GetSections("ACTIONS");
    for(auto& section : actionSections)
    {
        // Check condition and early out maybe.
        if(!section.condition.empty())
        {
            SheepScript* sheep = Services::GetSheep()->Compile(section.condition);
            if(!Services::GetSheep()->Evaluate(sheep))
            {
                continue;
            }
        }
        
        for(auto& line : section.lines)
        {
            //TODO: Should only read in NVC files that correspond to the current day.
            //Will have a number like "1" for day 1.
            
            NVC* nvc = Services::GetAssets()->LoadNVC(line.entries[0].key);
            if(nvc != nullptr)
            {
                mNVCs.push_back(nvc);
            }
        }
    }
}
