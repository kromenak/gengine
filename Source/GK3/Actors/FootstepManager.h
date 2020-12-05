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

#include "Type.h"

class Audio;

struct ShoeSounds
{
	std::unordered_map<std::string, std::vector<Audio*>> floorTypeToFootsteps;
	std::unordered_map<std::string, std::vector<Audio*>> floorTypeToFootscuffs;
};

class FootstepManager
{
	TYPE_DECL_BASE();
public:
	FootstepManager();
	
	Audio* GetFootstep(std::string shoeType, std::string floorTextureName);
	Audio* GetFootscuff(std::string shoeType, std::string floorTextureName);
	
private:
	// There's probably a more efficient way to store this stuff.
	// But let's do something simple for now.
	
	// Need to be able to determine a floor type from a texture name.
	std::unordered_map<std::string, std::string> mTextureNameToFloorType;
	
	// Need to be able to get get audio lists by shoe type.
	std::unordered_map<std::string, ShoeSounds> mShoeTypeToShoeSounds;
};
