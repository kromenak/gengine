#include "CharacterManager.h"

#include "IniParser.h"
#include "Loader.h"
#include "Services.h"
#include "StringUtil.h"

TYPE_DEF_BASE(CharacterManager);

CharacterManager::CharacterManager()
{
    Loader::Load([this]() {
        // Read in info about characters.
        {
            // Get CHARACTERS text file as a raw buffer.
            unsigned int bufferSize = 0;
            char* buffer = Services::GetAssets()->LoadRaw("CHARACTERS.TXT", bufferSize);

            // Pass that along to INI parser, since it is plain text and in INI format.
            IniParser parser(buffer, bufferSize);

            // Read one section at a time.
            // Each section correlates to one character.
            // The section name is the character's three-letter code (GAB, ABE, GRA, etc).
            IniSection section;
            while(parser.ReadNextSection(section))
            {
                CharacterConfig config;
                config.identifier = section.name;

                // Each entry in a section is some property about the character.
                for(auto& line : section.lines)
                {
                    IniKeyValue& entry = line.entries.front();
                    if(StringUtil::EqualsIgnoreCase(entry.key, "WalkerHeight"))
                    {
                        config.walkerHeight = entry.GetValueAsFloat();
                    }
                    else if(StringUtil::EqualsIgnoreCase(entry.key, "ShoeThickness"))
                    {
                        config.shoeThickness = entry.GetValueAsFloat();
                    }
                    else if(StringUtil::EqualsIgnoreCase(entry.key, "StartAnim"))
                    {
                        config.walkStartAnim = Services::GetAssets()->LoadAnimation(entry.value);
                    }
                    else if(StringUtil::EqualsIgnoreCase(entry.key, "ContAnim"))
                    {
                        config.walkLoopAnim = Services::GetAssets()->LoadAnimation(entry.value);
                    }
                    else if(StringUtil::EqualsIgnoreCase(entry.key, "StopAnim"))
                    {
                        config.walkStopAnim = Services::GetAssets()->LoadAnimation(entry.value);
                    }
                    else if(StringUtil::EqualsIgnoreCase(entry.key, "StartTurnRightAnim"))
                    {
                        config.walkStartTurnRightAnim = Services::GetAssets()->LoadAnimation(entry.value);
                    }
                    else if(StringUtil::EqualsIgnoreCase(entry.key, "StartTurnLeftAnim"))
                    {
                        config.walkStartTurnLeftAnim = Services::GetAssets()->LoadAnimation(entry.value);
                    }
                    else if(StringUtil::EqualsIgnoreCase(entry.key, "HipAxesMeshIndex"))
                    {
                        config.hipAxesMeshIndex = entry.GetValueAsInt();
                    }
                    else if(StringUtil::EqualsIgnoreCase(entry.key, "HipAxesGroupIndex"))
                    {
                        config.hipAxesGroupIndex = entry.GetValueAsInt();
                    }
                    else if(StringUtil::EqualsIgnoreCase(entry.key, "HipAxesPointIndex"))
                    {
                        config.hipAxesPointIndex = entry.GetValueAsInt();
                    }
                    else if(StringUtil::EqualsIgnoreCase(entry.key, "LShoeAxisMeshIndex"))
                    {
                        config.leftShoeAxesMeshIndex = entry.GetValueAsInt();
                    }
                    else if(StringUtil::EqualsIgnoreCase(entry.key, "LShoeAxesGroupIndex"))
                    {
                        config.leftShoeAxesGroupIndex = entry.GetValueAsInt();
                    }
                    else if(StringUtil::EqualsIgnoreCase(entry.key, "LShoeAxesPointIndex"))
                    {
                        config.leftShoeAxesPointIndex = entry.GetValueAsInt();
                    }
                    else if(StringUtil::EqualsIgnoreCase(entry.key, "RShoeAxisMeshIndex"))
                    {
                        config.rightShoeAxesMeshIndex = entry.GetValueAsInt();
                    }
                    else if(StringUtil::EqualsIgnoreCase(entry.key, "RShoeAxesGroupIndex"))
                    {
                        config.rightShoeAxesGroupIndex = entry.GetValueAsInt();
                    }
                    else if(StringUtil::EqualsIgnoreCase(entry.key, "RShoeAxesPointIndex"))
                    {
                        config.rightShoeAxesPointIndex = entry.GetValueAsInt();
                    }
                    else if(StringUtil::StartsWithIgnoreCase(entry.key, "Clothes"))
                    {
                        // Get portion of key after "Clothes" - this indicates WHEN this clothing anim applies (e.g. day 1 10am).
                        std::string timeblockStr(entry.key, 7);

                        // Find an empty slot for this clothes anim. If no empty slot, this entry is discarded.
                        for(int i = 0; i < CharacterConfig::kMaxClothesAnims; ++i)
                        {
                            if(config.clothesAnims[i].first.empty())
                            {
                                config.clothesAnims[i].first = timeblockStr;
                                config.clothesAnims[i].second = Services::GetAssets()->LoadAnimation(entry.value);
                                break;
                            }
                        }
                    }
                }

                // Key each config by its identifier.
                mCharacterConfigs[config.identifier] = config;
            }
            delete[] buffer;
        }

        // Read in info about character faces.
        {
            // Get FACES text file as a raw buffer.
            unsigned int facesBufferSize = 0;
            char* facesBuffer = Services::GetAssets()->LoadRaw("FACES.TXT", facesBufferSize);

            // Pass that along to INI parser, since it is plain text and in INI format.
            IniParser facesParser(facesBuffer, facesBufferSize);
            facesParser.SetMultipleKeyValuePairsPerLine(false); // Stops splitting on commas.

            // Read in each section of the FACES file.
            IniSection section;
            while(facesParser.ReadNextSection(section))
            {
                // Default section contains some values to use if not defined for a particular character.
                if(StringUtil::EqualsIgnoreCase(section.name, "Default"))
                {
                    // Some variables to hold defaults.
                    Vector2 maxEyeLookDistanceDefault;
                    Vector2 eyeJitterFrequencyDefault;
                    Vector2 maxEyeJitterDistanceDefault;
                    Vector2 eyeFieldOfViewDefault;
                    Vector2 eyeShortFieldOfViewDefault;
                    float eyeSeparationDefault = 0.0f;
                    float headRadiusDefault = 0.0f;
                    Vector3 headCenterOffsetDefault;
                    Vector2 blinkFrequencyDefault;

                    // Read in default values.
                    for(auto& line : section.lines)
                    {
                        IniKeyValue& entry = line.entries.front();
                        if(StringUtil::EqualsIgnoreCase(entry.key, "Max Look Distance"))
                        {
                            maxEyeLookDistanceDefault = entry.GetValueAsVector2();
                        }
                        else if(StringUtil::EqualsIgnoreCase(entry.key, "Jitter Frequency"))
                        {
                            eyeJitterFrequencyDefault = entry.GetValueAsVector2();
                        }
                        else if(StringUtil::EqualsIgnoreCase(entry.key, "Max Jitter Distance"))
                        {
                            maxEyeJitterDistanceDefault = entry.GetValueAsVector2();
                        }
                        else if(StringUtil::EqualsIgnoreCase(entry.key, "Eye Field of View"))
                        {
                            eyeFieldOfViewDefault = entry.GetValueAsVector2();
                        }
                        else if(StringUtil::EqualsIgnoreCase(entry.key, "Eye Short Field of View"))
                        {
                            eyeShortFieldOfViewDefault = entry.GetValueAsVector2();
                        }
                        else if(StringUtil::EqualsIgnoreCase(entry.key, "Eye Separation"))
                        {
                            eyeSeparationDefault = entry.GetValueAsFloat();
                        }
                        else if(StringUtil::EqualsIgnoreCase(entry.key, "Head Radius"))
                        {
                            headRadiusDefault = entry.GetValueAsFloat();
                        }
                        else if(StringUtil::EqualsIgnoreCase(entry.key, "Head Center Offset"))
                        {
                            headCenterOffsetDefault = entry.GetValueAsVector3();
                        }
                        else if(StringUtil::EqualsIgnoreCase(entry.key, "Blink Frequency"))
                        {
                            blinkFrequencyDefault = entry.GetValueAsVector2();
                        }
                    }

                    // Apply defaults to all character face configs.
                    for(auto& pair : mCharacterConfigs)
                    {
                        CharacterConfig& config = pair.second;
                        config.faceConfig.maxEyeLookDistance = maxEyeLookDistanceDefault;
                        config.faceConfig.eyeJitterFrequency = eyeJitterFrequencyDefault;
                        config.faceConfig.maxEyeJitterDistance = maxEyeJitterDistanceDefault;
                        config.faceConfig.eyeFieldOfView = eyeFieldOfViewDefault;
                        config.faceConfig.eyeShortFieldOfView = eyeShortFieldOfViewDefault;
                        config.faceConfig.eyeSeparation = eyeSeparationDefault;
                        config.faceConfig.headRadius = headRadiusDefault;
                        config.faceConfig.headCenterOffset = headCenterOffsetDefault;
                        config.faceConfig.blinkFrequency = blinkFrequencyDefault;
                    }
                }
                else if(StringUtil::EqualsIgnoreCase(section.name, "Eyes"))
                {
                    // Eyes section contains eye definitions.
                    // But they are all the same...<bitmap> = 4x4, DownSampleOnly
                    // No need to read in.
                }
                else
                {
                    // If the section name matches a character identifier, this section is the face info for that character!
                    auto it = mCharacterConfigs.find(section.name);
                    if(it != mCharacterConfigs.end())
                    {
                        // First, try to load the entry's face/eyelid/forehead textures.
                        // These are derived from the section name.
                        CharacterConfig& config = it->second;
                        config.faceConfig.faceTexture = Services::GetAssets()->LoadSceneTexture(section.name + "_face");
                        config.faceConfig.eyelidsTexture = Services::GetAssets()->LoadTexture(section.name + "_eyelids");
                        config.faceConfig.foreheadTexture = Services::GetAssets()->LoadTexture(section.name + "_forehead");

                        // Each entry is a face property for the character.
                        for(auto& line : section.lines)
                        {
                            IniKeyValue& entry = line.entries.front();
                            if(StringUtil::EqualsIgnoreCase(entry.key, "Left Eye Name"))
                            {
                                config.faceConfig.leftEyeTexture = Services::GetAssets()->LoadTexture(entry.value);
                            }
                            else if(StringUtil::EqualsIgnoreCase(entry.key, "Right Eye Name"))
                            {
                                config.faceConfig.rightEyeTexture = Services::GetAssets()->LoadTexture(entry.value);
                            }
                            else if(StringUtil::EqualsIgnoreCase(entry.key, "Left Eye Offset"))
                            {
                                config.faceConfig.leftEyeOffset = entry.GetValueAsVector2();
                            }
                            else if(StringUtil::EqualsIgnoreCase(entry.key, "Right Eye Offset"))
                            {
                                config.faceConfig.rightEyeOffset = entry.GetValueAsVector2();
                            }
                            else if(StringUtil::EqualsIgnoreCase(entry.key, "Left Eye Bias"))
                            {
                                config.faceConfig.leftEyeBias = entry.GetValueAsVector2();
                            }
                            else if(StringUtil::EqualsIgnoreCase(entry.key, "Right Eye Bias"))
                            {
                                config.faceConfig.rightEyeBias = entry.GetValueAsVector2();
                            }
                            else if(StringUtil::EqualsIgnoreCase(entry.key, "Max Look Distance"))
                            {
                                config.faceConfig.maxEyeLookDistance = entry.GetValueAsVector2();
                            }
                            else if(StringUtil::EqualsIgnoreCase(entry.key, "Jitter Frequency"))
                            {
                                config.faceConfig.eyeJitterFrequency = entry.GetValueAsVector2();
                            }
                            else if(StringUtil::EqualsIgnoreCase(entry.key, "Max Jitter Distance"))
                            {
                                config.faceConfig.maxEyeLookDistance = entry.GetValueAsVector2();
                            }
                            else if(StringUtil::EqualsIgnoreCase(entry.key, "Eye Field of View"))
                            {
                                config.faceConfig.eyeFieldOfView = entry.GetValueAsVector2();
                            }
                            else if(StringUtil::EqualsIgnoreCase(entry.key, "Eye Short Field of View"))
                            {
                                config.faceConfig.eyeShortFieldOfView = entry.GetValueAsVector2();
                            }
                            else if(StringUtil::EqualsIgnoreCase(entry.key, "Eye Separation"))
                            {
                                config.faceConfig.eyeSeparation = entry.GetValueAsFloat();
                            }
                            else if(StringUtil::EqualsIgnoreCase(entry.key, "Head Radius"))
                            {
                                config.faceConfig.headRadius = entry.GetValueAsFloat();
                            }
                            else if(StringUtil::EqualsIgnoreCase(entry.key, "Head Center Offset"))
                            {
                                config.faceConfig.headCenterOffset = entry.GetValueAsVector3();
                            }
                            else if(StringUtil::EqualsIgnoreCase(entry.key, "Forehead Offset"))
                            {
                                config.faceConfig.foreheadOffset = entry.GetValueAsVector2();
                            }
                            else if(StringUtil::EqualsIgnoreCase(entry.key, "Eyelids Offset"))
                            {
                                config.faceConfig.eyelidsOffset = entry.GetValueAsVector2();
                            }
                            else if(StringUtil::EqualsIgnoreCase(entry.key, "Eyelids Alpha Channel"))
                            {
                                config.faceConfig.eyelidsAlphaChannel = Services::GetAssets()->LoadTexture(entry.value);

                                // If we have eyelids and an alpha channel, just apply it right away, why not?
                                if(config.faceConfig.eyelidsTexture != nullptr && config.faceConfig.eyelidsAlphaChannel != nullptr)
                                {
                                    config.faceConfig.eyelidsTexture->ApplyAlphaChannel(*config.faceConfig.eyelidsAlphaChannel);
                                }
                            }
                            else if(StringUtil::EqualsIgnoreCase(entry.key, "Blink Anims"))
                            {
                                // Value will be a comma-separated list of elements. Gotta split them up.
                                std::vector<std::string> tokens = StringUtil::Split(entry.value, ',');

                                // First element is an animation name, second is a probability, and so on.
                                // Technically, I think the system was meant to support a variable-sized list of anims and probabilities.
                                // But in practice, it seems to only have ever supported 2 blink anims per character.
                                for(int i = 0; i < tokens.size(); i += 2)
                                {
                                    if(i == 0)
                                    {
                                        config.faceConfig.blinkAnim1 = Services::GetAssets()->LoadAnimation(tokens[i]);
                                        config.faceConfig.blinkAnim1Probability = (i + 1 < tokens.size()) ? StringUtil::ToInt(tokens[i + 1]) : 0;
                                    }
                                    else
                                    {
                                        config.faceConfig.blinkAnim2 = Services::GetAssets()->LoadAnimation(tokens[i]);
                                        config.faceConfig.blinkAnim2Probability = (i + 1 < tokens.size()) ? StringUtil::ToInt(tokens[i + 1]) : 0;
                                    }
                                }
                            }
                            else if(StringUtil::EqualsIgnoreCase(entry.key, "Blink Frequency"))
                            {
                                config.faceConfig.blinkFrequency = entry.GetValueAsVector2();
                            }
                            else if(StringUtil::EqualsIgnoreCase(entry.key, "Mouth Offset"))
                            {
                                config.faceConfig.mouthOffset = entry.GetValueAsVector2();
                            }
                            else if(StringUtil::EqualsIgnoreCase(entry.key, "Mouth Size"))
                            {
                                config.faceConfig.mouthSize = entry.GetValueAsVector2();
                            }
                        }
                    }
                }
            }
            delete[] facesBuffer;
        }

        // Load in valid actors list.
        {
            unsigned int actorsBufferSize = 0;
            char* actorsBuffer = Services::GetAssets()->LoadRaw("Actors.txt", actorsBufferSize);

            // Parse as INI file.
            IniParser actorsParser(actorsBuffer, actorsBufferSize);
            actorsParser.ParseAll();

            IniSection actors = actorsParser.GetSection("ACTORS");
            for(auto& line : actors.lines)
            {
                IniKeyValue& entry = line.entries.front();
                mCharacterNouns.insert(entry.key);
            }
            delete[] actorsBuffer;
        }
    });
}

CharacterConfig& CharacterManager::GetCharacterConfig(const std::string& identifier)
{
	auto it = mCharacterConfigs.find(identifier);
	if(it != mCharacterConfigs.end())
	{
        return it->second;
	}
	return mDefaultCharacterConfig;
}

bool CharacterManager::IsValidName(const std::string& name)
{
    std::string key = name;
    StringUtil::ToUpper(key);

	if(mCharacterNouns.find(key) == mCharacterNouns.end())
	{
		Services::GetReports()->Log("Error", StringUtil::Format("Error: who the hell is '%s'?", name.c_str()));
		return false;
	}
	return true;
}
