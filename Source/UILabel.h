//
// UILabel.h
//
// Clark Kromenaker
//
// A UI element that displays some text.
//
#pragma once
#include "UIWidget.h"

#include "Material.h"

class Font;
class Mesh;

class UILabel : public UIWidget
{
	TYPE_DECL_CHILD();
public:
	UILabel(Actor* owner);
	
	void Render() override;
	
	void SetText(std::string text);
	void SetFont(Font* font);
	
private:
	// The text to be displayed by the label.
	std::string mText;
	
	// The font used to display the label.
	Font* mFont = nullptr;
	
	// Material used for rendering.
	Material mMaterial;
	
	// Mesh used for rendering.
	// This is generated from the desired text before rendering.
	Mesh* mMesh = nullptr;
	bool mNeedMeshRegen = false;
	
	void GenerateMesh();
};
