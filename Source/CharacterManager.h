//
// CharacterManager.h
//
// Clark Kromenaker
//
// Provides configuration data about the various characters in the game.
//
#pragma once
#include <string>
#include <unordered_map>

#include "AtomicTypes.h"
#include "Type.h"
#include "Vector4.h"

class Animation;

struct CharacterConfig
{
	std::string identifier;
	
	uint8 headMeshIndex = 0;
	Vector4 headFov; 			// left, top, right, bottom
	Vector4 headComfortableFov;
	
	uint8 hipAxesMeshIndex = 0;
	uint8 hipAxesGroupIndex = 0;
	uint16 hipAxesPointIndex = 0;
	
	uint8 leftShoeAxesMeshIndex = 0;
	uint8 leftShoeAxesGroupIndex = 0;
	uint16 leftShoeAxesPointIndex = 0;
	
	uint8 rightShoeAxesMeshIndex = 0;
	uint8 rightShoeAxesGroupIndex = 0;
	uint16 rightShoeAxesPointIndex = 0;
	
	float walkerHeight = 72.0f;
	float shoeThickness = 0.75f;
	std::string shoeType = "Male Boot";
	
	Animation* walkStartAnim = nullptr;
	Animation* walkStartTurnRightAnim = nullptr;
	Animation* walkStartTurnLeftAnim = nullptr;
	
	uint16 walkStartTurnRightFrame = 0;
	uint16 walkStartTurnLeftFrame = 0;
	
	Animation* walkLoopAnim = nullptr;
	Animation* walkStopAnim = nullptr;
	
	//TODO: Clothes anims?
	//TODO: Eye/mouth coordinates?
	//TODO: Blink?
	//TODO: Model/mouth name?
};

class CharacterManager
{
	TYPE_DECL_BASE();
public:
	CharacterManager();
	
	CharacterConfig& GetCharacterConfig(std::string identifier);
	
	void SetCharacterLocation(std::string name, std::string location);
	std::string GetCharacterLocation(std::string name) const;
	
	void SetCharacterOffstage(std::string name);
	bool IsCharacterOffstage(std::string name) const;
	
private:
	// Character configs, keyed by the 3-letter character identifier.
	std::unordered_map<std::string, CharacterConfig> mCharacterConfigs;
	
	// A default character config, in case you request one that doesn't exist.
	CharacterConfig mDefaultCharacterConfig;
	
	// A mapping of character to location. If not present, the actor is "offstage".
	std::unordered_map<std::string, std::string> mCharacterLocations;
};
