//
// Animation.cpp
//
// Clark Kromenaker
//
#include "Animation.h"

#include "AnimationNodes.h"
#include "IniParser.h"
#include "Services.h"
#include "StringUtil.h"

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
				
				// Create and push back the animation node now.
				// The remaining fields are optional.
                VertexAnimNode* node = new VertexAnimNode();
                node->frameNumber = frameNumber;
                node->vertexAnimation = Services::GetAssets()->LoadVertexAnimation(entry->value);
                mFrames[frameNumber].push_back(node);
				
				// Next up, are (x1, y1, z1) and (angle1).
				// Not sure what these are meant for yet. Doesn't seem to be a position; that's the next set.
				if(entry->next == nullptr) { continue; }
				entry = entry->next;
				node->offsetFromOrigin.SetX(entry->GetValueAsFloat());
				
				if(entry->next == nullptr) { continue; }
				entry = entry->next;
				node->offsetFromOrigin.SetZ(entry->GetValueAsFloat());
				
				if(entry->next == nullptr) { continue; }
				entry = entry->next;
				node->offsetFromOrigin.SetY(entry->GetValueAsFloat());
				
				if(entry->next == nullptr) { continue; }
				entry = entry->next;
				node->headingFromOrigin = entry->GetValueAsFloat();
				
				// Next are (x2, z2, y2) and (angle2). Note z/y are flipped, due to Maya->Game conversion.
				// Based on examining anim files (like RC1 fountain) and expected positions,
				// this appears to be a position to move the model to at the start of the animation?
				if(entry->next == nullptr) { continue; }
				entry = entry->next;
				node->position.SetX(entry->GetValueAsFloat());
				
				if(entry->next == nullptr) { continue; }
				entry = entry->next;
				node->position.SetZ(entry->GetValueAsFloat());
				
				if(entry->next == nullptr) { continue; }
				entry = entry->next;
				node->position.SetY(entry->GetValueAsFloat());
				
				if(entry->next == nullptr) { continue; }
				entry = entry->next;
				node->heading = entry->GetValueAsFloat();
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
				
				// Create and add the anim node.
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
				
				// Create and add the anim node.
				SceneModelVisibilityAnimNode* node = new SceneModelVisibilityAnimNode();
				node->sceneName = sceneName;
				node->sceneModelName = sceneModelName;
				node->visible = visible;
                mFrames[frameNumber].push_back(node);
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
                int submeshIndex = entry->GetValueAsInt();
                
                // Read the texture name.
                if(entry->next == nullptr) { continue; }
                entry = entry->next;
                std::string textureName = entry->value;
				
				// Create and add node.
				ModelTextureAnimNode* node = new ModelTextureAnimNode();
				node->modelName = modelName;
				node->meshIndex = static_cast<unsigned char>(meshIndex);
				node->submeshIndex = static_cast<unsigned char>(submeshIndex);
				node->textureName = textureName;
				mFrames[frameNumber].push_back(node);
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
				
				// Create and add node.
				ModelVisibilityAnimNode* node = new ModelVisibilityAnimNode();
				node->modelName = modelName;
				node->visible = visible;
                mFrames[frameNumber].push_back(node);
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
				
				// Create node here - remaining entries are optional.
				SoundAnimNode* node = new SoundAnimNode();
				node->frameNumber = frameNumber;
				node->audio = Services::GetAssets()->LoadAudio(soundName);
				node->volume = volume;
				mFrames[frameNumber].push_back(node);
				
                // If there are 6 parameters, next up is the name of the model that plays the sound.
                // If there are 8 parameters, next up are a sound position (x,y,z).
                if(paramCount == 6)
                {
                    if(entry->next == nullptr) { continue; }
                    entry = entry->next;
                    std::string modelName = entry->value;
					node->modelName = modelName;
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
					
					node->position = Vector3(x, y, z);
                }
                
                // Read in min distance for sound.
                if(entry->next == nullptr) { continue; }
                entry = entry->next;
                int minDist = entry->GetValueAsInt();
                
                // Read in max distance for sound.
                if(entry->next == nullptr) { continue; }
                entry = entry->next;
                int maxDist = entry->GetValueAsInt();
				
				node->minDistance = minDist;
				node->maxDistance = maxDist;
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
                //int frameNumber = entry->GetValueAsInt();
                
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
                    //mFramesPerSecond = entry->GetValueAsInt();
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
                    std::string actorNoun = entry->value;
					
					// Create and add node.
					FootstepAnimNode* node = new FootstepAnimNode();
					node->actorNoun = actorNoun;
					mFrames[frameNumber].push_back(node);
                }
                else if(StringUtil::EqualsIgnoreCase(keyword, "FOOTSCUFF"))
                {
                    if(entry->next == nullptr) { continue; }
                    entry = entry->next;
                    std::string actorNoun = entry->value;
					
					// Create and add node.
					FootscuffAnimNode* node = new FootscuffAnimNode();
					node->actorNoun = actorNoun;
					mFrames[frameNumber].push_back(node);
                }
                else if(StringUtil::EqualsIgnoreCase(keyword, "STOPSOUNDTRACK"))
                {
                    if(entry->next == nullptr) { continue; }
                    entry = entry->next;
                    std::string soundtrackName = entry->value;
					
					// Create and add node.
					StopSoundtrackAnimNode* node = new StopSoundtrackAnimNode();
					node->soundtrackName = soundtrackName;
					mFrames[frameNumber].push_back(node);
                }
                else if(StringUtil::EqualsIgnoreCase(keyword, "PLAYSOUNDTRACK"))
                {
                    if(entry->next == nullptr) { continue; }
                    entry = entry->next;
                    std::string soundtrackName = entry->value;
					
					// Create and add node.
					PlaySoundtrackAnimNode* node = new PlaySoundtrackAnimNode();
					node->soundtrackName = soundtrackName;
					mFrames[frameNumber].push_back(node);
                }
                else if(StringUtil::EqualsIgnoreCase(keyword, "PLAYSOUNDTRACKTBS"))
                {
                    if(entry->next == nullptr) { continue; }
                    entry = entry->next;
                    std::string soundtrackName = entry->value;
					
					// Create and add node.
					PlaySoundtrackAnimNode* node = new PlaySoundtrackAnimNode();
					node->soundtrackName = soundtrackName;
					mFrames[frameNumber].push_back(node);
                }
                else if(StringUtil::EqualsIgnoreCase(keyword, "STOPALLSOUNDTRACKS"))
                {
					// Create and add node.
					mFrames[frameNumber].push_back(new StopSoundtrackAnimNode());
                }
                else if(StringUtil::EqualsIgnoreCase(keyword, "CAMERA"))
                {
                    if(entry->next == nullptr) { continue; }
                    entry = entry->next;
                    std::string cameraPositionName = entry->value;
					
					// Create and add node.
					CameraAnimNode* node = new CameraAnimNode();
					node->cameraPositionName = cameraPositionName;
					mFrames[frameNumber].push_back(node);
                }
                else if(StringUtil::EqualsIgnoreCase(keyword, "LIPSYNCH"))
                {
                    if(entry->next == nullptr) { continue; }
                    entry = entry->next;
                    std::string actorNoun = entry->value;
                    
                    if(entry->next == nullptr) { continue; }
                    entry = entry->next;
                    std::string mouthTexName = entry->value;
					
					// Create and add node.
					LipSyncAnimNode* node = new LipSyncAnimNode();
					node->actorNoun = actorNoun;
					node->mouthTextureName = mouthTexName;
					mFrames[frameNumber].push_back(node);
                }
                else if(StringUtil::EqualsIgnoreCase(keyword, "FACETEX"))
                {
                    // Read the actor name.
                    if(entry->next == nullptr) { continue; }
                    entry = entry->next;
                    std::string actorNoun = entry->value;
                    
                    // Read texture name.
                    // This sometimes has a forward slash in it, which
                    // indicates "tex"/"alpha_tex".
                    if(entry->next == nullptr) { continue; }
                    entry = entry->next;
                    std::string textureName = entry->value;
                    
                    // A value indicating what part of the face is changed. Always H, E, M.
					// M = mouth
					// E = eye
					// H = head/forehead
                    if(entry->next == nullptr) { continue; }
                    entry = entry->next;
					StringUtil::ToLower(entry->value);
					FaceElement faceElement = FaceElement::Mouth;
					switch(entry->value[0])
					{
						case 'm':
							faceElement = FaceElement::Mouth;
							break;
						case 'e':
							faceElement = FaceElement::Eyelids;
							break;
						case 'h':
							faceElement = FaceElement::Forehead;
							break;
					}
					
					// Create and add node.
					FaceTexAnimNode* node = new FaceTexAnimNode();
					node->actorNoun = actorNoun;
					node->textureName = textureName;
					node->faceElement = faceElement;
					mFrames[frameNumber].push_back(node);
                }
                else if(StringUtil::EqualsIgnoreCase(keyword, "UNFACETEX"))
                {
                    // Read the actor name.
                    if(entry->next == nullptr) { continue; }
                    entry = entry->next;
                    std::string actorNoun = entry->value;
					
					// A value indicating what part of the face is changed. Always H, E, M.
					// M = mouth
					// E = eye
					// H = head/forehead
                    if(entry->next == nullptr) { continue; }
                    entry = entry->next;
					StringUtil::ToLower(entry->value);
					FaceElement faceElement = FaceElement::Mouth;
					switch(entry->value[0])
					{
						case 'm':
							faceElement = FaceElement::Mouth;
							break;
						case 'e':
							faceElement = FaceElement::Eyelids;
							break;
						case 'h':
							faceElement = FaceElement::Forehead;
							break;
					}
					
					// Create and add node.
					UnFaceTexAnimNode* node = new UnFaceTexAnimNode();
					node->actorNoun = actorNoun;
					node->faceElement = faceElement;
					mFrames[frameNumber].push_back(node);
                }
                else if(StringUtil::EqualsIgnoreCase(keyword, "GLANCE"))
                {
                    // Read the actor name.
                    if(entry->next == nullptr) { continue; }
                    entry = entry->next;
                    std::string actorNoun = entry->value;
                    
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
					
					// Create and add node.
					GlanceAnimNode* node = new GlanceAnimNode();
					node->actorNoun = actorNoun;
					node->position = Vector3(num1, num2, num3);
					mFrames[frameNumber].push_back(node);
                }
				else if(StringUtil::EqualsIgnoreCase(keyword, "MOOD"))
				{
					// Read the actor name.
					if(entry->next == nullptr) { continue; }
					entry = entry->next;
					std::string actorNoun = entry->value;
					
					// Read the mood name.
					if(entry->next == nullptr) { continue; }
					entry = entry->next;
					std::string moodName = entry->value;
					
					// Create and add node.
					MoodAnimNode* node = new MoodAnimNode();
					node->actorNoun = actorNoun;
					node->moodName = moodName;
					mFrames[frameNumber].push_back(node);
				}
                else
                {
                    std::cout << "Unexpected GK3 animation keyword: " << keyword << std::endl;
                }
            }
        }
        else
        {
            std::cout << "Unexpected animation header: " << section.name << std::endl;
        }
    }
}
