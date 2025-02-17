//
// Clark Kromenaker
// 
// A UI overlay that mimics looking through binoculars.
// At certain points in the game, this allows you to "zoom in" on far away scenes to see what other characters are up to.
//
#pragma once
#include "Actor.h"

#include <string>

#include "Rect.h"
#include "StringUtil.h"
#include "Vector2.h"
#include "Vector3.h"

class BSP;
class GameCamera;
class SheepScript;
class UIButton;

class BinocsOverlay : public Actor
{
public:
    BinocsOverlay();

    void Show();
    void Hide();

protected:
    void OnUpdate(float deltaTime) override;

private:
    // Buttons used to move the binocs view up/down/left/right.
    UIButton* mUpButton = nullptr;
    UIButton* mDownButton = nullptr;
    UIButton* mLeftButton = nullptr;
    UIButton* mRightButton = nullptr;

    // Buttons used to zoom in and out.
    UIButton* mZoomInButton = nullptr;
    UIButton* mZoomOutButton = nullptr;

    // Button used to exit this screen.
    UIButton* mExitButton = nullptr;

    // A reference to the game camera, so we can easily update it.
    GameCamera* mGameCamera = nullptr;

    // All binocular logic zoom logic is in this SheepScript.
    SheepScript* mBinocsScript = nullptr;

    // Represents a location that can be zoomed to from some other location.
    struct ZoomLocation
    {
        // The rectangle on the skybox that enables the "Zoom 50x" button to view this instance.
        Rect zoomInRect;

        // The name of the scene asset to use.
        std::string sceneAssetName;

        // In the zoomed scene, the name of the floor model to use.
        std::string floorModelName;

        // When zoomed in, the camera angle and position to start at.
        Vector2 cameraAngle;
        Vector3 cameraPos;
        
        // When entering/exiting this instance, sheep functions in BINOCS.SHP to execute.
        std::string enterSheepFunctionName;
        std::string exitSheepFunctionName;

        // When zoomed in, a rectangle in which some VO will play.
        Rect voRect;
        std::string voLicensePlate;
    };

    // Represents one location/time where binocs can be used to zoom to see other locations.
    struct UseCase
    {
        // From this time/location, what locations can we zoom to? Key is the location code.
        std::string_map_ci<ZoomLocation> zoomLocations;

        // The anim to play when exiting the binocs overlay (e.g. to have Ego lower the binocs).
        std::string exitAnimName;
    };

    // A map of location and time to binoc use case at that location/time.
    // The key is the location and time concatenated (e.g. CD1102P).
    std::string_map_ci<UseCase> mUseCases;

    // The current use case, based on location binocs were used from.
    UseCase* mCurrentUseCase = nullptr;

    // For the life of me, I could not decipher the ZOOMRECT/VORECT fields in BINOCS.TXT and how they correlate to the view angle of the binoculars.
    // So...as an alt approach, I recorded approximate angles that should allow you to zoom to certain locations.
    struct ZoomAngles
    {
        Vector2 minAngles;
        Vector2 maxAngles;
        ZoomAngles(const Vector2& min, const Vector2& max) : minAngles(min), maxAngles(max) { }
    };
    std::string_map_ci<std::string_map_ci<ZoomAngles>> mZoomAngles = {
        { "CD1", {
            { "PL3", ZoomAngles(Vector2(52.13f, 0.10f), Vector2(60.75f, 4.77f)) },
            { "LHM", ZoomAngles(Vector2(88.44f, -1.54f), Vector2(105.63f, 13.73f)) },
            { "MA3", ZoomAngles(Vector2(154.48f, -1.54f), Vector2(168.80f, 11.82f)) },
            { "CSD", ZoomAngles(Vector2(90.16f, 7.05f), Vector2(96.85f, 13.73f)) }
        }},
        { "MA3", {
            { "CD1", ZoomAngles(Vector2(332.02f, -9.31f), Vector2(350.17f, 5.11f)) }
        }}
    };
    
    // Since the game camera doesn't store its angle in this format internally, we remember and update it here.
    Vector2 mZoomedOutCameraAngle;

    // When we zoom in, the camera position/angle change.
    // We need to save the zoomed out camera position/angle so we can restore it on zoom out.
    Vector3 mZoomedOutCameraPos;

    // What location can we currently zoom into, if any?
    std::string mCamZoomToLocCode;
    ZoomLocation* mCurrentZoomLocation = nullptr;

    // Are we currently zoomed in?
    bool mIsZoomedIn = false;

    // The camera angle when the camera is zoomed in.
    Vector2 mZoomedInCameraAngle;

    // When zoomed in, we must temporarily replace the scene's BSP geometry.
    // We need to remember the old one so we can swap back to it when we zoom back out.
    BSP* mZoomedOutBSP = nullptr;
    
    void OnZoomInButtonPressed();
    void OnZoomOutButtonPressed();
};