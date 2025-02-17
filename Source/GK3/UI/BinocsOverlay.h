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
#include "Vector2.h"
#include "Vector3.h"

class GameCamera;
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
    UIButton* mUpButton = nullptr;
    UIButton* mDownButton = nullptr;
    UIButton* mRightButton = nullptr;
    UIButton* mLeftButton = nullptr;


    struct ZoomInstance
    {
        // The rectangle on the skybox that enables the "Zoom 50x" button to view this instance.
        Rect zoomInRect;

        // The name of the scene asset to use.
        std::string sceneAssetName;

        // When zoomed in, the camera angle and position to start at.
        Vector2 cameraAngle;
        Vector3 cameraPos;

        // In the zoomed scene, the name of the floor model to use.
        std::string floorModelName;

        // When entering/exiting this instance, sheep functions in BINOCS.SHP to execute.
        std::string enterSheepFunctionName;
        std::string exitSheepFunctionName;

        // When zoomed in, a rectangle in which some VO will play.
        Rect voRect;
        std::string voLicensePlate;
    };

    struct LocAndTime
    {


        // The anim to play when exiting the binocs overlay (e.g. to have Ego lower the binocs).
        std::string exitAnimName;
    };

    // A reference to the game camera, for convenience.
    GameCamera* mGameCamera = nullptr;

    Vector2 mCameraAngle;

    void OnZoomInButtonPressed();
    void OnZoomOutButtonPressed();
};