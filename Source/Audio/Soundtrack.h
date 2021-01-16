//
// Soundtrack.h
//
// Clark Kromenaker
//
// Represents a "Soundtrack" asset, used to create dynamic musical
// scores for game locales using music snippets and node-based logic.
//
// In-memory representation of ".STK" files.
//
#pragma once
#include "Asset.h"

#include <string>
#include <vector>
#include <cstdlib>

#include "Vector3.h"

struct IniSection;

enum class SoundtrackSoundType
{
    Ambient,
    SFX
};

struct SoundtrackNode
{
    // Number of times node will execute before being ignored.
    // For example, if set to "1", node only executes first time through node list.
    // If set to 2, node only executes first two times through node list.
    // A value of zero means it will always execute.
    int repeat = 0;
    
    // Percent chance this node will be executed. Should be 1-100.
    // Repeat count is still decremented if node is not executed due to random!
    int random = 100;
    
    virtual int Execute(SoundtrackSoundType soundType) = 0;
    
    virtual bool IsLooping()
    {
        return false;
    }
    
    virtual void Reset()
    {
        executionCount = 0;
    }
    
protected:
    int executionCount = 0;
};

struct WaitNode : public SoundtrackNode
{
    // Amount of time to wait. For a fixed/constant wait time, only set "min" value.
    // If both are set, wait time is randomly chosen from range.
    int minWaitTimeMs = 0;
    int maxWaitTimeMs = 0;
    
    int Execute(SoundtrackSoundType soundType) override;
};

struct SoundNode : public SoundtrackNode
{
    // Name of audio asset to play.
    std::string soundName;
    
    // Volume to play at; a percent 0-100.
    int volume = 100;
    
    // If true, this node loops.
    // A looping node stops any subsequent node from running!
    bool loop = false;
    
    // If non-zero, sound will fade in over X milliseconds.
    int fadeInTimeMs = 0;
    
    // Indicates how this node behaves if the Soundtrack is stopped during playback.
    enum class StopMethod
    {
        PlayToEnd,  // Wait until the sound finishes playing.
        FadeOut,    // Fade out (using fadeOutTimeMs).
        Immediate   // Stop playing immediately.
    };
    StopMethod stopMethod = StopMethod::PlayToEnd;
    int fadeOutTimeMs = 0;
    
    // If true, this is a 3D sound.
    bool is3d = false;
    
    // Min and max distances for 3D sound volume.
    float minDist = 0.0f;
    float maxDist = 0.0f;
    
    // The position of a 3D sound.
    Vector3 position;
    
    // For 3D sounds, a model in the scene to follow. Sound plays at center of bounding box.
    // If empty, or model is not found, 3D position is used instead.
    std::string followModelName;
    
    bool IsLooping() override { return loop; }
    int Execute(SoundtrackSoundType soundType) override;
};

struct PrsNode : public SoundtrackNode
{
    // I believe PRS stands for "Pick Random Sound"
    // So basically, PRS consists of multiple sound nodes, one of which is picked at random.
    std::vector<SoundNode*> soundNodes;
    
    int Execute(SoundtrackSoundType soundType) override
    {
        if(soundNodes.size() == 0) { return 0; }
        
        int randomIndex = rand() % soundNodes.size();
        return soundNodes[randomIndex]->Execute(soundType);
    }
};

class Soundtrack : public Asset
{
public:
    Soundtrack(std::string name, char* data, int dataLength);
    
    SoundtrackSoundType GetSoundType() const { return mSoundType; }
    std::vector<SoundtrackNode*> GetNodesCopy() const { return mNodes; }
    
private:
    // Type indicates whether this audio is considered music or SFX or what.
    SoundtrackSoundType mSoundType = SoundtrackSoundType::Ambient;
    
    // A soundtrack is a list of nodes that play audio or wait X seconds.
    std::vector<SoundtrackNode*> mNodes;
    
    void ParseFromData(char* data, int dataLength);
    SoundNode* ParseSoundNodeFromSection(IniSection& section);
};
