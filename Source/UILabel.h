//
// UILabel.h
//
// Clark Kromenaker
//
// A UI element that displays some text.
//
#pragma once
#include "UIWidget.h"

#include <string>
#include <vector>

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
	Font* GetFont() const { return mFont; }
	
	void SetText(std::string text);
	std::string GetText() const { return mText; }
	
	void SetLineSpacing(int spacing);
	
protected:
	Vector2 GetGlyphPos(int index) const;
	const Vector2& GetNextGlyphPos() const { return mNextGlyphPos; }
	
private:
	// The font used to display the label.
	Font* mFont = nullptr;
	
	// The text to be displayed by the label.
	std::string mText;
	
	// Extra space to be added between each line.
	int mLineSpacing = 0;
	
	// The position that the next glyph would be positioned, if there was a next glyph.
	// This is primarily helpful to track for positioning a caret, in the case of a text input field.
	std::vector<Vector2> mGlyphPositions;
	Vector2 mNextGlyphPos;
	
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
