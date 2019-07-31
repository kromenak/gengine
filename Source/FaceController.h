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

class Texture;

class FaceController : public Component
{
	TYPE_DECL_CHILD();
public:
	FaceController(Actor* owner);
	
protected:
	void UpdateInternal(float deltaTime) override;
	
private:
	Texture* mFaceTexture = nullptr;
	Texture* mForeheadTexture = nullptr;
	Texture* mEyelidsTexture = nullptr;
	
	Texture* mLeftEyeTexture = nullptr;
	Texture* mRightEyeTexture = nullptr;
	
};
