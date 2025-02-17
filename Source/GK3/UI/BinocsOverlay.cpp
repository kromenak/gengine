#include "BinocsOverlay.h"

#include "Animator.h"
#include "AssetManager.h"
#include "BSP.h"
#include "GameCamera.h"
#include "GameProgress.h"
#include "IniParser.h"
#include "LocationManager.h"
#include "Renderer.h"
#include "TextAsset.h"
#include "Texture.h"
#include "Scene.h"
#include "SceneGeometryData.h"
#include "SceneManager.h"
#include "SheepManager.h"
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
        mExitButton = exitButton;
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
        zoomInButton->SetPressCallback([this](UIButton* button){
            OnZoomInButtonPressed();
        });
        mZoomInButton = zoomInButton;
    }
    {
        UIButton* zoomOutButton = UIUtil::NewUIActorWithWidget<UIButton>(baseImage->GetOwner());
        zoomOutButton->SetUpTexture(gAssetManager.LoadTexture("BINOCBTNZOOMOUTU.BMP"));
        zoomOutButton->SetHoverTexture(gAssetManager.LoadTexture("BINOCBTNZOOMOUTHOV.BMP"));
        zoomOutButton->SetDownTexture(gAssetManager.LoadTexture("BINOCBTNZOOMOUTD.BMP"));
        zoomOutButton->SetDisabledTexture(gAssetManager.LoadTexture("BINOCBTNZOOMOUTDIS.BMP"));
        zoomOutButton->GetRectTransform()->SetAnchor(AnchorPreset::BottomRight);
        zoomOutButton->GetRectTransform()->SetAnchoredPosition(-95.0f, 62.0f);
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

    mGameCamera = gSceneManager.GetScene()->GetCamera();
    mGameCamera->SetSceneActive(false);
    Vector3 camPos = mGameCamera->GetPosition();
    camPos.y += 10000.0f;
    mGameCamera->SetPosition(camPos);

    mGameCamera->SetAngle(Vector2::Zero);

    // Figure out the use case from the current location and timeblock.
    auto& it = mUseCases.find(gLocationManager.GetLocation() + gGameProgress.GetTimeblock().ToString());
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
    SetActive(false);

    mGameCamera->SetSceneActive(true);
    Vector3 camPos = mGameCamera->GetPosition();
    camPos.y -= 10000.0f;
    mGameCamera->SetPosition(camPos);

    // Play animation to put away binocs when exiting this screen.
    // Without this, Gabe/Grace just continue to stand with the binocs up to their faces!
    if(mCurrentUseCase != nullptr)
    {
        Animation* anim = gAssetManager.LoadAnimation(mCurrentUseCase->exitAnimName);
        if(anim != nullptr)
        {
            gSceneManager.GetScene()->GetAnimator()->Start(anim);
        }
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
        //mZoomedInCameraAngle.y = Math::Clamp(mZoomedOutCameraAngle.y, -0.16259, 0.239742);
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

        // Zoom in button is interactable if we have a location we could zoom to.
        mZoomInButton->SetCanInteract(!mCamZoomToLocCode.empty());
    }
    
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
    mZoomedOutBSP = gRenderer.GetBSP();
    SceneGeometryData geometryData;
    geometryData.Load(mCurrentZoomLocation->sceneAssetName);
    geometryData.GetBSP()->SetFloorObjectName(mCurrentZoomLocation->floorModelName);
    gRenderer.SetBSP(geometryData.GetBSP());

    // Update the camera position and angle for this zoom location.
    mGameCamera->SetPosition(mCurrentZoomLocation->cameraPos);
    mGameCamera->SetAngle(mCurrentZoomLocation->cameraAngle);
    mZoomedInCameraAngle = mCurrentZoomLocation->cameraAngle;

    // Play the entry Sheep.
    if(!mCurrentZoomLocation->enterSheepFunctionName.empty())
    {
        // BINOCS.SHP *may* also have an "all" version of the enter script. This should be called if it exists.
        std::string allFuncName = gLocationManager.GetLocation() + "all" + mCamZoomToLocCode + "ent$";
        gSheepManager.Execute(mBinocsScript, allFuncName, nullptr);
        gSheepManager.Execute(mBinocsScript, mCurrentZoomLocation->enterSheepFunctionName, nullptr);
    }

    // We are zoomed in!
    printf("Zoom to %s\n", mCamZoomToLocCode.c_str());
    mIsZoomedIn = true;
}

void BinocsOverlay::OnZoomOutButtonPressed()
{
    // Go back to the normal scene BSP.
    gRenderer.SetBSP(mZoomedOutBSP);

    // Set camera back to the zoomed out position and angle.
    mGameCamera->SetPosition(mZoomedOutCameraPos);
    mGameCamera->SetAngle(mZoomedOutCameraAngle);

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
