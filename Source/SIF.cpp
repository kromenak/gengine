//
// SIF.cpp
//
// Clark Kromenaker
//
#include "SIF.h"
#include "IniParser.h"
#include <iostream>
#include "StringUtil.h"
#include "Services.h"

SIF::SIF(std::string name, char* data, int dataLength) : Asset(name)
{
    ParseFromData(data, dataLength);
}

void SIF::ParseFromData(char *data, int dataLength)
{
    IniParser parser(data, dataLength);
    parser.ParseAll();
    
    // Read in general section.
    std::vector<IniSection> generals = parser.GetSections("GENERAL");
    for(auto& section : generals)
    {
        //TODO: Check condition?
        for(auto& entry : section.entries)
        {
            IniKeyValue* keyValue = entry;
            if(keyValue->key == "scene")
            {
                mSceneAssetName = keyValue->value;
            }
            else if(keyValue->key == "floor")
            {
                mFloorSceneModelName = keyValue->value;
            }
            else if(keyValue->key == "boundary")
            {
                mWalkBoundaryTextureName = keyValue->value;
                keyValue = keyValue->next;
                while(keyValue != nullptr)
                {
                    if(keyValue->key == "size")
                    {
                        mWalkBoundarySize = keyValue->GetValueAsVector2();
                    }
                    else if(keyValue->key == "offset")
                    {
                        mWalkBoundaryOffset = keyValue->GetValueAsVector2();
                    }
                    keyValue = keyValue->next;
                }
            }
            else if(keyValue->key == "cameraBounds")
            {
                mCameraBoundsModelName = keyValue->value;
                keyValue = keyValue->next;
                if(keyValue != nullptr && keyValue->key == "type")
                {
                    mCameraBoundsDynamic = (keyValue->value == "dynamic");
                }
            }
            else if(keyValue->key == "globalLight")
            {
                keyValue = keyValue->next;
                while(keyValue != nullptr)
                {
                    if(keyValue->key == "pos")
                    {
                        mGlobalLightPosition = keyValue->GetValueAsVector3();
                    }
                    else if(keyValue->key == "ambient")
                    {
                        mGlobalLightAmbient = keyValue->GetValueAsVector3();
                    }
                    keyValue = keyValue->next;
                }
            }
            else if(keyValue->key == "skybox")
            {
                keyValue = keyValue->next;
                while(keyValue != nullptr)
                {
                    if(keyValue->key == "left")
                    {
                        mSkybox.leftTexture = Services::GetAssets()->LoadTexture(keyValue->value + ".BMP");
                    }
                    else if(keyValue->key == "right")
                    {
                        mSkybox.rightTexture = Services::GetAssets()->LoadTexture(keyValue->value + ".BMP");
                    }
                    else if(keyValue->key == "front")
                    {
                        mSkybox.frontTexture = Services::GetAssets()->LoadTexture(keyValue->value + ".BMP");
                    }
                    else if(keyValue->key == "back")
                    {
                        mSkybox.backTexture = Services::GetAssets()->LoadTexture(keyValue->value + ".BMP");
                    }
                    else if(keyValue->key == "up")
                    {
                        mSkybox.upTexture = Services::GetAssets()->LoadTexture(keyValue->value + ".BMP");
                    }
                    else if(keyValue->key == "down")
                    {
                        mSkybox.downTexture = Services::GetAssets()->LoadTexture(keyValue->value + ".BMP");
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
        //TODO: Check condition?
        for(auto& entry : section.entries)
        {
            SceneCamera* camera = new SceneCamera();
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
                    camera->angle = keyValue->GetValueAsVector2();
                }
                keyValue = keyValue->next;
            }
            mInspectCameras.push_back(camera);
        }
    }
    
    std::vector<IniSection> roomCameras = parser.GetSections("ROOM_CAMERAS");
    for(auto& section : roomCameras)
    {
        //TODO: Check condition?
        for(auto& entry : section.entries)
        {
            SceneCamera* camera = new SceneCamera();
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
                }
                else if(keyValue->key == "default")
                {
                    camera->isDefault = true;
                }
                keyValue = keyValue->next;
            }
            mRoomCameras.push_back(camera);
        }
    }
    
    std::vector<IniSection> cinematicCameras = parser.GetSections("CINEMATIC_CAMERAS");
    for(auto& section : cinematicCameras)
    {
        //TODO: Check condition?
        for(auto& entry : section.entries)
        {
            SceneCamera* camera = new SceneCamera();
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
                }
                keyValue = keyValue->next;
            }
            mCinematicCameras.push_back(camera);
        }
    }
    
    std::vector<IniSection> dialogueCameras = parser.GetSections("DIALOGUE_CAMERAS");
    for(auto& section : dialogueCameras)
    {
        //TODO: Check condition?
        for(auto& entry : section.entries)
        {
            DialogueCamera* camera = new DialogueCamera();
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
            mDialogueCameras.push_back(camera);
        }
    }
    
    // Read in positions.
    std::vector<IniSection> positionSections = parser.GetSections("POSITIONS");
    for(auto& section : positionSections)
    {
        //TODO: Check condition?
        for(auto& entry : section.entries)
        {
            // First pair is always the identifier.
            ScenePosition* position = new ScenePosition();
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
                    position->heading = keyValue->GetValueAsFloat();
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
        //TODO: Check condition?
        for(auto& entry : section.entries)
        {
            ActorDefinition* actor = new ActorDefinition();
            IniKeyValue* keyValue = entry;
            while(keyValue != nullptr)
            {
                if(keyValue->key == "model")
                {
                    actor->model = Services::GetAssets()->LoadModel(keyValue->value + ".MOD");
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
                    //TODO: Gas file
                }
                else if(keyValue->key == "talk")
                {
                    
                }
                else if(keyValue->key == "listen")
                {
                    
                }
                else if(keyValue->key == "initAnim")
                {
                    
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
            mActorDefinitions.push_back(actor);
        }
    }
    
    // Read in models.
    std::vector<IniSection> modelSections = parser.GetSections("MODELS");
    for(auto& section : modelSections)
    {
        //TODO: Check condition?
        for(auto& entry : section.entries)
        {
            ModelDefinition* model = new ModelDefinition();
            std::string modelName;
            
            IniKeyValue* keyValue = entry;
            while(keyValue != nullptr)
            {
                if(keyValue->key == "model")
                {
                    modelName = keyValue->value;
                }
                else if(keyValue->key == "noun")
                {
                    model->noun = keyValue->value;
                }
                else if(keyValue->key == "type")
                {
                    if(keyValue->value == "scene")
                    {
                        model->type = ModelDefinition::Type::Scene;
                    }
                    else if(keyValue->value == "prop")
                    {
                        model->type = ModelDefinition::Type::Prop;
                    }
                    else if(keyValue->value == "hittest")
                    {
                        model->type = ModelDefinition::Type::HitTest;
                    }
                    else if(keyValue->value == "gasprop")
                    {
                        model->type = ModelDefinition::Type::GasProp;
                    }
                }
                else if(keyValue->key == "verb")
                {
                    model->verb = keyValue->value;
                }
                else if(keyValue->key == "initAnim")
                {
                    //TODO
                }
                else if(keyValue->key == "hidden")
                {
                    model->hidden = true;
                }
                else if(keyValue->key == "gas")
                {
                    //TODO
                }
                keyValue = keyValue->next;
            }
            
            // After parsing all the data, if this is a prop, load the model.
            // For non-props, we don't load a model - the model is baked into the BSP.
            if(!modelName.empty() &&
               (model->type == ModelDefinition::Type::Prop ||
                model->type == ModelDefinition::Type::GasProp))
            {
                model->model = Services::GetAssets()->LoadModel(modelName + ".MOD");
            }
            mModelDefinitions.push_back(model);
        }
    }
    
    // Read in regions and triggers.
    std::vector<IniSection> regionSections = parser.GetSections("REGIONS");
    for(auto& section : regionSections)
    {
        //TODO: Check condition?
        for(auto& entry : section.entries)
        {
            SceneRegionOrTrigger* region = new SceneRegionOrTrigger();
            region->label = entry->key;
            
            IniKeyValue* keyValue = entry->next;
            while(keyValue != nullptr)
            {
                if(keyValue->key == "rect")
                {
                    //TODO: read in rect.
                }
                keyValue = entry->next;
            }
            mRegions.push_back(region);
        }
    }
    
    std::vector<IniSection> triggerSections = parser.GetSections("TRIGGERS");
    for(auto& section : triggerSections)
    {
        //TODO: Check condition?
        for(auto& entry : section.entries)
        {
            SceneRegionOrTrigger* region = new SceneRegionOrTrigger();
            
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
                keyValue = entry->next;
            }
            mTriggers.push_back(region);
        }
    }
}
