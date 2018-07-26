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
                
                if(entry->next == nullptr) { continue; }
                entry = entry->next;
                AnimationNode* node = new AnimationNode();
                node->action = Services::GetAssets()->LoadACT(entry->value);
                
                mFrames[frameNumber].push_back(node);
                //TODO: Come back to do additional parsing here.
            }
        }
        else if(StringUtil::EqualsIgnoreCase(section.name, "STEXTURES"))
        {
            
        }
        else if(StringUtil::EqualsIgnoreCase(section.name, "SVISIBILITY"))
        {
            
        }
        else if(StringUtil::EqualsIgnoreCase(section.name, "MTEXTURES"))
        {
            
        }
        else if(StringUtil::EqualsIgnoreCase(section.name, "MVISIBILITY"))
        {
            
        }
        else if(StringUtil::EqualsIgnoreCase(section.name, "SOUNDS"))
        {
            
        }
        else if(StringUtil::EqualsIgnoreCase(section.name, "OPTIONS"))
        {
            
        }
        else if(StringUtil::EqualsIgnoreCase(section.name, "GK3"))
        {
            
        }
        else
        {
            std::cout << "Unexpected animation header: " << section.name << std::endl;
        }
    }
}
