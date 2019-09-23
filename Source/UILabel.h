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
	enum class HorizontalAlignment
	{
		Left,
		Right,
		//Center
	};
	
	enum class VerticalAlignment
	{
		Bottom,
		Top,
		//Center
	};
	
	enum class HorizontalOverflow
	{
		Wrap,
		Overflow
	};
	
	enum class VerticalOverflow
	{
		Truncate,
		Overflow
	};
	
	UILabel(Actor* owner);
	
	void Render() override;
	
	void SetFont(Font* font);
	
	void SetText(std::string text);
	std::string GetText() const { return mText; }
	
private:
	// The text to be displayed by the label.
	std::string mText;
	
	// The font used to display the label.
	Font* mFont = nullptr;
	
	// Desired text alignment within the transform's rect.
	HorizontalAlignment mHorizontalAlignment = HorizontalAlignment::Left;
	VerticalAlignment mVerticalAlignment = VerticalAlignment::Bottom;
	
	// Material used for rendering.
	Material mMaterial;
	
	// Mesh used for rendering.
	// This is generated from the desired text before rendering.
	Mesh* mMesh = nullptr;
	bool mNeedMeshRegen = false;
	
	void GenerateMesh();
};
