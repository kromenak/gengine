//
// Clark Kromenaker
//
// A UI element that displays some text.
//
#pragma once
#include "UIWidget.h"

#include <string>

#include "Color32.h"
#include "Material.h"
#include "TextLayout.h"
#include "Vector2.h"

class Font;
class Mesh;

class UILabel : public UIWidget
{
    TYPEINFO_SUB(UILabel, UIWidget);
public:
    UILabel(Actor* owner);
    ~UILabel();

    void Render() override;

    void SetFont(Font* font);
    Font* GetFont() const { return mFont; }

    void SetColor(const Color32& color);
    Color32 GetColor() const { return mColor; }

    void SetHorizonalAlignment(HorizontalAlignment ha) { mHorizontalAlignment = ha; }
    void SetVerticalAlignment(VerticalAlignment va) { mVerticalAlignment = va; }

    void SetHorizontalOverflow(HorizontalOverflow ho) { mHorizontalOverflow = ho; }
    void SetVerticalOverflow(VerticalOverflow vo) { mVerticalOverflow = vo; }

    void SetText(const std::string& text);
    const std::string& GetText() const { return mText; }

    void SetMasked(bool masked) { mMask = masked; }

    int GetLineCount();
    float GetTextWidth();
    float GetTextHeight();
    void FitRectTransformToText();

    Vector2 GetCharPos(int index) const;
    Vector2 GetNextCharPos() const { return mTextLayout.GetNextCharPos(); }

    void SetDirty() override { mNeedMeshRegen = true; }

protected:
    virtual void PopulateTextLayout(TextLayout& textLayout);

    void GenerateMesh();

private:
    // The font used to display the label.
    Font* mFont = nullptr;

    // Desired text alignment within the transform's rect.
    HorizontalAlignment mHorizontalAlignment = HorizontalAlignment::Left;
    VerticalAlignment mVerticalAlignment = VerticalAlignment::Bottom;

    // Desired word wrap and overflow settings.
    HorizontalOverflow mHorizontalOverflow = HorizontalOverflow::Overflow;
    VerticalOverflow mVerticalOverflow = VerticalOverflow::Overflow;

    // If true, the text mesh will be clamped based on the size of the label's rect.
    // Anything outside that rect will simply be cut off.
    bool mMask = false;

    // The text to be displayed by the label.
    std::string mText;

    // Helper for laying out text within the available space with desired alignment/overflow.
    TextLayout mTextLayout;

    // Text color.
    Color32 mColor = Color32::White;

    // Material used for rendering.
    Material mMaterial;

    // Mesh used for rendering.
    // This is generated from the desired text before rendering.
    Mesh* mMesh = nullptr;
    bool mNeedMeshRegen = false;
};
