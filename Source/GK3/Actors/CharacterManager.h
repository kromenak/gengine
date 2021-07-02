//
// CharacterManager.h
//
// Clark Kromenaker
//
// Provides configuration data about the various characters in the game.
//
#pragma once
#include <set>
#include <string>
#include <unordered_map>
#include <utility>

#include "Atomics.h"
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
	
	U8 headMeshIndex = 0;
	Vector4 headFov; 			// left, top, right, bottom
	Vector4 headComfortableFov;
	
	U8 hipAxesMeshIndex = 0;
	U8 hipAxesGroupIndex = 0;
	U16 hipAxesPointIndex = 0;
	
	U8 leftShoeAxesMeshIndex = 0;
	U8 leftShoeAxesGroupIndex = 0;
	U16 leftShoeAxesPointIndex = 0;
	
	U8 rightShoeAxesMeshIndex = 0;
	U8 rightShoeAxesGroupIndex = 0;
	U16 rightShoeAxesPointIndex = 0;
	
	float walkerHeight = 72.0f;
	float shoeThickness = 0.75f;
	std::string shoeType = "Male Boot";
	
	Animation* walkStartAnim = nullptr;
	Animation* walkStartTurnRightAnim = nullptr;
	Animation* walkStartTurnLeftAnim = nullptr;
	
	U16 walkStartTurnRightFrame = 0;
	U16 walkStartTurnLeftFrame = 0;
	
	Animation* walkLoopAnim = nullptr;
	Animation* walkStopAnim = nullptr;
	
	FaceConfig faceConfig;

    // Characters can have up to three "sets of clothes."
    // The clothes anim will perform texture swaps on the model.
    // To determine which to apply, the animation name is queried:
    // *Default = use if no other applies
    // *TimeBlockStr = use if at or after this timeblock
    static const int kMaxClothesAnims = 3;
    std::pair<std::string, Animation*> clothesAnims[kMaxClothesAnims];
};

class CharacterManager
{
	TYPE_DECL_BASE();
public:
	CharacterManager();
	
	CharacterConfig& GetCharacterConfig(const std::string& identifier);
	
	bool IsValidName(const std::string& name);
	
private:
	// Set of valid nouns referring to characters.
	// Used to verify sheep commands are valid.
	std::set<std::string> mCharacterNouns;
	
	// Character configs, keyed by the 3-letter character identifier.
	std::unordered_map<std::string, CharacterConfig> mCharacterConfigs;
	
	// A default character config, in case you request one that doesn't exist.
	CharacterConfig mDefaultCharacterConfig;
};
