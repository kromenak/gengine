#include "CharacterManager.h"

#include "AssetManager.h"
#include "IniParser.h"
#include "Loader.h"
#include "Profiler.h"
#include "ReportManager.h"
#include "TextAsset.h"
#include "Texture.h"

CharacterManager gCharacterManager;

void CharacterManager::Init()
{
    Loader::Load([this]() {
        TIMER_SCOPED("CharacterManager::Load");

        // Read in faces first, as character configs reference these.
        {
            // Get FACES text file as a raw buffer.
            TextAsset* textFile = gAssetManager.LoadText("FACES.TXT", AssetScope::Manual);

            // Pass that along to INI parser, since it is plain text and in INI format.
            IniParser facesParser(textFile->GetText(), textFile->GetTextLength());
            facesParser.SetMultipleKeyValuePairsPerLine(false); // Stops splitting on commas.

            // All face configs start from this template.
            // Note that the DEFAULTS section occurs before any face config instance, so that's why this works.
            FaceConfig defaultFaceConfig;

            // Read in each section of the FACES file.
            IniSection section;
            while(facesParser.ReadNextSection(section))
            {
                // Default section contains some values to use if not defined for a particular character.
                if(StringUtil::EqualsIgnoreCase(section.name, "Default"))
                {
                    for(auto& line : section.lines)
                    {
                        IniKeyValue& entry = line.entries.front();
                        if(StringUtil::EqualsIgnoreCase(entry.key, "Max Look Distance"))
                        {
                            defaultFaceConfig.maxEyeLookDistance = entry.GetValueAsVector2();
                        }
                        else if(StringUtil::EqualsIgnoreCase(entry.key, "Jitter Frequency"))
                        {
                           defaultFaceConfig.eyeJitterFrequency = entry.GetValueAsVector2();
                        }
                        else if(StringUtil::EqualsIgnoreCase(entry.key, "Max Jitter Distance"))
                        {
                            defaultFaceConfig.maxEyeJitterDistance = entry.GetValueAsVector2();
                        }
                        else if(StringUtil::EqualsIgnoreCase(entry.key, "Eye Field of View"))
                        {
                            defaultFaceConfig.eyeFieldOfView = entry.GetValueAsVector2();
                        }
                        else if(StringUtil::EqualsIgnoreCase(entry.key, "Eye Short Field of View"))
                        {
                            defaultFaceConfig.eyeShortFieldOfView = entry.GetValueAsVector2();
                        }
                        else if(StringUtil::EqualsIgnoreCase(entry.key, "Eye Separation"))
                        {
                            defaultFaceConfig.eyeSeparation = entry.GetValueAsFloat();
                        }
                        else if(StringUtil::EqualsIgnoreCase(entry.key, "Head Radius"))
                        {
                            defaultFaceConfig.headRadius = entry.GetValueAsFloat();
                        }
                        else if(StringUtil::EqualsIgnoreCase(entry.key, "Head Center Offset"))
                        {
                            defaultFaceConfig.headCenterOffset = entry.GetValueAsVector3();
                        }
                        else if(StringUtil::EqualsIgnoreCase(entry.key, "Blink Frequency"))
                        {
                            defaultFaceConfig.blinkFrequency = entry.GetValueAsVector2();
                        }
                    }

                    // Save as a fallback for characters who don't have faces defined for one reason or another.
                    mDefaultFaceConfig = defaultFaceConfig;
                }
                else if(StringUtil::EqualsIgnoreCase(section.name, "Eyes"))
                {
                    // Eyes section contains eye definitions.
                    // But they are all the same...<bitmap> = 4x4, DownSampleOnly
                    // No need to read in.
                }
                else
                {
                    // Must be an instance of a face config.
                    auto it = mFaceConfigs.emplace(section.name, defaultFaceConfig).first; // returns pair (iterator, bool)
                    FaceConfig& faceConfig = it->second;
                    faceConfig.identifier = section.name;

                    // First, try to load the entry's face/eyelid/forehead textures.
                    // These are derived from the section name.
                    faceConfig.faceTexture = gAssetManager.LoadSceneTexture(section.name + "_face");
                    faceConfig.eyelidsTexture = gAssetManager.LoadTexture(section.name + "_eyelids");
                    faceConfig.foreheadTexture = gAssetManager.LoadTexture(section.name + "_forehead");
                    faceConfig.mouthTexture = gAssetManager.LoadTexture(section.name + "_mouth00");

                    // Each entry is a face property for the character.
                    for(auto& line : section.lines)
                    {
                        IniKeyValue& entry = line.entries.front();
                        if(StringUtil::EqualsIgnoreCase(entry.key, "Left Eye Name"))
                        {
                            faceConfig.leftEyeTexture = gAssetManager.LoadTexture(entry.value);
                        }
                        else if(StringUtil::EqualsIgnoreCase(entry.key, "Right Eye Name"))
                        {
                            faceConfig.rightEyeTexture = gAssetManager.LoadTexture(entry.value);
                        }
                        else if(StringUtil::EqualsIgnoreCase(entry.key, "Left Eye Offset"))
                        {
                            faceConfig.leftEyeOffset = entry.GetValueAsVector2();
                        }
                        else if(StringUtil::EqualsIgnoreCase(entry.key, "Right Eye Offset"))
                        {
                            faceConfig.rightEyeOffset = entry.GetValueAsVector2();
                        }
                        else if(StringUtil::EqualsIgnoreCase(entry.key, "Left Eye Bias"))
                        {
                            faceConfig.leftEyeBias = entry.GetValueAsVector2();
                        }
                        else if(StringUtil::EqualsIgnoreCase(entry.key, "Right Eye Bias"))
                        {
                            faceConfig.rightEyeBias = entry.GetValueAsVector2();
                        }
                        else if(StringUtil::EqualsIgnoreCase(entry.key, "Max Look Distance"))
                        {
                            faceConfig.maxEyeLookDistance = entry.GetValueAsVector2();
                        }
                        else if(StringUtil::EqualsIgnoreCase(entry.key, "Jitter Frequency"))
                        {
                            faceConfig.eyeJitterFrequency = entry.GetValueAsVector2();
                        }
                        else if(StringUtil::EqualsIgnoreCase(entry.key, "Max Jitter Distance"))
                        {
                            faceConfig.maxEyeLookDistance = entry.GetValueAsVector2();
                        }
                        else if(StringUtil::EqualsIgnoreCase(entry.key, "Eye Field of View"))
                        {
                            faceConfig.eyeFieldOfView = entry.GetValueAsVector2();
                        }
                        else if(StringUtil::EqualsIgnoreCase(entry.key, "Eye Short Field of View"))
                        {
                            faceConfig.eyeShortFieldOfView = entry.GetValueAsVector2();
                        }
                        else if(StringUtil::EqualsIgnoreCase(entry.key, "Eye Separation"))
                        {
                            faceConfig.eyeSeparation = entry.GetValueAsFloat();
                        }
                        else if(StringUtil::EqualsIgnoreCase(entry.key, "Head Radius"))
                        {
                            faceConfig.headRadius = entry.GetValueAsFloat();
                        }
                        else if(StringUtil::EqualsIgnoreCase(entry.key, "Head Center Offset"))
                        {
                            faceConfig.headCenterOffset = entry.GetValueAsVector3();
                        }
                        else if(StringUtil::EqualsIgnoreCase(entry.key, "Forehead Offset"))
                        {
                            faceConfig.foreheadOffset = entry.GetValueAsVector2();
                        }
                        else if(StringUtil::EqualsIgnoreCase(entry.key, "Eyelids Offset"))
                        {
                            faceConfig.eyelidsOffset = entry.GetValueAsVector2();
                        }
                        else if(StringUtil::EqualsIgnoreCase(entry.key, "Eyelids Alpha Channel"))
                        {
                            faceConfig.eyelidsAlphaChannel = gAssetManager.LoadTexture(entry.value);

                            // If we have eyelids and an alpha channel, just apply it right away, why not?
                            if(faceConfig.eyelidsTexture != nullptr && faceConfig.eyelidsAlphaChannel != nullptr)
                            {
                                faceConfig.eyelidsTexture->ApplyAlphaChannel(*faceConfig.eyelidsAlphaChannel);
                            }
                        }
                        else if(StringUtil::EqualsIgnoreCase(entry.key, "Blink Anims"))
                        {
                            // Value will be a comma-separated list of elements. Gotta split them up.
                            std::vector<std::string> tokens = StringUtil::Split(entry.value, ',');

                            // First element is an animation name, second is a probability, and so on.
                            // Technically, I think the system was meant to support a variable-sized list of anims and probabilities.
                            // But in practice, it seems to only have ever supported 2 blink anims per character.
                            for(size_t i = 0; i < tokens.size(); i += 2)
                            {
                                if(i == 0)
                                {
                                    faceConfig.blinkAnim1 = gAssetManager.LoadAnimation(tokens[i]);
                                    faceConfig.blinkAnim1Probability = (i + 1 < tokens.size()) ? StringUtil::ToInt(tokens[i + 1]) : 0;
                                }
                                else
                                {
                                    faceConfig.blinkAnim2 = gAssetManager.LoadAnimation(tokens[i]);
                                    faceConfig.blinkAnim2Probability = (i + 1 < tokens.size()) ? StringUtil::ToInt(tokens[i + 1]) : 0;
                                }
                            }
                        }
                        else if(StringUtil::EqualsIgnoreCase(entry.key, "Blink Frequency"))
                        {
                            faceConfig.blinkFrequency = entry.GetValueAsVector2();
                        }
                        else if(StringUtil::EqualsIgnoreCase(entry.key, "Mouth Offset"))
                        {
                            faceConfig.mouthOffset = entry.GetValueAsVector2();
                        }
                        else if(StringUtil::EqualsIgnoreCase(entry.key, "Mouth Size"))
                        {
                            faceConfig.mouthSize = entry.GetValueAsVector2();
                        }
                    }
                }
            }

            // Done with this asset.
            delete textFile;
        }
        
        // Read in characters.
        {
            // Get CHARACTERS text file as a raw buffer.
            TextAsset* textFile = gAssetManager.LoadText("CHARACTERS.TXT", AssetScope::Manual);

            // Pass that along to INI parser, since it is plain text and in INI format.
            IniParser parser(textFile->GetText(), textFile->GetTextLength());
            
            // Read one section at a time.
            // Each section correlates to one character.
            // The section name is the character's three-letter code (GAB, ABE, GRA, etc).
            IniSection section;
            while(parser.ReadNextSection(section))
            {
                auto it = mCharacterConfigs.emplace(section.name, CharacterConfig()).first;

                CharacterConfig& config = it->second;
                config.identifier = section.name;

                // Use character identifier as face identifier, by default.
                // An override may be specified while parsing this section.
                std::string faceIdentifer = config.identifier;

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
                    else if(StringUtil::EqualsIgnoreCase(entry.key, "ShoeType"))
                    {
                        config.shoeType = entry.value;
                    }
                    else if(StringUtil::EqualsIgnoreCase(entry.key, "StartAnim"))
                    {
                        config.walkStartAnim = gAssetManager.LoadAnimation(entry.value);
                    }
                    else if(StringUtil::EqualsIgnoreCase(entry.key, "ContAnim"))
                    {
                        config.walkLoopAnim = gAssetManager.LoadAnimation(entry.value);
                    }
                    else if(StringUtil::EqualsIgnoreCase(entry.key, "StopAnim"))
                    {
                        config.walkStopAnim = gAssetManager.LoadAnimation(entry.value);
                    }
                    else if(StringUtil::EqualsIgnoreCase(entry.key, "StartTurnRightAnim"))
                    {
                        config.walkStartTurnRightAnim = gAssetManager.LoadAnimation(entry.value);
                    }
                    else if(StringUtil::EqualsIgnoreCase(entry.key, "StartTurnLeftAnim"))
                    {
                        config.walkStartTurnLeftAnim = gAssetManager.LoadAnimation(entry.value);
                    }
                    else if(StringUtil::EqualsIgnoreCase(entry.key, "HipAxesMeshIndex"))
                    {
                        config.hipAxesMeshIndex = static_cast<int8_t>(entry.GetValueAsInt());
                    }
                    else if(StringUtil::EqualsIgnoreCase(entry.key, "HipAxesGroupIndex"))
                    {
                        config.hipAxesGroupIndex = static_cast<int8_t>(entry.GetValueAsInt());
                    }
                    else if(StringUtil::EqualsIgnoreCase(entry.key, "HipAxesPointIndex"))
                    {
                        config.hipAxesPointIndex = static_cast<int16_t>(entry.GetValueAsInt());
                    }
                    else if(StringUtil::EqualsIgnoreCase(entry.key, "LShoeAxesMeshIndex"))
                    {
                        config.leftShoeAxesMeshIndex = static_cast<int8_t>(entry.GetValueAsInt());
                    }
                    else if(StringUtil::EqualsIgnoreCase(entry.key, "LShoeAxesGroupIndex"))
                    {
                        config.leftShoeAxesGroupIndex = static_cast<int8_t>(entry.GetValueAsInt());
                    }
                    else if(StringUtil::EqualsIgnoreCase(entry.key, "LShoeAxesPointIndex"))
                    {
                        config.leftShoeAxesPointIndex = static_cast<int16_t>(entry.GetValueAsInt());
                    }
                    else if(StringUtil::EqualsIgnoreCase(entry.key, "RShoeAxesMeshIndex"))
                    {
                        config.rightShoeAxesMeshIndex = static_cast<int8_t>(entry.GetValueAsInt());
                    }
                    else if(StringUtil::EqualsIgnoreCase(entry.key, "RShoeAxesGroupIndex"))
                    {
                        config.rightShoeAxesGroupIndex = static_cast<int8_t>(entry.GetValueAsInt());
                    }
                    else if(StringUtil::EqualsIgnoreCase(entry.key, "RShoeAxesPointIndex"))
                    {
                        config.rightShoeAxesPointIndex = static_cast<int16_t>(entry.GetValueAsInt());
                    }
                    else if(StringUtil::EqualsIgnoreCase(entry.key, "HeadMeshIndex"))
                    {
                        config.headMeshIndex = static_cast<int8_t>(entry.GetValueAsInt());
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
                                config.clothesAnims[i].second = gAssetManager.LoadAnimation(entry.value);
                                break;
                            }
                        }
                    }
                    else if(StringUtil::EqualsIgnoreCase(entry.key, "MouthName"))
                    {
                        faceIdentifer = entry.value;
                    }
                }

                // Try to assign a face config.
                auto faceIt = mFaceConfigs.find(faceIdentifer);
                if(faceIt != mFaceConfigs.end())
                {
                    config.faceConfig = &faceIt->second;
                }
                else
                {
                    config.faceConfig = &mDefaultFaceConfig;
                }
            }

            // Done with this asset.
            delete textFile;
        }

        // Load in valid actors list.
        {
            TextAsset* textFile = gAssetManager.LoadText("Actors.txt", AssetScope::Manual);

            // Parse as INI file.
            IniParser actorsParser(textFile->GetText(), textFile->GetTextLength());
            actorsParser.ParseAll();

            IniSection actors = actorsParser.GetSection("ACTORS");
            for(auto& line : actors.lines)
            {
                IniKeyValue& entry = line.entries.front();
                mCharacterNouns.insert(entry.key);
            }

            // Done with this asset.
            delete textFile;
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
		gReportManager.Log("Error", StringUtil::Format("Error: who the hell is '%s'?", name.c_str()));
		return false;
	}
	return true;
}
