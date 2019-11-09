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
            int count = section.lines[0].entries[0].GetValueAsInt();
            //std::cout << "SOUNDS Count: " << count << std::endl;
			
            for(int i = 0; i < count; i++)
            {
                IniKeyValue keyValue = section.lines[i + 1].entries[0];
                //int frame = keyValue->GetValueAsInt();
                
                keyValue = section.lines[i + 1].entries[1];
                mSounds.push_back(keyValue.key);
                
                keyValue = section.lines[i + 1].entries[2];
                //int someVal = keyValue->GetValueAsInt();
            }
        }
        else if(StringUtil::EqualsIgnoreCase(section.name, "GK3"))
        {
            int count = section.lines[0].entries[0].GetValueAsInt();
            //std::cout << "GK3 Count: " << count << std::endl;
			
            for(int i = 0; i < count; i++)
            {
				IniKeyValue keyValue = section.lines[i + 1].entries[0];
                //int frame = keyValue->GetValueAsInt();
                
                keyValue = section.lines[i + 1].entries[1];
                std::string type = keyValue.key;
                if(StringUtil::EqualsIgnoreCase(type, "SPEAKER"))
                {
                    keyValue = section.lines[i + 1].entries[2];
                    std::string speakerName = keyValue.key;
                }
                else if(StringUtil::EqualsIgnoreCase(type, "CAPTION"))
                {
                    keyValue = section.lines[i + 1].entries[2];
                    std::string caption = keyValue.key;
                }
                else if(StringUtil::EqualsIgnoreCase(type, "DIALOGUECUE"))
                {
                    // Nothing else.
                }
                else if(StringUtil::EqualsIgnoreCase(type, "SPEAKERCAPTION"))
                {
                    //TODO
                }
                else if(StringUtil::EqualsIgnoreCase(type, "LIPSYNCH"))
                {
                    //TODO
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
