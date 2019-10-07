//
// Yak.cpp
//
// Clark Kromenaker
//
#include "Yak.h"

#include "IniParser.h"
#include "Services.h"
#include "StringUtil.h"

Yak::Yak(std::string name, char* data, int dataLength) : Asset(name)
{
    ParseFromData(data, dataLength);
}

void Yak::Play(int numLines)
{
    Audio* vo = Services::GetAssets()->LoadAudio(mSounds[0]);
    Services::GetAudio()->Play(vo);
}

void Yak::ParseFromData(char *data, int dataLength)
{
    IniParser parser(data, dataLength);
    
    IniSection section;
    while(parser.ReadNextSection(section))
    {
        if(StringUtil::EqualsIgnoreCase(section.name, "HEADER"))
        {
            //int length = section.entries[0]->GetValueAsInt();
            //std::cout << "YAK Length: " << length << std::endl;
        }
        else if(StringUtil::EqualsIgnoreCase(section.name, "SOUNDS"))
        {
            int count = section.entries[0]->GetValueAsInt();
            //std::cout << "SOUNDS Count: " << count << std::endl;
            for(int i = 0; i < count; i++)
            {
                IniKeyValue* keyValue = section.entries[i + 1];
                //int frame = keyValue->GetValueAsInt();
                
                keyValue = keyValue->next;
                mSounds.push_back(keyValue->value);
                
                keyValue = keyValue->next;
                //int someVal = keyValue->GetValueAsInt();
            }
        }
        else if(StringUtil::EqualsIgnoreCase(section.name, "GK3"))
        {
            int count = section.entries[0]->GetValueAsInt();
            //std::cout << "GK3 Count: " << count << std::endl;
            for(int i = 0; i < count; i++)
            {
                IniKeyValue* keyValue = section.entries[i + 1];
                //int frame = keyValue->GetValueAsInt();
                
                keyValue = keyValue->next;
                std::string type = keyValue->value;
                if(StringUtil::EqualsIgnoreCase(type, "SPEAKER"))
                {
                    keyValue = keyValue->next;
                    std::string speakerName = keyValue->value;
                }
                else if(StringUtil::EqualsIgnoreCase(type, "CAPTION"))
                {
                    keyValue = keyValue->next;
                    std::string caption = keyValue->value;
                }
                else if(StringUtil::EqualsIgnoreCase(type, "DIALOGUECUE"))
                {
                    // Nothing else.
                }
                else if(StringUtil::EqualsIgnoreCase(type, "SPEAKERCAPTION"))
                {
                    
                }
                else if(StringUtil::EqualsIgnoreCase(type, "LIPSYNCH"))
                {
                    
                }
                else
                {
                    std::cout << "Unaccounted for YAK type: " << type << std::endl;
                }
            }
        }
        else
        {
            std::cout << "Unaccounted for YAK section: " << section.name << std::endl;
        }
    }
}
