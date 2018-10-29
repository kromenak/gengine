//
// UIButton.h
//
// Clark Kromenaker
//
// A UI element that can be touched/clicked to execute some action.
//
#pragma once
#include "UIWidget.h"

#include "Material.h"

class Texture;

class UIButton : public UIWidget
{
	TYPE_DECL_CHILD();
public:
	UIButton(Actor* actor);
	
	void Render() override;
	
	void SetUpTexture(Texture* texture) { mUpTexture = texture; }
	void SetDownTexture(Texture* texture) { mDownTexture = texture; }
	void SetHoverTexture(Texture* texture) { mHoverTexture = texture; }
	void SetDisabledTexture(Texture* texture) { mDisabledTexture = texture; }
	
	void Enable() { mEnabled = true; }
	void Disable() { mEnabled = false; }
	
private:
	bool mEnabled = true;
	
	Texture* mUpTexture = nullptr;
	Texture* mDownTexture = nullptr;
	Texture* mHoverTexture = nullptr;
	Texture* mDisabledTexture = nullptr;
	
	Material mMaterial;
	
	Texture* GetDefaultTexture();
};
