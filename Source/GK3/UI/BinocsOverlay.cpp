#include "BinocsOverlay.h"

#include "AssetManager.h"
#include "GameCamera.h"
#include "Texture.h"
#include "Scene.h"
#include "SceneManager.h"
#include "UIButton.h"
#include "UICanvas.h"
#include "UIImage.h"
#include "UIUtil.h"

BinocsOverlay::BinocsOverlay() : Actor("BinocsOverlay", TransformType::RectTransform)
{
    // Add canvas to render UI elements.
    AddComponent<UICanvas>(1);

    // Add base binocs image with cutout center.
    UIImage* baseImage = UIUtil::NewUIActorWithWidget<UIImage>(this);
    baseImage->SetTexture(gAssetManager.LoadTexture("BINOCMASK.BMP"), true);
    baseImage->SetReceivesInput(true);

    // Add arrow control image at bottom-center.
    UIImage* arrowsImage = UIUtil::NewUIActorWithWidget<UIImage>(baseImage->GetOwner());
    arrowsImage->SetTexture(gAssetManager.LoadTexture("BINOCBTNAREA.BMP"), true);
    arrowsImage->GetRectTransform()->SetAnchor(AnchorPreset::Bottom);
    arrowsImage->GetRectTransform()->SetAnchoredPosition(0.0f, 19.0f);

    // Add buttons for each arrow direction.
    {
        mUpButton = UIUtil::NewUIActorWithWidget<UIButton>(arrowsImage->GetOwner());
        mUpButton->SetUpTexture(gAssetManager.LoadTexture("BINOCBTNUPU.BMP"));
        mUpButton->SetDownTexture(gAssetManager.LoadTexture("BINOCBTNUPD.BMP"));
        mUpButton->GetRectTransform()->SetAnchor(AnchorPreset::BottomLeft);
        mUpButton->GetRectTransform()->SetAnchoredPosition(44.0f, 67.0f);
    }
    {
        mDownButton = UIUtil::NewUIActorWithWidget<UIButton>(arrowsImage->GetOwner());
        mDownButton->SetUpTexture(gAssetManager.LoadTexture("BINOCBTNDOWNU.BMP"));
        mDownButton->SetDownTexture(gAssetManager.LoadTexture("BINOCBTNDOWND.BMP"));
        mDownButton->GetRectTransform()->SetAnchor(AnchorPreset::BottomLeft);
        mDownButton->GetRectTransform()->SetAnchoredPosition(44.0f, 3.0f);
    }
    {
        mRightButton = UIUtil::NewUIActorWithWidget<UIButton>(arrowsImage->GetOwner());
        mRightButton->SetUpTexture(gAssetManager.LoadTexture("BINOCBTNRIGHTU.BMP"));
        mRightButton->SetDownTexture(gAssetManager.LoadTexture("BINOCBTNRIGHTD.BMP"));
        mRightButton->GetRectTransform()->SetAnchor(AnchorPreset::BottomLeft);
        mRightButton->GetRectTransform()->SetAnchoredPosition(65.0f, 46.0f);
    }
    {
        mLeftButton = UIUtil::NewUIActorWithWidget<UIButton>(arrowsImage->GetOwner());
        mLeftButton->SetUpTexture(gAssetManager.LoadTexture("BINOCBTNLEFTU.BMP"));
        mLeftButton->SetDownTexture(gAssetManager.LoadTexture("BINOCBTNLEFTD.BMP"));
        mLeftButton->GetRectTransform()->SetAnchor(AnchorPreset::BottomLeft);
        mLeftButton->GetRectTransform()->SetAnchoredPosition(2.0f, 46.0f);
    }

    // Add exit button.
    {
        UIButton* exitButton = UIUtil::NewUIActorWithWidget<UIButton>(baseImage->GetOwner());
        exitButton->SetUpTexture(gAssetManager.LoadTexture("BINOCBTNEXITU.BMP"));
        exitButton->SetHoverTexture(gAssetManager.LoadTexture("BINOCBTNEXITHOV.BMP"));
        exitButton->SetDownTexture(gAssetManager.LoadTexture("BINOCBTNEXITD.BMP"));
        exitButton->SetDisabledTexture(gAssetManager.LoadTexture("BINOCBTNEXITDIS.BMP"));
        exitButton->GetRectTransform()->SetAnchor(AnchorPreset::BottomLeft);
        exitButton->GetRectTransform()->SetAnchoredPosition(95.0f, 62.0f);
        exitButton->SetPressCallback([this](UIButton* button){
            Hide();
        });
    }

    // Add zoom 50x and 5x buttons.
    {
        UIButton* zoomInButton = UIUtil::NewUIActorWithWidget<UIButton>(baseImage->GetOwner());
        zoomInButton->SetUpTexture(gAssetManager.LoadTexture("BINOCBTNZOOMINU.BMP"));
        zoomInButton->SetHoverTexture(gAssetManager.LoadTexture("BINOCBTNZOOMINHOV.BMP"));
        zoomInButton->SetDownTexture(gAssetManager.LoadTexture("BINOCBTNZOOMIND.BMP"));
        zoomInButton->SetDisabledTexture(gAssetManager.LoadTexture("BINOCBTNZOOMINDIS.BMP"));
        zoomInButton->GetRectTransform()->SetAnchor(AnchorPreset::BottomRight);
        zoomInButton->GetRectTransform()->SetAnchoredPosition(-95.0f, 62.0f);
    }
    {
        UIButton* zoomOutButton = UIUtil::NewUIActorWithWidget<UIButton>(baseImage->GetOwner());
        zoomOutButton->SetUpTexture(gAssetManager.LoadTexture("BINOCBTNZOOMOUTU.BMP"));
        zoomOutButton->SetHoverTexture(gAssetManager.LoadTexture("BINOCBTNZOOMOUTHOV.BMP"));
        zoomOutButton->SetDownTexture(gAssetManager.LoadTexture("BINOCBTNZOOMOUTD.BMP"));
        zoomOutButton->SetDisabledTexture(gAssetManager.LoadTexture("BINOCBTNZOOMOUTDIS.BMP"));
        zoomOutButton->GetRectTransform()->SetAnchor(AnchorPreset::BottomRight);
        zoomOutButton->GetRectTransform()->SetAnchoredPosition(-95.0f, 62.0f);
    }

    // Hide by default.
    SetActive(false);
}

void BinocsOverlay::Show()
{
    SetActive(true);

    mGameCamera = gSceneManager.GetScene()->GetCamera();
    mGameCamera->SetSceneActive(false);
    Vector3 camPos = mGameCamera->GetPosition();
    camPos.y += 10000.0f;
    mGameCamera->SetPosition(camPos);

    mGameCamera->SetAngle(Vector2::Zero);
}

void BinocsOverlay::Hide()
{
    SetActive(false);

    mGameCamera->SetSceneActive(true);
    Vector3 camPos = mGameCamera->GetPosition();
    camPos.y -= 10000.0f;
    mGameCamera->SetPosition(camPos);
}

void BinocsOverlay::OnUpdate(float deltaTime)
{
    // Determine if camera angle should change in any direction, depending on which buttons are pressed.
    Vector2 angleChangeDir = Vector2::Zero;
    if(mUpButton->IsPressedDown())
    {
        angleChangeDir.y -= 1.0f;
    }
    if(mDownButton->IsPressedDown())
    {
        angleChangeDir.y += 1.0f;
    }
    if(mLeftButton->IsPressedDown())
    {
        angleChangeDir.x -= 1.0f;
    }
    if(mRightButton->IsPressedDown())
    {
        angleChangeDir.x += 1.0f;
    }

    // Update camera angle.
    // Clamp vertical angle within some bounds.
    mCameraAngle += angleChangeDir * deltaTime;
    mCameraAngle.y = Math::Clamp(mCameraAngle.y, -0.16259, 0.239742);
    mGameCamera->SetAngle(mCameraAngle);

    printf("Camera angle is: (%f, %f)\n", Math::ToDegrees(mCameraAngle.x), Math::ToDegrees(mCameraAngle.y));

    /*
    * Camera angle ranges (degrees) for each focus location.
    *
    * AT CD1:
    *   - PL3: MinX: 52.13f, MaxX: 60.75f, MinY: 0.10f , MaxY: 4.77f
    *   - LHM: MinX: 88.44f, MaxX: 105.63f, MinY: -1.54f, MaxY: 13.73f
    *   - MA3: MinX: 154.48f, MaxX: 168.80f, MinY: -1.54f, MaxY: 11.82f
    *   - CSD: MinX: 90.16f, MaxX: 96.85f, MinY: 7.05f, MaxY: 13.73f
    *
    * At MA3:
    *   - CD1: MinX: 332.02f, MaxX: 350.17f, MinY: -9.31f, MaxY: 5.11f
    */
}

void BinocsOverlay::OnZoomInButtonPressed()
{

}

void BinocsOverlay::OnZoomOutButtonPressed()
{

}
