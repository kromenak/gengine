//
// FootstepManager.h
//
// Clark Kromenaker
//
// Handles complexities of storing and retrieving appropriate
// footstep and footscuff sound effects based on a character's
// shoe type, current walking surface, and config file contents.
//
#pragma once
#include <unordered_map>
#include <vector>

#include "StringUtil.h"
#include "Type.h"

class Audio;

struct ShoeSounds
{
	std::string_map_ci<std::vector<Audio*>> floorTypeToFootsteps;
	std::string_map_ci<std::vector<Audio*>> floorTypeToFootscuffs;
};

class FootstepManager
{
public:
    void Init();
	
	Audio* GetFootstep(const std::string& shoeType, const std::string& floorTextureName);
	Audio* GetFootscuff(const std::string& shoeType, const std::string& floorTextureName);
	
private:
	// There's probably a more efficient way to store this stuff.
	// But let's do something simple for now.
	
	// Need to be able to determine a floor type from a texture name.
	std::string_map_ci<std::string> mTextureNameToFloorType;
	
	// Need to be able to get get audio lists by shoe type.
	std::string_map_ci<ShoeSounds> mShoeTypeToShoeSounds;
};

extern FootstepManager gFootstepManager;