//
// Animation.cpp
//
// Clark Kromenaker
//
#include "Animation.h"
#include "IniParser.h"
#include "StringUtil.h"
#include "Services.h"

Animation::Animation(std::string name, char* data, int dataLength) : Asset(name)
{
    ParseFromData(data, dataLength);
}

std::vector<AnimationNode*>* Animation::GetFrame(int num)
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
                
                AnimationNode* node = new AnimationNode();
                node->mFrameNumber = frameNumber;
                node->mVertexAnimation = Services::GetAssets()->LoadVertexAnimation(entry->value);
                mFrames[frameNumber].push_back(node);
                
                //TODO: Come back to do additional parsing here.
            }
        }
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
                
                //mFrames[frameNumber].push_back(node);
            }
        }
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
                
                //mFrames[frameNumber].push_back(node);
            }
        }
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
                
                //mFrames[frameNumber].push_back(node);
            }
        }
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
                
                //mFrames[frameNumber].push_back(node);
            }
        }
        else if(StringUtil::EqualsIgnoreCase(section.name, "SOUNDS"))
        {
            for(int i = 1; i < section.entries.size(); i++)
            {
                IniKeyValue* entry = section.entries[i];
                
                // <frame_num>, <sound_name>, <volume>, <model_name>, <min_dist>, <max_dist>
                // <frame_num>, <sound_name>, <volume>, <x1>, <y1>, <z1>, <min_dist>, <max_dist>
                uint paramCount = 1;
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
                
                //mFrames[frameNumber].push_back(node);
            }
        }
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
                    int simpleValue = entry->GetValueAsInt();
                }
                else if(StringUtil::EqualsIgnoreCase(option, "NOINTERPOLATE"))
                {
                    
                }
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
                    
                    // Not sure what this is? Always H, E, M.
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
