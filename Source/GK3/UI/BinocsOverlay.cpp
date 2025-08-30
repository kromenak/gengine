#include "BinocsOverlay.h"

#include "ActionManager.h"
#include "AssetManager.h"
#include "BSP.h"
#include "Camera.h"
#include "GameCamera.h"
#include "GameProgress.h"
#include "GEngine.h"
#include "GKPrefs.h"
#include "IniParser.h"
#include "LocationManager.h"
#include "TextAsset.h"
#include "Texture.h"
#include "Scene.h"
#include "SceneManager.h"
#include "SheepManager.h"
#include "UIButton.h"
#include "UICanvas.h"
#include "UIImage.h"
#include "UIUtil.h"
#include "Window.h"

BinocsOverlay::BinocsOverlay() : Actor("BinocsOverlay", TransformType::RectTransform),
    mLayer("BinocsLayer")
{
    // Add canvas to render UI elements.
    mCanvas = UI::AddCanvas(this, 5);

    // Add base binocs image with cutout center.
    mCutoutImage = UI::CreateWidgetActor<UIImage>("Mask", this);
    mCutoutImage->SetTexture(gAssetManager.LoadTexture("BINOCMASK.BMP"), true);
    mCutoutImage->SetReceivesInput(true);

    // Depending on your resolution, there can be a lot of scene visible outside the binocs image.
    // Add black overlays to cover this.
    {
        mTopImage = UI::CreateWidgetActor<UIImage>("TopBG", this);
        mTopImage->SetTexture(&Texture::Black);
        mTopImage->GetRectTransform()->SetAnchoredPosition(0.0f, 240.0f);
        mTopImage->GetRectTransform()->SetPivot(0.5f, 0.0f);
        mTopImage->GetRectTransform()->SetSizeDelta(10000.0f, 10000.0f);
    }
    {
        mBottomImage = UI::CreateWidgetActor<UIImage>("BottomBG", this);
        mBottomImage->SetTexture(&Texture::Black);
        mBottomImage->GetRectTransform()->SetAnchoredPosition(0.0f, -240.0f);
        mBottomImage->GetRectTransform()->SetPivot(0.5f, 1.0f);
        mBottomImage->GetRectTransform()->SetSizeDelta(10000.0f, 10000.0f);
    }
    {
        mLeftImage = UI::CreateWidgetActor<UIImage>("LeftBG", this);
        mLeftImage->SetTexture(&Texture::Black);
        mLeftImage->GetRectTransform()->SetAnchoredPosition(-320.0f, 0.0f);
        mLeftImage->GetRectTransform()->SetPivot(1.0f, 0.5f);
        mLeftImage->GetRectTransform()->SetSizeDelta(10000.0f, 10000.0f);
    }
    {
        mRightImage = UI::CreateWidgetActor<UIImage>("RightBG", this);
        mRightImage->SetTexture(&Texture::Black);
        mRightImage->GetRectTransform()->SetAnchoredPosition(320.0f, 0.0f);
        mRightImage->GetRectTransform()->SetPivot(0.0f, 0.5f);
        mRightImage->GetRectTransform()->SetSizeDelta(10000.0f, 10000.0f);
    }

    // Add arrow control image at bottom-center.
    mArrowButtonsBacking = UI::CreateWidgetActor<UIImage>("Arrows", mCutoutImage);
    mArrowButtonsBacking->SetTexture(gAssetManager.LoadTexture("BINOCBTNAREA.BMP"), true);
    mArrowButtonsBacking->GetRectTransform()->SetAnchor(AnchorPreset::Bottom);
    mArrowButtonsBacking->GetRectTransform()->SetAnchoredPosition(0.0f, 19.0f);

    // Add buttons for each arrow direction.
    {
        mUpButton = UI::CreateWidgetActor<UIButton>("UpButton", mArrowButtonsBacking);
        mUpButton->SetUpTexture(gAssetManager.LoadTexture("BINOCBTNUPU.BMP"));
        mUpButton->SetDownTexture(gAssetManager.LoadTexture("BINOCBTNUPD.BMP"));
        mUpButton->GetRectTransform()->SetAnchor(AnchorPreset::BottomLeft);
        mUpButton->GetRectTransform()->SetAnchoredPosition(44.0f, 67.0f);
    }
    {
        mDownButton = UI::CreateWidgetActor<UIButton>("DownButton", mArrowButtonsBacking);
        mDownButton->SetUpTexture(gAssetManager.LoadTexture("BINOCBTNDOWNU.BMP"));
        mDownButton->SetDownTexture(gAssetManager.LoadTexture("BINOCBTNDOWND.BMP"));
        mDownButton->GetRectTransform()->SetAnchor(AnchorPreset::BottomLeft);
        mDownButton->GetRectTransform()->SetAnchoredPosition(44.0f, 3.0f);
    }
    {
        mRightButton = UI::CreateWidgetActor<UIButton>("RightButton", mArrowButtonsBacking);
        mRightButton->SetUpTexture(gAssetManager.LoadTexture("BINOCBTNRIGHTU.BMP"));
        mRightButton->SetDownTexture(gAssetManager.LoadTexture("BINOCBTNRIGHTD.BMP"));
        mRightButton->GetRectTransform()->SetAnchor(AnchorPreset::BottomLeft);
        mRightButton->GetRectTransform()->SetAnchoredPosition(65.0f, 46.0f);
    }
    {
        mLeftButton = UI::CreateWidgetActor<UIButton>("LeftButton", mArrowButtonsBacking);
        mLeftButton->SetUpTexture(gAssetManager.LoadTexture("BINOCBTNLEFTU.BMP"));
        mLeftButton->SetDownTexture(gAssetManager.LoadTexture("BINOCBTNLEFTD.BMP"));
        mLeftButton->GetRectTransform()->SetAnchor(AnchorPreset::BottomLeft);
        mLeftButton->GetRectTransform()->SetAnchoredPosition(2.0f, 46.0f);
    }

    // Add exit button.
    {
        UIButton* exitButton = UI::CreateWidgetActor<UIButton>("ExitButton", mCutoutImage);
        exitButton->SetUpTexture(gAssetManager.LoadTexture("BINOCBTNEXITU.BMP"));
        exitButton->SetHoverTexture(gAssetManager.LoadTexture("BINOCBTNEXITHOV.BMP"));
        exitButton->SetDownTexture(gAssetManager.LoadTexture("BINOCBTNEXITD.BMP"));
        exitButton->SetDisabledTexture(gAssetManager.LoadTexture("BINOCBTNEXITDIS.BMP"));
        exitButton->GetRectTransform()->SetAnchor(AnchorPreset::TopLeft);
        exitButton->GetRectTransform()->SetAnchoredPosition(95.0f, -389.0f);
        exitButton->SetPressCallback([this](UIButton* button){
            Hide();
        });
        mExitButton = exitButton;
    }

    // Add zoom 50x and 5x buttons.
    {
        UIButton* zoomInButton = UI::CreateWidgetActor<UIButton>("ZoomInButton", mCutoutImage);
        zoomInButton->SetUpTexture(gAssetManager.LoadTexture("BINOCBTNZOOMINU.BMP"));
        zoomInButton->SetHoverTexture(gAssetManager.LoadTexture("BINOCBTNZOOMINHOV.BMP"));
        zoomInButton->SetDownTexture(gAssetManager.LoadTexture("BINOCBTNZOOMIND.BMP"));
        zoomInButton->SetDisabledTexture(gAssetManager.LoadTexture("BINOCBTNZOOMINDIS.BMP"));
        zoomInButton->GetRectTransform()->SetAnchor(AnchorPreset::TopLeft);
        zoomInButton->GetRectTransform()->SetAnchoredPosition(399.0f, -389.0f);
        zoomInButton->SetPressCallback([this](UIButton* button){
            OnZoomInButtonPressed();
        });
        mZoomInButton = zoomInButton;
    }
    {
        UIButton* zoomOutButton = UI::CreateWidgetActor<UIButton>("ZoomOutButton", mCutoutImage);
        zoomOutButton->SetUpTexture(gAssetManager.LoadTexture("BINOCBTNZOOMOUTU.BMP"));
        zoomOutButton->SetHoverTexture(gAssetManager.LoadTexture("BINOCBTNZOOMOUTHOV.BMP"));
        zoomOutButton->SetDownTexture(gAssetManager.LoadTexture("BINOCBTNZOOMOUTD.BMP"));
        zoomOutButton->SetDisabledTexture(gAssetManager.LoadTexture("BINOCBTNZOOMOUTDIS.BMP"));
        zoomOutButton->GetRectTransform()->SetAnchor(AnchorPreset::TopLeft);
        zoomOutButton->GetRectTransform()->SetAnchoredPosition(399.0f, -389.0f);
        zoomOutButton->SetPressCallback([this](UIButton* button){
            OnZoomOutButtonPressed();
        });
        mZoomOutButton = zoomOutButton;
    }

    // Read in Binocs use case data.
    {
        TextAsset* textAsset = gAssetManager.LoadText("BINOCS.TXT", AssetScope::Manual);
        IniParser parser(textAsset->GetText(), textAsset->GetTextLength());
        parser.SetMultipleKeyValuePairsPerLine(false);

        // This file is guaranteed to have its data in a specific order, so we can make some assumptions here.
        bool readMoreData = true;
        while(readMoreData)
        {
            // First, there will always be a "header section" for a specific location/timeblock combination.
            IniSection section;
            readMoreData = parser.ReadNextSection(section);
            if(readMoreData)
            {
                // The key will be the location and timeblock strings concatenated together (e.g. CD1102P).
                // This is what we use to key the use case map.
                UseCase& useCase = mUseCases[section.name];

                // Parse the lines within this use case.
                std::vector<std::string> locs;
                for(auto& line : section.lines)
                {
                    if(StringUtil::EqualsIgnoreCase(line.entries[0].key, "LOC"))
                    {
                        locs = StringUtil::Split(line.entries[0].value, ',', true);
                    }
                    else if(StringUtil::EqualsIgnoreCase(line.entries[0].key, "ANIM"))
                    {
                        useCase.exitAnimName = line.entries[0].value;
                    }
                }

                // Then, there will be X sections following, one for each location in the LOC field of the header section.
                for(int i = 0; i < locs.size(); ++i)
                {
                    readMoreData = parser.ReadNextSection(section);
                    if(!readMoreData) { break; }

                    // The location this section correlates to is embedded within the section name.
                    // We can extract it because all sections use the same naming convention.
                    std::string sceneAssetName = section.name.substr(7);
                    std::string locationCode = sceneAssetName.substr(0, 3);

                    // Create a zoom location entry for this location.
                    ZoomLocation& location = useCase.zoomLocations[locationCode];
                    location.sceneAssetName = sceneAssetName;

                    // Populate from the data in this section.
                    for(auto& line : section.lines)
                    {
                        if(StringUtil::EqualsIgnoreCase(line.entries[0].key, "CAMANGLE"))
                        {
                            location.cameraAngle = line.entries[0].GetValueAsVector2();
                            location.cameraAngle.x = Math::ToRadians(location.cameraAngle.x);
                            location.cameraAngle.y = Math::ToRadians(location.cameraAngle.y);
                        }
                        else if(StringUtil::EqualsIgnoreCase(line.entries[0].key, "CAMPOS"))
                        {
                            location.cameraPos = line.entries[0].GetValueAsVector3();
                        }
                        else if(StringUtil::EqualsIgnoreCase(line.entries[0].key, "FLOOR"))
                        {
                            location.floorModelName = line.entries[0].value;
                        }
                        else if(StringUtil::EqualsIgnoreCase(line.entries[0].key, "ENTERSHEEP"))
                        {
                            location.enterSheepFunctionName = line.entries[0].value;

                            // Sheep functions are supposed to end with a $ symbol.
                            if(!location.enterSheepFunctionName.empty() && location.enterSheepFunctionName.back() != '$')
                            {
                                location.enterSheepFunctionName.push_back('$');
                            }
                        }
                        else if(StringUtil::EqualsIgnoreCase(line.entries[0].key, "EXITSHEEP"))
                        {
                            location.exitSheepFunctionName = line.entries[0].value;

                            // Sheep functions are supposed to end with a $ symbol.
                            if(!location.exitSheepFunctionName.empty() && location.exitSheepFunctionName.back() != '$')
                            {
                                location.exitSheepFunctionName.push_back('$');
                            }
                        }
                    }
                }
            }
        }

        // We're done with this text asset.
        delete textAsset;
    }

    // Load binocs sheep script.
    mBinocsScript = gAssetManager.LoadSheep("BINOCS.SHP");

    // Hide by default.
    SetActive(false);
}

void BinocsOverlay::Show()
{
    SetActive(true);
    gLayerManager.PushLayer(&mLayer);

    RefreshUIScaling();

    // HACK: Scene layer should not pause when binocs layer is up.
    gSceneManager.GetScene()->SetPaused(false);

    // The binocs operate using the normal game camera in the current scene, but perform some trickery to make things look right.
    mGameCamera = gSceneManager.GetScene()->GetCamera();

    // Set the disabled scene flag in the camera, which stops normal camera movement code from running.
    mGameCamera->SetSceneActive(false);

    // Raise the camera up really high in the sky, so it doesn't show any objects in the current scene.
    // This works because the skybox looks the same regardless of position.
    Vector3 camPos = mGameCamera->GetPosition();
    camPos.y += 10000.0f;
    mGameCamera->SetPosition(camPos);

    // The binocs start at whichever angle the game camera was using.
    // This is regardless of the direction Gabe/Grace are facing when they start using the binocs!
    mZoomedOutCameraAngle = mGameCamera->GetAngle();

    // Figure out the use case from the current location and timeblock.
    auto it = mUseCases.find(gLocationManager.GetLocation() + gGameProgress.GetTimeblock().ToString());
    if(it != mUseCases.end())
    {
        mCurrentUseCase = &it->second;
    }
    else
    {
        printf("ERROR: Using binocs from an invalid location or timeblock\n");
        Hide();
    }

    // Not zoomed in for starters.
    mIsZoomedIn = false;
}

void BinocsOverlay::Hide()
{
    if(!IsActive()) { return; }
    SetActive(false);
    gLayerManager.PopLayer(&mLayer);

    // Move the game camera back down to the ground, undoing the upward move when we shows the binocs interface.
    Vector3 camPos = mGameCamera->GetPosition();
    camPos.y -= 10000.0f;
    mGameCamera->SetPosition(camPos);

    // We do not modify the camera angle at all.
    // Whichever angle the binocs are at when you exit is what you stay at when exiting the binocs.

    // Re-enable scene inputs for the camera.
    mGameCamera->SetSceneActive(true);

    // Play animation to put away binocs when exiting this screen.
    // Without this, Gabe/Grace just continue to stand with the binocs up to their faces!
    if(mCurrentUseCase != nullptr)
    {
        // Do this in an action so the player can skip if they want.
        gActionManager.ExecuteSheepAction(StringUtil::Format("wait StartAnimation(\"%s\")", mCurrentUseCase->exitAnimName.c_str()));
    }
}

void BinocsOverlay::OnUpdate(float deltaTime)
{
    // Show/hide zoom in/out buttons as appropriate.
    mZoomInButton->SetEnabled(!mIsZoomedIn);
    mZoomOutButton->SetEnabled(mIsZoomedIn);

    // Exit button is disabled when zoomed in.
    mExitButton->SetCanInteract(!mIsZoomedIn);

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
    if(mIsZoomedIn)
    {
        // Clamp vertical angle within some bounds.
        mZoomedInCameraAngle += angleChangeDir * deltaTime;
        mZoomedInCameraAngle.x = Math::Clamp(mZoomedInCameraAngle.x, mZoomedInCameraAngleLimits.minAngles.x, mZoomedInCameraAngleLimits.maxAngles.x);
        mZoomedInCameraAngle.y = Math::Clamp(mZoomedInCameraAngle.y, mZoomedInCameraAngleLimits.minAngles.y, mZoomedInCameraAngleLimits.maxAngles.y);
        mGameCamera->SetAngle(mZoomedInCameraAngle);
    }
    else
    {
        // Clamp vertical angle within some bounds.
        mZoomedOutCameraAngle += angleChangeDir * deltaTime;
        mZoomedOutCameraAngle.y = Math::Clamp(mZoomedOutCameraAngle.y, -0.16259f, 0.239742f);
        //printf("Camera angle is: (%f, %f)\n", Math::ToDegrees(mZoomedOutCameraAngle.x), Math::ToDegrees(mZoomedOutCameraAngle.y));
        mGameCamera->SetAngle(mZoomedOutCameraAngle);
    }

    // When not zoomed in, the zoom in button is pressable only when pointing at a zoom location.
    if(!mIsZoomedIn)
    {
        // Convert angles to degrees.
        float camAngleDegX = Math::ToDegrees(mZoomedOutCameraAngle.x);
        float camAngleDegY = Math::ToDegrees(mZoomedOutCameraAngle.y);

        // Ensure angle is within 0-360 range, since that's the range the zoom in rects are defined within.
        camAngleDegX = Math::Mod(camAngleDegX, 360.0f);
        while(camAngleDegX < 0.0f)
        {
            camAngleDegX += 360.0f;
        }

        // Figure out if the current camera angle is within the min/max of any locations we can zoom to from this location.
        // If so, we'll save the zoom location code.
        mCamZoomToLocCode.clear();
        auto& map = mZoomAngles[gLocationManager.GetLocation()];
        for(auto& entry : map)
        {
            if(camAngleDegX >= entry.second.minAngles.x &&
               camAngleDegX <= entry.second.maxAngles.x &&
               camAngleDegY >= entry.second.minAngles.y &&
               camAngleDegY <= entry.second.maxAngles.y)
            {
                mCamZoomToLocCode = entry.first;
            }
        }

        // If this loc code isn't present in the current use case, we can't zoom there.
        if(mCurrentUseCase != nullptr && mCurrentUseCase->zoomLocations.find(mCamZoomToLocCode) == mCurrentUseCase->zoomLocations.end())
        {
            mCamZoomToLocCode.clear();
        }

        // Zoom in button is interactable if we have a location we could zoom to.
        mZoomInButton->SetCanInteract(!mCamZoomToLocCode.empty());
    }

    // Keep UI scaled correctly if resolution changes.
    RefreshUIScaling();
}

void BinocsOverlay::OnZoomInButtonPressed()
{
    // Get the zoom location info.
    auto it = mCurrentUseCase->zoomLocations.find(mCamZoomToLocCode);
    if(it == mCurrentUseCase->zoomLocations.end())
    {
        printf("ERROR: no info on how to zoom to %s exists for the current location/timeblock.\n", mCamZoomToLocCode.c_str());
        return;
    }
    mCurrentZoomLocation = &it->second;

    // Remember the camera's current position, for when we zoom back out.
    mZoomedOutCameraPos = mGameCamera->GetPosition();

    // Temporarily change the BSP used to that of the zoomed location.
    gSceneManager.GetScene()->OverrideSceneAsset(mCurrentZoomLocation->sceneAssetName, mCurrentZoomLocation->floorModelName);

    // Update the camera position and angle for this zoom location.
    mGameCamera->SetPosition(mCurrentZoomLocation->cameraPos);
    mGameCamera->SetAngle(mCurrentZoomLocation->cameraAngle);
    mZoomedInCameraAngle = mCurrentZoomLocation->cameraAngle;

    // When zoomed in, the range of rotation on the camera is pretty low.
    // These values aren't derived from the original game, but they seem to mimic the behavior well enough.
    const Vector2 kZoomedInAngleRange(0.5f, 0.20f);
    mZoomedInCameraAngleLimits.minAngles = mZoomedInCameraAngle - kZoomedInAngleRange;
    mZoomedInCameraAngleLimits.maxAngles = mZoomedInCameraAngle + kZoomedInAngleRange;

    // Zoomed in views always show at 30 degree FOV.
    mSavedCameraFov = mGameCamera->GetCamera()->GetCameraFovDegrees();
    mGameCamera->GetCamera()->SetCameraFovDegrees(30.0f);

    // Similar to action skip, if we're zooming in, but a zoom in Sheepscript is still playing, fast forward to finish it before moving on!
    while(gSheepManager.IsThreadRunning(mZoomInSheepThreadId))
    {
        GEngine::Instance()->UpdateGameWorld(10.0f);
    }

    // BINOCS.SHP *may* also have an "all" version of the enter script. This should be called if it exists.
    std::string allFuncName = gLocationManager.GetLocation() + "all" + mCamZoomToLocCode + "ent$";
    mZoomInSheepThreadId = gSheepManager.Execute(mBinocsScript, allFuncName, [this](){
        if(!mCurrentZoomLocation->enterSheepFunctionName.empty())
        {
            mZoomInSheepThreadId = gSheepManager.Execute(mBinocsScript, mCurrentZoomLocation->enterSheepFunctionName, nullptr);
        }
    });

    // We are zoomed in!
    printf("Zoom to %s\n", mCamZoomToLocCode.c_str());
    mIsZoomedIn = true;
}

void BinocsOverlay::OnZoomOutButtonPressed()
{
    // Go back to the normal scene BSP.
    gSceneManager.GetScene()->ClearSceneAssetOverride();

    // Set camera back to the zoomed out position and angle.
    mGameCamera->SetPosition(mZoomedOutCameraPos);
    mGameCamera->SetAngle(mZoomedOutCameraAngle);

    // Go back to previous FOV.
    mGameCamera->GetCamera()->SetCameraFovDegrees(mSavedCameraFov);

    // Perform some logic specific to the zoomed location.
    if(mCurrentZoomLocation != nullptr)
    {
        // Play the exit sheep.
        if(!mCurrentZoomLocation->exitSheepFunctionName.empty())
        {
            gSheepManager.Execute(mBinocsScript, mCurrentZoomLocation->exitSheepFunctionName, nullptr);
        }
    }

    // No longer zoomed in.
    mIsZoomedIn = false;
}

void BinocsOverlay::RefreshUIScaling()
{
    Vector2 bgImageSize(640.0f, 480.0f);

    // The original game actually does scale this UI up to match the current resolution.
    // The logic is similar to the title screen, though the button logic differs.
    if(Prefs::UseOriginalUIScalingLogic() && Window::GetHeight() <= Prefs::GetMinimumScaleUIHeight())
    {
        // Turn off canvas autoscaling. This sets canvas scale to 1, and width/height equal to window width/height.
        mCanvas->SetAutoScale(false);

        // Resize background image to fit within window size, preserving aspect ratio.
        mCutoutImage->ResizeToFitPreserveAspect(Window::GetSize());

        // The background image size is now whatever was calculated.
        bgImageSize = mCutoutImage->GetRectTransform()->GetSizeDelta();
    }
    else // not using original game's logic.
    {
        // In this case, just use 640x480 and have it auto-scale when the resolution gets too big.
        mCanvas->SetAutoScale(true);
        mCutoutImage->ResizeToTexture();
    }

    // The images around the cutout need to reposition as the cutout size changes.
    float halfWidth = bgImageSize.x / 2.0f;
    float halfHeight = bgImageSize.y / 2.0f;
    mBottomImage->GetRectTransform()->SetAnchoredPosition(0.0f, -halfHeight);
    mTopImage->GetRectTransform()->SetAnchoredPosition(0.0f, halfHeight);
    mLeftImage->GetRectTransform()->SetAnchoredPosition(-halfWidth, 0.0f);
    mRightImage->GetRectTransform()->SetAnchoredPosition(halfWidth, 0.0f);

    // The arrow buttons y-position changes with the screen size.
    float arrowButtonsY = 19.0f + (bgImageSize.y - 480.0f) * 0.18f;
    mArrowButtonsBacking->GetRectTransform()->SetAnchoredPosition(0.0f, arrowButtonsY);

    // The exit/zoom buttons also change position. But their logic is a bit more complex.
    {
        // First, calculate y-pos and distance between buttons.
        float sideButtonsY = 63.0f + (bgImageSize.y - 480.0f) * 0.18f;
        float distBetweenButtons = Math::RoundToInt(Math::Min(30.0f + (bgImageSize.x - 640.0f) * 0.15f, 100.0f));

        // The complexity here is that these buttons are top-left anchored (they don't look right otherwise).
        // So first, let's calculate their top-left corner positions relative to bottom center.
        float arrowBackingHalfWidth = mArrowButtonsBacking->GetRectTransform()->GetSizeDelta().x / 2.0f;
        float buttonWidth = mExitButton->GetRectTransform()->GetSizeDelta().x;
        float buttonHeight = mExitButton->GetRectTransform()->GetSizeDelta().y;
        Vector3 exitButtonPos(-arrowBackingHalfWidth - distBetweenButtons - buttonWidth, sideButtonsY + buttonHeight);
        Vector3 zoomButtonPos(arrowBackingHalfWidth + distBetweenButtons, sideButtonsY + buttonHeight);

        // Then, we can convert the calculated position (relative to bottom center) to be relative to top-left.
        Vector3 bottomToTopRight(-halfWidth, bgImageSize.y);
        mExitButton->GetRectTransform()->SetAnchoredPosition(exitButtonPos - bottomToTopRight);
        mZoomInButton->GetRectTransform()->SetAnchoredPosition(zoomButtonPos - bottomToTopRight);
        mZoomOutButton->GetRectTransform()->SetAnchoredPosition(zoomButtonPos - bottomToTopRight);
    }
}