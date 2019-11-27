//
// FootstepManager.cpp
//
// Clark Kromenaker
//
#include "FootstepManager.h"

#include "IniParser.h"
#include "Random.h"
#include "Services.h"

TYPE_DEF_BASE(FootstepManager);

FootstepManager::FootstepManager()
{
	// STEP 1: FLOORMAP maps texture names to a floor type.
	// Get FLOORMAP text file as a raw buffer.
	unsigned int bufferSize = 0;
	char* buffer = Services::GetAssets()->LoadRaw("FLOORMAP.TXT", bufferSize);
	
	// Pass that along to INI parser, since it is plain text and in INI format.
	IniParser parser(buffer, bufferSize);
	
	// There's only one section in this file - FloorMap.
	IniSection section;
	while(parser.ReadNextSection(section))
	{
		for(auto& line : section.lines)
		{
			// Key of first entry is the floor type.
			std::string floorType = line.entries[0].key;
			
			// All values are texture names (with no extension).
			// Note the value of the first entry is also used (it is a valid texture name too).
			// Map them to the floor type.
			for(int i = 0; i < line.entries.size(); ++i)
			{
				IniKeyValue& current = line.entries[i];
				mTextureNameToFloorType[current.value] = floorType;
			}
		}
	}
	delete[] buffer;
	
	// STEP 2: FOOTSTEPS maps floor types to audio files for footsteps.
	// Next up: read in all the footstep data.
	buffer = Services::GetAssets()->LoadRaw("FOOTSTEPS.TXT", bufferSize);
	
	// Again, it's just an INI text file.
	IniParser footstepParser(buffer, bufferSize);

	// Each section in this file correlates to a shoe type.
	while(footstepParser.ReadNextSection(section))
	{
		// Section name is the shoe type.
		std::string shoeType = section.name;
		
		// This will either return an existing list or create a new one.
		ShoeSounds& shoeSounds = mShoeTypeToShoeSounds[shoeType];
		
		// Each entry maps a floor type to an audio file name.
		for(auto& line : section.lines)
		{
			// Key of first entry is the floor type.
			std::string floorType = line.entries[0].key;
			
			// Grab the audio vector for this floor type.
			// Gets an existing list or creates a new empty one.
			auto& footsteps = shoeSounds.floorTypeToFootsteps[floorType];
			
			// All values are audio files that go along with this shoeType/floorType pair.
			for(int i = 0; i < line.entries.size(); ++i)
			{
				IniKeyValue& current = line.entries[i];
				Audio* audio = Services::GetAssets()->LoadAudio(current.value);
				if(audio != nullptr)
				{
					footsteps.push_back(audio);
				}
			}
		}
	}
	delete[] buffer;
	
	// STEP 2: FOOTSCUFFS maps floor types to audio files for footscuffs.
	// Finally, very similar thing with the footscuff data.
	buffer = Services::GetAssets()->LoadRaw("FOOTSCUFFS.TXT", bufferSize);
	
	// Again, it's just an INI text file.
	IniParser footscuffParser(buffer, bufferSize);
	
	// Each section in this file correlates to a shoe type.
	while(footscuffParser.ReadNextSection(section))
	{
		// Section name is the shoe type.
		std::string shoeType = section.name;
		
		// This will either return an existing list or create a new one.
		ShoeSounds& shoeSounds = mShoeTypeToShoeSounds[shoeType];
		
		// Each entry maps a floor type to an audio file name.
		for(auto& line : section.lines)
		{
			// Key of first entry is the floor type.
			std::string floorType = line.entries[0].key;
			
			// Grab the audio vector for this floor type.
			// Again, gets an existing list or creates a new empty one.
			auto& footscuffs = shoeSounds.floorTypeToFootscuffs[floorType];
			
			// All values are audio files that go along with this shoeType/floorType pair.
			for(int i = 0; i < line.entries.size(); ++i)
			{
				IniKeyValue& current = line.entries[i];
				Audio* audio = Services::GetAssets()->LoadAudio(current.value);
				if(audio != nullptr)
				{
					footscuffs.push_back(audio);
				}
			}
		}
	}
	delete[] buffer;
}

Audio* FootstepManager::GetFootstep(std::string shoeType, std::string floorTextureName)
{
	// First off, look up the floor type that correlates to this floor texture.
	// If we can't find it, no footstep sound!
	auto floorTypeIt = mTextureNameToFloorType.find(floorTextureName);
	if(floorTypeIt == mTextureNameToFloorType.end())
	{
		return nullptr;
	}
	
	// Look up the shoe sounds based on shoe type.
	// Again, if we can't find it, no footstep sound!
	auto shoeTypeIt = mShoeTypeToShoeSounds.find(shoeType);
	if(shoeTypeIt == mShoeTypeToShoeSounds.end())
	{
		return nullptr;
	}
	
	// For this shoe type, grab the list of sounds we can use for this floor type.
	// Yet again, if we can't find any sounds, no footstep sound!
	auto& floorTypeToFootsteps = shoeTypeIt->second.floorTypeToFootsteps;
	auto footstepSoundsIt = floorTypeToFootsteps.find(floorTypeIt->second);
	if(footstepSoundsIt == floorTypeToFootsteps.end())
	{
		return nullptr;
	}
	
	// We finally have the list of sounds. Hopefully it's not empty...
	auto& footstepSounds = footstepSoundsIt->second;
	if(footstepSounds.size() <= 0) { return nullptr; }
	
	// Return one randomly.
	return footstepSounds[Random::Range(0, (int)footstepSounds.size())];
}

Audio* FootstepManager::GetFootscuff(std::string shoeType, std::string floorTextureName)
{
	// First off, look up the floor type that correlates to this floor texture.
	// If we can't find it, no footstep sound!
	auto floorTypeIt = mTextureNameToFloorType.find(floorTextureName);
	if(floorTypeIt == mTextureNameToFloorType.end())
	{
		return nullptr;
	}
	
	// Look up the shoe sounds based on shoe type.
	// Again, if we can't find it, no footstep sound!
	auto shoeTypeIt = mShoeTypeToShoeSounds.find(shoeType);
	if(shoeTypeIt == mShoeTypeToShoeSounds.end())
	{
		return nullptr;
	}
	
	// For this shoe type, grab the list of sounds we can use for this floor type.
	// Yet again, if we can't find any sounds, no footstep sound!
	auto& floorTypeToFootscuffs = shoeTypeIt->second.floorTypeToFootscuffs;
	auto footscuffSoundsIt = floorTypeToFootscuffs.find(floorTypeIt->second);
	if(footscuffSoundsIt == floorTypeToFootscuffs.end())
	{
		return nullptr;
	}
	
	// We finally have the list of sounds. Hopefully it's not empty...
	auto& footscuffSounds = footscuffSoundsIt->second;
	if(footscuffSounds.size() <= 0) { return nullptr; }
	
	// Return one randomly.
	return footscuffSounds[Random::Range(0, (int)footscuffSounds.size())];
}
