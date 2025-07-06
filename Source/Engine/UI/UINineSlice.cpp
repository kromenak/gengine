#include "UINineSlice.h"

#include "Actor.h"
#include "UIImage.h"

TYPEINFO_INIT(UINineSlice, UIWidget, 23)
{

}

UINineSlice::UINineSlice(Actor* actor) : UIWidget(actor)
{
    // Create center.
    mCenterImage = actor->AddComponent<UIImage>();

    // Create top/bottom sides.
    {
        Actor* top = new Actor("T", TransformType::RectTransform);
        top->GetTransform()->SetParent(actor->GetTransform());

        UIImage* topImage = top->AddComponent<UIImage>();
        topImage->GetRectTransform()->SetAnchor(AnchorPreset::TopStretch);
        topImage->GetRectTransform()->SetPivot(0.0f, 1.0f);
        topImage->SetRenderMode(UIImage::RenderMode::Tiled);
        mTopImage = topImage;
    }
    {
        Actor* bottom = new Actor("B", TransformType::RectTransform);
        bottom->GetTransform()->SetParent(actor->GetTransform());

        UIImage* bottomImage = bottom->AddComponent<UIImage>();
        bottomImage->GetRectTransform()->SetAnchor(AnchorPreset::BottomStretch);
        bottomImage->GetRectTransform()->SetPivot(0.0f, 0.0f);
        bottomImage->SetRenderMode(UIImage::RenderMode::Tiled);
        mBottomImage = bottomImage;
    }

    // Create left/right sides.
    {
        Actor* left = new Actor("L", TransformType::RectTransform);
        left->GetTransform()->SetParent(actor->GetTransform());

        UIImage* leftImage = left->AddComponent<UIImage>();
        leftImage->GetRectTransform()->SetAnchor(AnchorPreset::LeftStretch);
        leftImage->GetRectTransform()->SetPivot(0.0f, 0.0f);
        leftImage->SetRenderMode(UIImage::RenderMode::Tiled);
        mLeftImage = leftImage;
    }
    {
        Actor* right = new Actor("R", TransformType::RectTransform);
        right->GetTransform()->SetParent(actor->GetTransform());

        UIImage* rightImage = right->AddComponent<UIImage>();
        rightImage->GetRectTransform()->SetAnchor(AnchorPreset::RightStretch);
        rightImage->GetRectTransform()->SetPivot(1.0f, 0.0f);
        rightImage->SetRenderMode(UIImage::RenderMode::Tiled);
        mRightImage = rightImage;
    }

    // Create four corners.
    {
        Actor* topLeft = new Actor("TL", TransformType::RectTransform);
        topLeft->GetTransform()->SetParent(actor->GetTransform());

        UIImage* topLeftImage = topLeft->AddComponent<UIImage>();
        topLeftImage->GetRectTransform()->SetAnchor(AnchorPreset::TopLeft);
        mTopLeftImage = topLeftImage;
    }
    {
        Actor* topRight = new Actor("TR", TransformType::RectTransform);
        topRight->GetTransform()->SetParent(actor->GetTransform());

        UIImage* topRightImage = topRight->AddComponent<UIImage>();
        topRightImage->GetRectTransform()->SetAnchor(AnchorPreset::TopRight);
        mTopRightImage = topRightImage;
    }
    {
        Actor* bottomRight = new Actor("BR", TransformType::RectTransform);
        bottomRight->GetTransform()->SetParent(actor->GetTransform());

        UIImage* bottomRightImage = bottomRight->AddComponent<UIImage>();
        bottomRightImage->GetRectTransform()->SetAnchor(AnchorPreset::BottomRight);
        mBottomRightImage = bottomRightImage;
    }
    {
        Actor* bottomLeft = new Actor("BL", TransformType::RectTransform);
        bottomLeft->GetTransform()->SetParent(actor->GetTransform());

        UIImage* bottomLeftImage = bottomLeft->AddComponent<UIImage>();
        bottomLeftImage->GetRectTransform()->SetAnchor(AnchorPreset::BottomLeft);
        mBottomLeftImage = bottomLeftImage;
    }
}

UINineSlice::UINineSlice(Actor* actor, const UINineSliceParams& params) : UINineSlice(actor)
{
    SetTexturesAndColors(params, true);
}

void UINineSlice::SetTexturesAndColors(const UINineSliceParams& params, bool resizeTextures)
{
    mCenterImage->SetTexture(params.centerTexture, resizeTextures);
    mCenterImage->SetColor(params.centerColor);

    // Because the center image is "center stretch" anchor mode, we want to keep the size delta at (0, 0).
    if(resizeTextures && mCenterImage->GetRectTransform()->GetAnchorMin() == Vector2::Zero && mCenterImage->GetRectTransform()->GetAnchorMax() == Vector3::One)
    {
        mCenterImage->GetRectTransform()->SetSizeDelta(0.0f, 0.0f);
    }

    mTopLeftImage->SetTexture(params.topLeftTexture, resizeTextures);
    mTopLeftImage->SetColor(params.topLeftColor);

    mTopRightImage->SetTexture(params.topRightTexture, resizeTextures);
    mTopRightImage->SetColor(params.topRightColor);

    mBottomRightImage->SetTexture(params.bottomRightTexture, resizeTextures);
    mBottomRightImage->SetColor(params.bottomRightColor);

    mBottomLeftImage->SetTexture(params.bottomLeftTexture, resizeTextures);
    mBottomLeftImage->SetColor(params.bottomLeftColor);

    mLeftImage->SetTexture(params.leftTexture, resizeTextures);
    mLeftImage->SetColor(params.leftColor);

    mRightImage->SetTexture(params.rightTexture, resizeTextures);
    mRightImage->SetColor(params.rightColor);

    mBottomImage->SetTexture(params.bottomTexture, resizeTextures);
    mBottomImage->SetColor(params.bottomColor);

    mTopImage->SetTexture(params.topTexture, resizeTextures);
    mTopImage->SetColor(params.topColor);

    if(params.borderWidth > 0 && resizeTextures)
    {
        mTopImage->GetRectTransform()->SetSizeDeltaY(params.borderWidth);
        mBottomImage->GetRectTransform()->SetSizeDeltaY(params.borderWidth);

        mLeftImage->GetRectTransform()->SetSizeDeltaX(params.borderWidth);
        mRightImage->GetRectTransform()->SetSizeDeltaX(params.borderWidth);

        mTopLeftImage->GetRectTransform()->SetSizeDelta(params.borderWidth, params.borderWidth);
        mTopRightImage->GetRectTransform()->SetSizeDelta(params.borderWidth, params.borderWidth);
        mBottomRightImage->GetRectTransform()->SetSizeDelta(params.borderWidth, params.borderWidth);
        mBottomLeftImage->GetRectTransform()->SetSizeDelta(params.borderWidth, params.borderWidth);
    }

    // When setting the textures, we resize the RectTransform to match the texture size.
    // But for stretched RectTransforms, this sets the stretched axis to an undesirable value. Reset to zero to fix.
    mLeftImage->GetRectTransform()->SetSizeDeltaY(0.0f);
    mRightImage->GetRectTransform()->SetSizeDeltaY(0.0f);
    mBottomImage->GetRectTransform()->SetSizeDeltaX(0.0f);
    mTopImage->GetRectTransform()->SetSizeDeltaX(0.0f);
}