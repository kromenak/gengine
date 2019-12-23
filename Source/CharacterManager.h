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
#include "Vector2.h"
#include "Vector3.h"
#include "Vector4.h"

class Animation;
class Texture;

struct FaceConfig
{
	// Default textures for face/eyelids/forehead.
	Texture* faceTexture = nullptr;
	Texture* eyelidsTexture = nullptr;
	Texture* foreheadTexture = nullptr;
	
	// Left and right eye textures.
	Texture* leftEyeTexture = nullptr;
	Texture* rightEyeTexture = nullptr;
	
	// Offset (in pixels) of top-left corner of eyes in the face texture.
	Vector2 leftEyeOffset;
	Vector2 rightEyeOffset;
	
	// ???
	Vector2 leftEyeBias;
	Vector2 rightEyeBias;
	
	// Max distance left/right eyes can move, in pixels.
	Vector2 maxEyeLookDistance;
	
	// Frequency of eye jitters in milliseconds.
	// And max jitter distance in each direction, in subpixels.
	Vector2 eyeJitterFrequency;
	Vector2 maxEyeJitterDistance;
	
	// Horizontal & vertical FOV, in degrees.
	// Second one is used to determine whether to move head.
	Vector2 eyeFieldOfView;
	Vector2 eyeShortFieldOfView;
	
	// How far apart eyes are, in inches.
	float eyeSeparation = 0.0f;
	
	// Approximate head radius, in inches.
	// And an offset to "virtual" head center, in inches (everyone just uses (0,0,0)).
	float headRadius = 0.0f;
	Vector3 headCenterOffset;
	
	// Offset (in pixels) of top-left corner of forehead in the face texture.
	Vector2 foreheadOffset;
	
	// Offset (in pixels) of top-left corner of eyelids in the face texture.
	Vector2 eyelidsOffset;
	
	// An optional(?) alpha channel for the eyelids.
	// Seems to be used to determine where eye textures should show?
	Texture* eyelidsAlphaChannel = nullptr;
	
	// Up to two possible blink anims with different probabilities.
	Animation* blinkAnim1 = nullptr;
	int blinkAnim1Probability = 0;
	Animation* blinkAnim2 = nullptr;
	int blinkAnim2Probability = 0;
	
	// Min and max period between blinks, in milliseconds.
	Vector2 blinkFrequency;
	
	// Offset (in pixels) of top-left corner of mouth in the face texture.
	// Plus, mouth size? But mouth textures all match this size anyway.
	Vector2 mouthOffset;
	Vector2 mouthSize;
};

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
	
	FaceConfig faceConfig;
	
	//TODO: Clothes anims?
};

class CharacterManager
{
	TYPE_DECL_BASE();
public:
	CharacterManager();
	
	CharacterConfig& GetCharacterConfig(std::string identifier);
	
private:
	// Character configs, keyed by the 3-letter character identifier.
	std::unordered_map<std::string, CharacterConfig> mCharacterConfigs;
	
	// A default character config, in case you request one that doesn't exist.
	CharacterConfig mDefaultCharacterConfig;
};
