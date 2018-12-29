//
// Animation.cpp
//
// Clark Kromenaker
//
#include "Animation.h"

#include "GKActor.h"
#include "IniParser.h"
#include "Services.h"
#include "Scene.h"
#include "StringUtil.h"

void VertexAnimNode::Play(Animation* anim)
{
	if(vertexAnimation != nullptr)
	{
		GKActor* actor = GEngine::inst->GetScene()->GetActorByModelName(vertexAnimation->GetModelName());
		if(actor != nullptr)
		{
			if(anim != nullptr)
			{
				actor->PlayAnimation(vertexAnimation, anim->GetFramesPerSecond());
			}
			else
			{
				actor->PlayAnimation(vertexAnimation);
			}
		}
	}
}

void VertexAnimNode::Sample(Animation* anim, int frame)
{
	if(vertexAnimation != nullptr)
	{
		GKActor* actor = GEngine::inst->GetScene()->GetActorByModelName(vertexAnimation->GetModelName());
		if(actor != nullptr)
		{
			actor->SampleAnimation(vertexAnimation, frame);
		}
	}
}

void SceneTextureAnimNode::Play(Animation* anim)
{
	Texture* texture = Services::GetAssets()->LoadTexture(textureName);
	if(texture != nullptr)
	{
		//TODO: Ensure sceneName matches loaded scene name?
		GEngine::inst->GetScene()->ApplyTextureToSceneModel(sceneModelName, texture);
	}
}

Animation::Animation(std::string name, char* data, int dataLength) : Asset(name)
{
    ParseFromData(data, dataLength);
}

Animation::~Animation()
{
	
}

std::vector<AnimNode*>* Animation::GetFrame(int num)
{
    if(mFrames.find(num) != mFrames.end())
    {
        return &mFrames[num];
    }
    return nullptr;
}

void Animation::ParseFromData(char *data, int dataLength)
{
    IniParser parser(data, dataLength);
    IniSection section;
    while(parser.ReadNextSection(section))
    {
        // Header section has only one value: number of frames.
        if(StringUtil::EqualsIgnoreCase(section.name, "HEADER"))
        {
            if(section.entries.size() > 0)
            {
                mFrameCount = section.entries[0]->GetValueAsInt();
            }
        }
		// Actions section contains vertex animations to start playing on certain frames.
        else if(StringUtil::EqualsIgnoreCase(section.name, "ACTIONS"))
        {
            for(int i = 1; i < section.entries.size(); i++)
            {
                IniKeyValue* entry = section.entries[i];
                
                // Each entry has up to 10 (!!!) fields.
                // <frame_num>, <act_name>, <x1>, <y1>, <z1>, <angle1>, <x2>, <y2>, <z2>, <angle2>
                int frameNumber = entry->GetValueAsInt();
                
                // Load vertex animation with the name.
                if(entry->next == nullptr) { continue; }
                entry = entry->next;
                
                VertexAnimNode* node = new VertexAnimNode();
                node->frameNumber = frameNumber;
                node->vertexAnimation = Services::GetAssets()->LoadVertexAnimation(entry->value);
                mFrames[frameNumber].push_back(node);
                
                //TODO: Come back to do additional parsing here.
            }
        }
		// "STextures" changes textures on a scene (BSP) model.
        else if(StringUtil::EqualsIgnoreCase(section.name, "STEXTURES"))
        {
            for(int i = 1; i < section.entries.size(); i++)
            {
                IniKeyValue* entry = section.entries[i];
                
                // <frame_num>, <scn_name>, <scn_model_name>, <texture_name>
                int frameNumber = entry->GetValueAsInt();
                
                // Read the scene name.
                if(entry->next == nullptr) { continue; }
                entry = entry->next;
                std::string sceneName = entry->value;
                
                // Read the scene model name.
                if(entry->next == nullptr) { continue; }
                entry = entry->next;
                std::string sceneModelName = entry->value;
                
                // Read the texture name.
                if(entry->next == nullptr) { continue; }
                entry = entry->next;
                std::string textureName = entry->value;
				
				SceneTextureAnimNode* node = new SceneTextureAnimNode();
				node->frameNumber = frameNumber;
				node->sceneName = sceneName;
				node->sceneModelName = sceneModelName;
				node->textureName = textureName;
				
                mFrames[frameNumber].push_back(node);
            }
        }
		// "SVisibility" changes the visibility of a scene (BSP) model.
        else if(StringUtil::EqualsIgnoreCase(section.name, "SVISIBILITY"))
        {
            for(int i = 1; i < section.entries.size(); i++)
            {
                IniKeyValue* entry = section.entries[i];
                
                // <frame_num>, <scn_name>, <scn_model_name>, <on/off>
                int frameNumber = entry->GetValueAsInt();
                
                // Read the scene name.
                if(entry->next == nullptr) { continue; }
                entry = entry->next;
                std::string sceneName = entry->value;
                
                // Read the scene model name.
                if(entry->next == nullptr) { continue; }
                entry = entry->next;
                std::string sceneModelName = entry->value;
                
                // Read the on/off value.
                if(entry->next == nullptr) { continue; }
                entry = entry->next;
                bool visible = entry->GetValueAsBool();
				
				std::cout << "SVISIBILITY" << std::endl;
                //mFrames[frameNumber].push_back(node);
            }
        }
		// "MTextures" changes textures on a model or actor.
        else if(StringUtil::EqualsIgnoreCase(section.name, "MTEXTURES"))
        {
            for(int i = 1; i < section.entries.size(); i++)
            {
                IniKeyValue* entry = section.entries[i];
                
                // <frame_num>, <model_name>, <mesh_index>, <group_index>, <texture_name>
                int frameNumber = entry->GetValueAsInt();
                
                // Read the model name.
                if(entry->next == nullptr) { continue; }
                entry = entry->next;
                std::string modelName = entry->value;
                
                // Read the model mesh index.
                if(entry->next == nullptr) { continue; }
                entry = entry->next;
                int meshIndex = entry->GetValueAsInt();
                
                // Read the model mesh group index.
                if(entry->next == nullptr) { continue; }
                entry = entry->next;
                int groupIndex = entry->GetValueAsInt();
                
                // Read the texture name.
                if(entry->next == nullptr) { continue; }
                entry = entry->next;
                std::string textureName = entry->value;
				
				std::cout << "MTEXTURE " << modelName << ", " << meshIndex << ", " << groupIndex << ", " << textureName << std::endl;
                //mFrames[frameNumber].push_back(node);
            }
        }
		// "MVisibility" changes visibility on a model or actor.
        else if(StringUtil::EqualsIgnoreCase(section.name, "MVISIBILITY"))
        {
            for(int i = 1; i < section.entries.size(); i++)
            {
                IniKeyValue* entry = section.entries[i];
                
                // <frame_num>, <model_name>, <on/off>
                int frameNumber = entry->GetValueAsInt();
                
                // Read the model name.
                if(entry->next == nullptr) { continue; }
                entry = entry->next;
                std::string modelName = entry->value;
                
                // Read the on/off value.
                if(entry->next == nullptr) { continue; }
                entry = entry->next;
                bool visible = entry->GetValueAsBool();
				
				std::cout << "MVISIBILITY " << modelName << ", " << visible << std::endl;
                //mFrames[frameNumber].push_back(node);
            }
        }
		// Triggers sounds to play on certain frames at certain locations.
        else if(StringUtil::EqualsIgnoreCase(section.name, "SOUNDS"))
        {
            for(int i = 1; i < section.entries.size(); i++)
            {
                IniKeyValue* entry = section.entries[i];
                
                // <frame_num>, <sound_name>, <volume>, <model_name>, <min_dist>, <max_dist>
                // <frame_num>, <sound_name>, <volume>, <x1>, <y1>, <z1>, <min_dist>, <max_dist>
                unsigned int paramCount = 1;
                IniKeyValue* countEntry = entry;
                while(countEntry == nullptr)
                {
                    paramCount++;
                    countEntry = countEntry->next;
                }
                
                // Read frame number.
                int frameNumber = entry->GetValueAsInt();
                
                // Read the sound name.
                if(entry->next == nullptr) { continue; }
                entry = entry->next;
                std::string soundName = entry->value;
                
                // Read the volume.
                if(entry->next == nullptr) { continue; }
                entry = entry->next;
                int volume = entry->GetValueAsInt();
                
                // If there are 6 parameters, next up is the name of the model that plays the sound.
                // If there are 8 parameters, next up are a sound position (x,y,z).
                if(paramCount == 6)
                {
                    if(entry->next == nullptr) { continue; }
                    entry = entry->next;
                    std::string modelName = entry->value;
                }
                else
                {
                    if(entry->next == nullptr) { continue; }
                    entry = entry->next;
                    int x = entry->GetValueAsInt();
                    
                    if(entry->next == nullptr) { continue; }
                    entry = entry->next;
                    int y = entry->GetValueAsInt();
                    
                    if(entry->next == nullptr) { continue; }
                    entry = entry->next;
                    int z = entry->GetValueAsInt();
                }
                
                // Read in min distance for sound.
                if(entry->next == nullptr) { continue; }
                entry = entry->next;
                int minDist = entry->GetValueAsInt();
                
                // Read in max distance for sound.
                if(entry->next == nullptr) { continue; }
                entry = entry->next;
                int maxDist = entry->GetValueAsInt();
				
				std::cout << "SOUND " << soundName << std::endl;
                //mFrames[frameNumber].push_back(node);
            }
        }
		// Allows specifying of additional options that affect the entire animation.
        else if(StringUtil::EqualsIgnoreCase(section.name, "OPTIONS"))
        {
            for(int i = 1; i < section.entries.size(); i++)
            {
                // <frame_num>, <option>, <value>
                IniKeyValue* entry = section.entries[i];
                
                // Read frame number.
                int frameNumber = entry->GetValueAsInt();
                
                // Read in the option.
                if(entry->next == nullptr) { continue; }
                entry = entry->next;
                std::string option = entry->value;
                
                if(StringUtil::EqualsIgnoreCase(option, "SIMPLE"))
                {
                    if(entry->next == nullptr) { continue; }
                    entry = entry->next;
                    //int simpleValue = entry->GetValueAsInt();
                }
                else if(StringUtil::EqualsIgnoreCase(option, "NOINTERPOLATE"))
                {
                    
                }
				// Allows us to dictate the rate at which the animation proceeds.
                else if(StringUtil::EqualsIgnoreCase(option, "FRAMERATE"))
                {
                    if(entry->next == nullptr) { continue; }
                    entry = entry->next;
                    mFramesPerSecond = entry->GetValueAsInt();
                }
                else
                {
                    std::cout << "Unexpected option: " << option << std::endl;
                }
            }
        }
		// Contains some options specifically for GK3. I imagine this engine
		// was initially meant to be reused on future projects, so they tried
		// to isolate extremely specific to GK3 stuff here?
        else if(StringUtil::EqualsIgnoreCase(section.name, "GK3"))
        {
            for(int i = 1; i < section.entries.size(); i++)
            {
                IniKeyValue* entry = section.entries[i];
                
                // Read frame number.
                int frameNumber = entry->GetValueAsInt();
                
                // Read in the option.
                if(entry->next == nullptr) { continue; }
                entry = entry->next;
                std::string keyword = entry->value;
                
                if(StringUtil::EqualsIgnoreCase(keyword, "FOOTSTEP"))
                {
                    if(entry->next == nullptr) { continue; }
                    entry = entry->next;
                    std::string actorName = entry->value;
                }
                else if(StringUtil::EqualsIgnoreCase(keyword, "FOOTSCUFF"))
                {
                    if(entry->next == nullptr) { continue; }
                    entry = entry->next;
                    std::string actorName = entry->value;
                }
                else if(StringUtil::EqualsIgnoreCase(keyword, "STOPSOUNDTRACK"))
                {
                    if(entry->next == nullptr) { continue; }
                    entry = entry->next;
                    std::string soundtrackName = entry->value;
                }
                else if(StringUtil::EqualsIgnoreCase(keyword, "PLAYSOUNDTRACK"))
                {
                    if(entry->next == nullptr) { continue; }
                    entry = entry->next;
                    std::string soundtrackName = entry->value;
                }
                else if(StringUtil::EqualsIgnoreCase(keyword, "PLAYSOUNDTRACKTBS"))
                {
                    if(entry->next == nullptr) { continue; }
                    entry = entry->next;
                    std::string soundtrackName = entry->value;
                }
                else if(StringUtil::EqualsIgnoreCase(keyword, "STOPALLSOUNDTRACKS"))
                {
                    
                }
                else if(StringUtil::EqualsIgnoreCase(keyword, "CAMERA"))
                {
                    if(entry->next == nullptr) { continue; }
                    entry = entry->next;
                    std::string cameraName = entry->value;
                }
                else if(StringUtil::EqualsIgnoreCase(keyword, "LIPSYNCH"))
                {
                    if(entry->next == nullptr) { continue; }
                    entry = entry->next;
                    std::string actorName = entry->value;
                    
                    if(entry->next == nullptr) { continue; }
                    entry = entry->next;
                    std::string mouthTexName = entry->value;
                }
                else if(StringUtil::EqualsIgnoreCase(keyword, "FACETEX"))
                {
                    // Read the actor name.
                    if(entry->next == nullptr) { continue; }
                    entry = entry->next;
                    std::string actorName = entry->value;
                    
                    // Read texture name.
                    // This sometimes has a forward slash in it, which
                    // indicates "tex"/"alpha_tex".
                    if(entry->next == nullptr) { continue; }
                    entry = entry->next;
                    std::string textureName = entry->value;
                    
                    // A value indicating what part of the face is changed. Always H, E, M.
					// M = mouth
					// E = eye
					// H = eyebrow/forehead (H = head?)
                    if(entry->next == nullptr) { continue; }
                    entry = entry->next;
                    std::string someValue = entry->value;
                }
                else if(StringUtil::EqualsIgnoreCase(keyword, "UNFACETEX"))
                {
                    // Read the actor name.
                    if(entry->next == nullptr) { continue; }
                    entry = entry->next;
                    std::string actorName = entry->value;
                    
                    // Not sure what this is? Always H, E, M.
                    if(entry->next == nullptr) { continue; }
                    entry = entry->next;
                    std::string someValue = entry->value;
                }
                else if(StringUtil::EqualsIgnoreCase(keyword, "GLANCE"))
                {
                    // Read the actor name.
                    if(entry->next == nullptr) { continue; }
                    entry = entry->next;
                    std::string actorName = entry->value;
                    
                    // Unknown value
                    if(entry->next == nullptr) { continue; }
                    entry = entry->next;
                    int num1 = entry->GetValueAsInt();
                    
                    // Unknown value
                    if(entry->next == nullptr) { continue; }
                    entry = entry->next;
                    int num2 = entry->GetValueAsInt();
                    
                    // Unknown value
                    if(entry->next == nullptr) { continue; }
                    entry = entry->next;
                    int num3 = entry->GetValueAsInt();
                }
				else if(StringUtil::EqualsIgnoreCase(keyword, "MOOD"))
				{
					// Seems to be followed by two keywords:
					// 1) Actor to affect
					// 2) Mood to apply
					/*
					while(entry != nullptr)
					{
						std::cout << entry->value << std::endl;
						entry = entry->next;
					}
					*/
				}
                else
                {
                    std::cout << "Unexpected keyword: " << keyword << std::endl;
                }
            }
        }
        else
        {
            std::cout << "Unexpected animation header: " << section.name << std::endl;
        }
    }
}
