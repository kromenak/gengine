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

SIF::SIF(std::string name, char* data, int dataLength) : Asset(name)
{
    ParseFromData(data, dataLength);
}

Color32 SIF::GetWalkBoundaryColor(Vector3 position) const
{
	// If no texture...can walk anywhere?
	if(mWalkBoundaryTexture == nullptr) { return Color32::White; }
	//std::cout << "World Pos: " << position << std::endl;
	
	position.SetX(position.GetX() + mWalkBoundaryOffset.GetX());
	position.SetZ(position.GetZ() + mWalkBoundaryOffset.GetY());
	//std::cout << "Offset Pos: " << position << std::endl;
	
	position.SetX(position.GetX() / mWalkBoundarySize.GetX());
	position.SetZ(position.GetZ() / mWalkBoundarySize.GetY());
	//std::cout << "Normalized Pos: " << position << std::endl;
	
	position.SetX(position.GetX() * mWalkBoundaryTexture->GetWidth());
	position.SetZ(position.GetZ() * mWalkBoundaryTexture->GetHeight());
	//std::cout << "Pixel Pos: " << position << std::endl;
	
	// The color of the pixel at pos seems to indicate whether that spot is walkable.
	// White = totally OK to walk 				(255, 255, 255)
	// Blue = OK to walk						(0, 0, 255)
	// Green = sort of OK to walk 				(0, 255, 0)
	// Red = getting less OK to walk 			(255, 0, 0)
	// Yellow = sort of not OK to walk 			(255, 255, 0)
	// Magenta = really pushing it here 		(255, 0, 255)
	// Grey = pretty not OK to walk here 		(128, 128, 128)
	// Cyan = this is your last warning, buddy 	(0, 255, 255)
	// Black = totally not OK to walk 			(0, 0, 0)
	
	// Need to flip the Y because the calculated value is from lower-left. But X/Y are from upper-left.
	return mWalkBoundaryTexture->GetPixelColor32(position.GetX(), mWalkBoundaryTexture->GetHeight() - position.GetZ());
}

ScenePositionData* SIF::GetPosition(std::string positionName)
{
    for(int i = 0; i < mPositions.size(); i++)
    {
        if(mPositions[i]->label == positionName)
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
        
        for(auto& entry : section.entries)
        {
            IniKeyValue* keyValue = entry;
			
			if(StringUtil::EqualsIgnoreCase(keyValue->key, "scene"))
            {
                mSceneModelName = keyValue->value;
            }
            else if(StringUtil::EqualsIgnoreCase(keyValue->key, "floor"))
            {
                mFloorBspModelName = keyValue->value;
            }
            else if(StringUtil::EqualsIgnoreCase(keyValue->key, "boundary"))
            {
				// First value is name of a texture defining walk bounds.
				mWalkBoundaryTexture = Services::GetAssets()->LoadTexture(keyValue->value);
				
				// Remaining key/values are size/offset of the texture in 3D space.
				// This is used as a 2D overlay on the X/Z plane to determine walkable area.
                keyValue = keyValue->next;
                while(keyValue != nullptr)
                {
                    if(StringUtil::EqualsIgnoreCase(keyValue->key, "size"))
                    {
                        mWalkBoundarySize = keyValue->GetValueAsVector2();
                    }
                    else if(StringUtil::EqualsIgnoreCase(keyValue->key, "offset"))
                    {
                        mWalkBoundaryOffset = keyValue->GetValueAsVector2();
                    }
                    keyValue = keyValue->next;
                }
            }
            else if(StringUtil::EqualsIgnoreCase(keyValue->key, "cameraBounds"))
            {
                mCameraBoundsModelName = keyValue->value;
                keyValue = keyValue->next;
                if(keyValue != nullptr && StringUtil::EqualsIgnoreCase(keyValue->key, "type"))
                {
                    mCameraBoundsDynamic = (StringUtil::EqualsIgnoreCase(keyValue->value, "dynamic"));
                }
            }
            else if(StringUtil::EqualsIgnoreCase(keyValue->key, "globalLight"))
            {
                keyValue = keyValue->next;
                while(keyValue != nullptr)
                {
                    if(StringUtil::EqualsIgnoreCase(keyValue->key, "pos"))
                    {
                        mGlobalLightPosition = keyValue->GetValueAsVector3();
                    }
                    else if(StringUtil::EqualsIgnoreCase(keyValue->key, "ambient"))
                    {
                        mGlobalLightAmbient = keyValue->GetValueAsVector3();
                    }
                    keyValue = keyValue->next;
                }
            }
            else if(StringUtil::EqualsIgnoreCase(keyValue->key, "skybox"))
            {
                keyValue = keyValue->next;
                
                mSkybox = new Skybox();
                while(keyValue != nullptr)
                {
                    Texture* texture = Services::GetAssets()->LoadTexture(keyValue->value);
                    if(StringUtil::EqualsIgnoreCase(keyValue->key, "left"))
                    {
                        mSkybox->SetLeftTexture(texture);
                    }
                    else if(StringUtil::EqualsIgnoreCase(keyValue->key, "right"))
                    {
                        mSkybox->SetRightTexture(texture);
                    }
                    else if(StringUtil::EqualsIgnoreCase(keyValue->key, "front"))
                    {
                        mSkybox->SetFrontTexture(texture);
                    }
                    else if(StringUtil::EqualsIgnoreCase(keyValue->key, "back"))
                    {
                        mSkybox->SetBackTexture(texture);
                    }
                    else if(StringUtil::EqualsIgnoreCase(keyValue->key, "up"))
                    {
                        mSkybox->SetUpTexture(texture);
                    }
                    else if(StringUtil::EqualsIgnoreCase(keyValue->key, "down"))
                    {
                        mSkybox->SetDownTexture(texture);
                    }
                    keyValue = keyValue->next;
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
        
        for(auto& entry : section.entries)
        {
            SceneCameraData* camera = new SceneCameraData();
            IniKeyValue* keyValue = entry;
            while(keyValue != nullptr)
            {
                if(keyValue->key == "noun" || keyValue->key == "model")
                {
                    camera->label = keyValue->value;
                }
                else if(keyValue->key == "pos")
                {
                    camera->position = keyValue->GetValueAsVector3();
                }
                else if(keyValue->key == "angle")
                {
                    // Angle will be in degrees, but we want it in radians for internal use.
                    camera->angle = keyValue->GetValueAsVector2();
                    camera->angle.SetX(Math::ToRadians(camera->angle.GetX()));
                    camera->angle.SetY(Math::ToRadians(camera->angle.GetY()));
                }
                keyValue = keyValue->next;
            }
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
        
        for(auto& entry : section.entries)
        {
            SceneCameraData* camera = new SceneCameraData();
            camera->label = entry->key;
            
            IniKeyValue* keyValue = entry->next;
            while(keyValue != nullptr)
            {
                if(keyValue->key == "pos")
                {
                    camera->position = keyValue->GetValueAsVector3();
                }
                else if(keyValue->key == "angle")
                {
                    camera->angle = keyValue->GetValueAsVector2();
                    camera->angle.SetX(Math::ToRadians(camera->angle.GetX()));
                    camera->angle.SetY(Math::ToRadians(camera->angle.GetY()));
                }
                else if(keyValue->key == "default")
                {
                    // Save index for default room camera. Equals size, since about to add to list.
                    mDefaultRoomCameraIndex = (int)mRoomCameras.size();
                }
                keyValue = keyValue->next;
            }
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
        
        for(auto& entry : section.entries)
        {
            SceneCameraData* camera = new SceneCameraData();
            camera->label = entry->key;
            
            IniKeyValue* keyValue = entry->next;
            while(keyValue != nullptr)
            {
                if(keyValue->key == "pos")
                {
                    camera->position = keyValue->GetValueAsVector3();
                }
                else if(keyValue->key == "angle")
                {
                    camera->angle = keyValue->GetValueAsVector2();
                    camera->angle.SetX(Math::ToRadians(camera->angle.GetX()));
                    camera->angle.SetY(Math::ToRadians(camera->angle.GetY()));
                }
                keyValue = keyValue->next;
            }
            mCinematicCameras.push_back(camera);
        }
    }
    
    std::vector<IniSection> DialogueSceneCameraDatas = parser.GetSections("DIALOGUE_CAMERAS");
    for(auto& section : DialogueSceneCameraDatas)
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
        
        for(auto& entry : section.entries)
        {
            DialogueSceneCameraData* camera = new DialogueSceneCameraData();
            camera->label = entry->key;
            
            IniKeyValue* keyValue = entry->next;
            while(keyValue != nullptr)
            {
                if(keyValue->key == "dialogue")
                {
                    camera->dialogueName = keyValue->value;
                }
                else if(keyValue->key == "set")
                {
                    camera->setName = keyValue->value;
                }
                else if(keyValue->key == "pos")
                {
                    camera->position = keyValue->GetValueAsVector3();
                }
                else if(keyValue->key == "angle")
                {
                    camera->angle = keyValue->GetValueAsVector2();
                    camera->angle.SetX(Math::ToRadians(camera->angle.GetX()));
                    camera->angle.SetY(Math::ToRadians(camera->angle.GetY()));
                }
                else if(keyValue->key == "show")
                {
                    camera->showInToolbar = true;
                }
                else if(keyValue->key == "final")
                {
                    camera->isFinal = true;
                }
                keyValue = keyValue->next;
            }
            mDialogueSceneCameraDatas.push_back(camera);
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
        
        for(auto& entry : section.entries)
        {
            // First pair is always the identifier.
            ScenePositionData* position = new ScenePositionData();
            position->label = entry->key;
            
            // Remaining pairs are optional and in any order.
            IniKeyValue* keyValue = entry->next;
            while(keyValue != nullptr)
            {
                if(keyValue->key == "pos")
                {
                    position->position = keyValue->GetValueAsVector3();
                }
                else if(keyValue->key == "heading")
                {
                    position->heading = Math::ToRadians(keyValue->GetValueAsFloat());
                }
                else if(keyValue->key == "camera")
                {
                    for(auto& cam : mRoomCameras)
                    {
                        if(cam->label == keyValue->value)
                        {
                            position->camera = cam;
                            break;
                        }
                    }
                }
                keyValue = keyValue->next;
            }
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
        
        for(auto& entry : section.entries)
        {
            SceneActorData* actor = new SceneActorData();
            IniKeyValue* keyValue = entry;
            while(keyValue != nullptr)
            {
                if(keyValue->key == "model")
                {
                    actor->model = Services::GetAssets()->LoadModel(keyValue->value);
                }
                else if(keyValue->key == "noun")
                {
                    actor->noun = keyValue->value;
                }
                else if(keyValue->key == "pos")
                {
                    for(auto& position : mPositions)
                    {
                        if(position->label == keyValue->value)
                        {
                            actor->position = position;
                            break;
                        }
                    }
                }
                else if(keyValue->key == "idle")
                {
                    actor->idleGas = Services::GetAssets()->LoadGAS(keyValue->value);
                }
                else if(keyValue->key == "talk")
                {
                    actor->talkGas = Services::GetAssets()->LoadGAS(keyValue->value);
                }
                else if(keyValue->key == "listen")
                {
                    actor->listenGas = Services::GetAssets()->LoadGAS(keyValue->value);
                }
                else if(keyValue->key == "initAnim")
                {
                    actor->initAnim = Services::GetAssets()->LoadAnimation(keyValue->value);
                }
                else if(keyValue->key == "hidden")
                {
                    actor->hidden = true;
                }
                else if(keyValue->key == "ego")
                {
                    actor->ego = true;
                }
                keyValue = keyValue->next;
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
            mSceneActorDatas.push_back(actor);
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
        
        for(auto& entry : section.entries)
        {
            SceneModelData* model = new SceneModelData();
            
            IniKeyValue* keyValue = entry;
            while(keyValue != nullptr)
            {
				if(StringUtil::EqualsIgnoreCase(keyValue->key, "model"))
                {
                    model->name = keyValue->value;
                }
                else if(StringUtil::EqualsIgnoreCase(keyValue->key, "noun"))
                {
                    model->noun = keyValue->value;
                }
                else if(StringUtil::EqualsIgnoreCase(keyValue->key, "type"))
                {
                    if(StringUtil::EqualsIgnoreCase(keyValue->value, "scene"))
                    {
                        model->type = SceneModelData::Type::Scene;
                    }
                    else if(StringUtil::EqualsIgnoreCase(keyValue->value, "prop"))
                    {
                        model->type = SceneModelData::Type::Prop;
                    }
                    else if(StringUtil::EqualsIgnoreCase(keyValue->value, "hittest"))
                    {
                        model->type = SceneModelData::Type::HitTest;
                    }
                    else if(StringUtil::EqualsIgnoreCase(keyValue->value, "gasprop"))
                    {
                        model->type = SceneModelData::Type::GasProp;
                    }
                }
                else if(StringUtil::EqualsIgnoreCase(keyValue->key, "verb"))
                {
                    model->verb = keyValue->value;
                }
                else if(StringUtil::EqualsIgnoreCase(keyValue->key, "initanim"))
                {
                    model->initAnim = Services::GetAssets()->LoadAnimation(keyValue->value);
                }
                else if(StringUtil::EqualsIgnoreCase(keyValue->key, "hidden"))
                {
                    model->hidden = true;
                }
                else if(StringUtil::EqualsIgnoreCase(keyValue->key, "gas"))
                {
                    model->gas = Services::GetAssets()->LoadGAS(keyValue->value);
                }
                keyValue = keyValue->next;
            }
            
            // After parsing all the data, if this is a prop, load the model.
            // For non-props, we don't load a model - the model is baked into the BSP.
            if(!model->name.empty() &&
               (model->type == SceneModelData::Type::Prop ||
                model->type == SceneModelData::Type::GasProp))
            {
                model->model = Services::GetAssets()->LoadModel(model->name + ".MOD");
            }
            mSceneModelDatas.push_back(model);
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
        
        for(auto& entry : section.entries)
        {
            SceneRegionOrTriggerData* region = new SceneRegionOrTriggerData();
            region->label = entry->key;
            
            IniKeyValue* keyValue = entry->next;
            while(keyValue != nullptr)
            {
                if(keyValue->key == "rect")
                {
                    //TODO: read in rect.
                }
                keyValue = keyValue->next;
            }
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
        
        for(auto& entry : section.entries)
        {
            SceneRegionOrTriggerData* region = new SceneRegionOrTriggerData();
            
            IniKeyValue* keyValue = entry;
            while(keyValue != nullptr)
            {
                if(keyValue->key == "noun")
                {
                    region->label = keyValue->value;
                }
                if(keyValue->key == "rect")
                {
                    //TODO: read in rect.
                }
                keyValue = keyValue->next;
            }
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
        
        for(auto& entry : section.entries)
        {
            Soundtrack* soundtrack = Services::GetAssets()->LoadSoundtrack(entry->key);
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
        
        for(auto& entry : section.entries)
        {
            //TODO: Should only read in NVC files that correspond to the current day.
            //Will have a number like "1" for day 1.
            
            NVC* nvc = Services::GetAssets()->LoadNVC(entry->key);
            if(nvc != nullptr)
            {
                mNVCs.push_back(nvc);
            }
        }
    }
    
}
