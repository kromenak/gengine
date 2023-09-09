//
// Clark Kromenaker
//
// A set of UI images that resize to create a flexible bordered area.
// 
// Called "nine-slice" because it's a single image sliced into 9 parts.
// And some of the parts resize dynamically to fill the space.
//
#pragma once
#include "Color32.h"
#include "UIWidget.h"

class Texture;
class UIImage;

// A way to define a "preset" for all the textures/colors to use for a nine-slice config.
struct UINineSliceParams
{
    Texture* centerTexture = nullptr;
    Color32  centerColor = Color32::White;

    Texture* topLeftTexture = nullptr;
    Color32  topLeftColor = Color32::White;

    Texture* topRightTexture = nullptr;
    Color32  topRightColor = Color32::White;

    Texture* bottomRightTexture = nullptr;
    Color32  bottomRightColor = Color32::White;

    Texture* bottomLeftTexture = nullptr;
    Color32  bottomLeftColor = Color32::White;

    Texture* leftTexture = nullptr;
    Color32  leftColor = Color32::White;

    Texture* rightTexture = nullptr;
    Color32  rightColor = Color32::White;

    Texture* bottomTexture = nullptr;
    Color32  bottomColor = Color32::White;

    Texture* topTexture = nullptr;
    Color32  topColor = Color32::White;
};

class UINineSlice : public UIWidget
{
    TYPE_DECL_CHILD();
public:
    UINineSlice(Actor* actor);
    UINineSlice(Actor* actor, const UINineSliceParams& params);

    void Render() override { }

    void SetTexturesAndColors(const UINineSliceParams& params);

private:
    // All nine of the images that make up the nine-slice.
    UIImage* mCenterImage = nullptr;

    UIImage* mTopLeftImage = nullptr;
    UIImage* mTopRightImage = nullptr;
    UIImage* mBottomRightImage = nullptr;
    UIImage* mBottomLeftImage = nullptr;

    UIImage* mLeftImage = nullptr;
    UIImage* mRightImage = nullptr;

    UIImage* mBottomImage = nullptr;
    UIImage* mTopImage = nullptr;
};