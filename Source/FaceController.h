//
// FaceController.h
//
// Clark Kromenaker
//
// Controls facial elements, such as eye movement, blinking, lip movement,
// and expressions, etc.
//
#pragma once
#include "Component.h"

class CharacterConfig;
class Texture;

enum class FaceElement
{
	Forehead,
	Eyelids,
	Mouth
};

class FaceController : public Component
{
	TYPE_DECL_CHILD();
public:
	FaceController(Actor* owner);
	void SetCharacterConfig(const CharacterConfig& characterConfig);
	
	void Set(FaceElement element, Texture* texture);
	void Clear(FaceElement element);
	
	void SetMouth(Texture* texture);
	void ClearMouth();
	
	void SetEyelids(Texture* texture);
	void ClearEyelids();
	
	void SetForehead(Texture* texture);
	void ClearForehead();
	
	void Blink();
	
protected:
	void UpdateInternal(float deltaTime) override;
	
private:
	const CharacterConfig* mCharacterConfig = nullptr;
	
	Texture* mDefaultMouthTexture = nullptr;
	Texture* mDefaultEyelidsTexture = nullptr;
	Texture* mDefaultForeheadTexture = nullptr;
	
	float mBlinkTimer = 0.0f;
	
	void RollBlinkTimer();
};
