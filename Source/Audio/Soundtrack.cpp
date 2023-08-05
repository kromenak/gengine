#include "Soundtrack.h"

#include "AssetManager.h"
#include "Audio.h"
#include "GKObject.h"
#include "IniParser.h"
#include "Scene.h"
#include "SceneManager.h"
#include "StringUtil.h"

int WaitNode::Execute(Soundtrack* soundtrack, SoundtrackNodeResults& outResults)
{
    // Since a wait node does not actually have any audio, clear the provided sound handle to reflect this.
    outResults.soundHandle = PlayingSoundHandle();
    outResults.stopMethod = StopMethod::Immediate;
    outResults.fadeOutTimeMs = 0;

    // Do random check. If it fails, we don't execute.
    // But note execution count is still incremented!
    int randomCheck = rand() % 100 + 1;
    if(randomCheck > random) { return 0; }
    
    // We will execute this node. Decide wait time based on min/max.
    if(minWaitTimeMs == maxWaitTimeMs) { return minWaitTimeMs; }
    if(maxWaitTimeMs == 0 && minWaitTimeMs > 0) { return minWaitTimeMs; }
    if(maxWaitTimeMs != 0 && minWaitTimeMs > maxWaitTimeMs) { return minWaitTimeMs; }
    
    // Normal case - random between min and max.
    return (rand() % maxWaitTimeMs + minWaitTimeMs);
}

int SoundNode::Execute(Soundtrack* soundtrack, SoundtrackNodeResults& outResults)
{
    // Do random check. If it fails, we don't execute.
    // But note execution count is still incremented!
    int randomCheck = rand() % 100 + 1;
    if(randomCheck > random) { return 0; }
    
    // Definitely want to play the sound, if it exists.
    Audio* audio = gAssetManager.LoadAudio(soundName, soundtrack->GetScope());
    if(audio == nullptr) { return 0; }

    // Create audio play params struct.
    PlayAudioParams playParams;
    playParams.audio = audio;
    playParams.audioType = soundtrack->GetSoundType();

    // Fade in time is specified in milliseconds, but audio system wants it in seconds.
    playParams.fadeInTime = fadeInTimeMs * 0.001f;

    // Volume is specified as 0-100, but audio system expects 0.0-1.0.
    playParams.volume = volume * 0.01f;

    // Handle 3D parameters.
    playParams.is3d = is3d;
    if(is3d)
    {
        playParams.position = position;
        if(!followModelName.empty())
        {
            GKObject* obj = gSceneManager.GetScene()->GetSceneObjectByModelName(followModelName);
            if(obj != nullptr)
            {
                outResults.followObj = obj;
                playParams.position = obj->GetAudioPosition();
            }
        }

        playParams.minDist = minDist;
        playParams.maxDist = maxDist;
    }

    // Set looping or not.
    playParams.loopCount = loop ? -1 : 0;

    // Get the sound playing!
    outResults.soundHandle = gAudioManager.Play(playParams);
    
    // Let the caller know the desired stop method, in case the soundtrack needs to stop while this node is playing.
    outResults.stopMethod = stopMethod;
    outResults.fadeOutTimeMs = fadeOutTimeMs;
    
    // Return audio length. Gotta convert seconds to milliseconds.
    return (int)(audio->GetDuration() * 1000.0f);
}

int PrsNode::Execute(Soundtrack* soundtrack, SoundtrackNodeResults& outResults)
{
    if(soundNodes.size() == 0) { return 0; }

    int randomIndex = rand() % soundNodes.size();
    return soundNodes[randomIndex]->Execute(soundtrack, outResults);
}

Soundtrack::~Soundtrack()
{
    // Delete dynamically allocated nodes.
    for(auto& node : mNodes)
    {
        delete node;
    }
}

void Soundtrack::Load(uint8_t* data, uint32_t dataLength)
{
    IniParser parser(data, dataLength);
    IniSection section;
    std::vector<SoundNode*> prsSoundNodes;
    while(parser.ReadNextSection(section))
    {
        // If this isn't a PRS node, and we have just parsed one or more PRS nodes,
        // we need to generate the PRS node and add it to our list.
        if(prsSoundNodes.size() > 0
           && !StringUtil::EqualsIgnoreCase(section.name, "PRS"))
        {
            PrsNode* prsNode = new PrsNode();
            prsNode->soundNodes = prsSoundNodes;
            mNodes.push_back(prsNode);
            prsSoundNodes.clear();
        }
        
        if(StringUtil::EqualsIgnoreCase(section.name, "WAIT"))
        {
            // Parse wait node keys and add to nodes list.
            WaitNode* node = new WaitNode();
            for(auto& line : section.lines)
            {
				IniKeyValue& entry = line.entries[0];
                if(StringUtil::EqualsIgnoreCase(entry.key, "MinWaitMs"))
                {
                    node->minWaitTimeMs = entry.GetValueAsInt();
                }
                else if(StringUtil::EqualsIgnoreCase(entry.key, "MaxWaitMs"))
                {
                    node->maxWaitTimeMs = entry.GetValueAsInt();
                }
                else if(StringUtil::EqualsIgnoreCase(entry.key, "Repeat"))
                {
                    node->repeat = entry.GetValueAsInt();
                }
                else if(StringUtil::EqualsIgnoreCase(entry.key, "Random"))
                {
                    node->random = entry.GetValueAsInt();
                }
                else
                {
                    std::cout << "Unexpected key: " << entry.key << std::endl;
                }
            }
            mNodes.push_back(node);
        }
        else if(StringUtil::EqualsIgnoreCase(section.name, "SOUND"))
        {
            // Parse sound node and add to node list.
            SoundNode* node = ParseSoundNodeFromSection(section);
            mNodes.push_back(node);
        }
        else if(StringUtil::EqualsIgnoreCase(section.name, "PRS"))
        {
            // Parse sound node.
            SoundNode* soundNode = ParseSoundNodeFromSection(section);
            
            // Repeat and loop are ignored for PRS.
            soundNode->repeat = 0;
            soundNode->loop = false;
            
            // Add it to prs sound nodes.
            prsSoundNodes.push_back(soundNode);
        }
        else if(StringUtil::EqualsIgnoreCase(section.name, "SOUNDTRACK"))
        {
            for(auto& line : section.lines)
            {
				IniKeyValue& entry = line.entries[0];
                if(StringUtil::EqualsIgnoreCase(entry.key, "SoundType"))
                {
                    if(StringUtil::EqualsIgnoreCase(entry.value, "Music"))
                    {
                        mSoundType = AudioType::Music;
                    }
                    else if(StringUtil::EqualsIgnoreCase(entry.value, "Ambient"))
                    {
                        mSoundType = AudioType::Ambient;
                    }
                    else if(StringUtil::EqualsIgnoreCase(entry.value, "SFX"))
                    {
                        mSoundType = AudioType::SFX;
                    }
                }
                else
                {
                    std::cout << "Unexpected key: " << entry.key << std::endl;
                }
            }
        }
        else
        {
            std::cout << "Unexpected section name: " << section.name << std::endl;
        }
    }
}

SoundNode* Soundtrack::ParseSoundNodeFromSection(IniSection& section)
{
    SoundNode* node = new SoundNode();
    for(auto& line : section.lines)
    {
		IniKeyValue& entry = line.entries[0];
        if(StringUtil::EqualsIgnoreCase(entry.key, "Name"))
        {
            node->soundName = entry.value;
        }
        else if(StringUtil::EqualsIgnoreCase(entry.key, "Volume"))
        {
            node->volume = entry.GetValueAsInt();
        }
        else if(StringUtil::EqualsIgnoreCase(entry.key, "Repeat"))
        {
            node->repeat = entry.GetValueAsInt();
        }
        else if(StringUtil::EqualsIgnoreCase(entry.key, "Random"))
        {
            node->random = entry.GetValueAsInt();
        }
        else if(StringUtil::EqualsIgnoreCase(entry.key, "Loop"))
        {
            node->loop = entry.GetValueAsInt() != 0;
        }
        else if(StringUtil::EqualsIgnoreCase(entry.key, "FadeInMs"))
        {
            node->fadeInTimeMs = entry.GetValueAsInt();
        }
        else if(StringUtil::EqualsIgnoreCase(entry.key, "StopMethod"))
        {
            // Three stop methods are supported.
            // Default to first one if unknown value is passed.
            int stopMethod = entry.GetValueAsInt();
            switch(stopMethod)
            {
            default:
            case 0:
                node->stopMethod = StopMethod::PlayToEnd;
                break;
                
            case 1:
                node->stopMethod = StopMethod::FadeOut;
                break;
                
            case 2:
                node->stopMethod = StopMethod::Immediate;
                break;
            }
        }
        else if(StringUtil::EqualsIgnoreCase(entry.key, "FadeOutMs"))
        {
            node->fadeOutTimeMs = entry.GetValueAsInt();
        }
        else if(StringUtil::EqualsIgnoreCase(entry.key, "3D"))
        {
            node->is3d = entry.GetValueAsInt() != 0;
        }
        else if(StringUtil::EqualsIgnoreCase(entry.key, "MinDist"))
        {
            node->minDist = entry.GetValueAsFloat();
        }
        else if(StringUtil::EqualsIgnoreCase(entry.key, "MaxDist"))
        {
            node->maxDist = entry.GetValueAsFloat();
        }
        else if(StringUtil::EqualsIgnoreCase(entry.key, "X"))
        {
			node->position.x = entry.GetValueAsFloat();
        }
        else if(StringUtil::EqualsIgnoreCase(entry.key, "Y"))
        {
            node->position.y = entry.GetValueAsFloat();
        }
        else if(StringUtil::EqualsIgnoreCase(entry.key, "Z"))
        {
            node->position.z = entry.GetValueAsFloat();
        }
        else if(StringUtil::EqualsIgnoreCase(entry.key, "Follow"))
        {
            node->followModelName = entry.value;
        }
        else
        {
            std::cout << "Unexpected key: " << entry.key << std::endl;
        }
    }
    return node;
}
