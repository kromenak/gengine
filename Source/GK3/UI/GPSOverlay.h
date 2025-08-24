//
// Clark Kromenaker
//
// The GPS overlay is a UI that appears when using the in-game GPS device.
// It appears in the upper-left corner and shows a character's GPS coordinates in the current scene.
//
#pragma once
#include "Actor.h"
#include "StringUtil.h"

class Font;
struct IniSection;
class PersistState;
class Texture;
class UIButton;
class UIImage;
class UILabel;

class GPSOverlay : public Actor
{
public:
    GPSOverlay();

    void Show();
    void Hide();

    static void OnPersist(PersistState& ps);

protected:
    void OnUpdate(float deltaTime) override;

private:
    // Tracks whether the GPS overlay ought to be open right now.
    static bool sShowing;

    // The map shown on the GPS.
    UIImage* mMapImage = nullptr;

    // Labels showing current longitude and latitude.
    UILabel* mLongitudeLabel = nullptr;
    UILabel* mLatitudeLabel = nullptr;

    // Target reticule images on the GPS.
    UIImage* mVerticalLineImage = nullptr;
    UIImage* mHorizontalLineImage = nullptr;
    UIImage* mTargetSquareImage = nullptr;

    // A button to turn off the power (closes the GPS).
    UIButton* mPowerButton = nullptr;

    // When showing or hiding the GPS, there's a short delay where some UI elements are not displayed.
    // This simulates the power turning on or off.
    const float kPowerDelay = 1.0f;
    float mPowerDelayTimer = 0.0f;
    bool mPoweringOn = false;

    // The GPS UI is provided in three different sizes: small, medium, and large.
    // I believe the intention is to use different texture scales depending on the game's resolution.
    // 640x480 uses small size, 800x600 uses medium size, anything larger uses large size.
    struct Layout
    {
        // A suffix used when loading map images for this layout.
        std::string textureSuffix;

        // Target reticule textures used for this layout.
        Texture* verticalLineTexture = nullptr;
        Texture* horizontalLineTexture = nullptr;
        Texture* targetSquareTexture = nullptr;

        // Font and text positions used in this layout.
        Font* font = nullptr;
        float textStartX = 0.0f;
        float latitudeStartY = 0.0f;
        float longitudeStartY = 0.0f;

        // Values for doing world-to-texture calculations in this layout.
        Vector2 cornerSize;
        float mapWidth = 0.0f;
        float targetSquareWidth = 0.0f;

        // Location and appearance of power button in this layout.
        Vector2 powerButtonPos;
        Texture* powerButtonUpTexture = nullptr;
        Texture* powerButtonDownTexture = nullptr;
        Texture* powerButtonHoverTexture = nullptr;
        Texture* powerButtonDisabledTexture = nullptr;
    };
    Layout mLayouts[3];

    // The layout index to use.
    int mLayoutIndex = 1;

    // Geographic coordinates units of measure for latitude and longitude.
    struct CoordinateAxis
    {
        int degree = 0;
        int minute = 0;
        float second = 0.0f;
    };

    // Maps a three-letter location code to the GPS properties for that location.
    struct LocationData
    {
        // The GPS texture to use for this location.
        std::string mapTextureName;

        // An angle (counter-clockwise, in radians) between the world x-axis and the GPS image x-axis.
        // At an angle of zero degrees, walking down the +x axis in the 3D world moves you to the right on the GPS image.
        // At an angle of 90 degrees, walking down the +x axis in the 3D world moves you upwards on the GPS image.
        float xAxisAngle = 0.0f;

        // The size in world units that fits within the current map's width. Important for converting world-to-texture space.
        float worldWidth = 0.0f;

        // The origin of the 3D map corresponds to this pixel (from top-left) of the GPS texture.
        Vector2 worldOriginPixelPercent;

        // A reference world point and what its associated latitude/longitude should be.
        Vector3 referenceWorldPosition;
        CoordinateAxis referenceLatitude;
        CoordinateAxis referenceLongitude;
    };
    std::string_map_ci<LocationData> mLocations;

    // When shown, the specific location data we'll be using.
    LocationData* mCurrentLocation = nullptr;

    void SetPoweredOnUIVisible(bool visible);

    Vector2 WorldPosToGPSTexturePos(const Vector3& worldPos);
    void SetTargetReticuleTexturePos(const Vector2& texturePos);
    void SetLatLongFromTexturePos(const Vector2& texturePos);

    bool ParseLayout(const IniSection& section);
    void ParseLocation(const IniSection& section);
    void ApplyLayout(int index);
    void RefreshLayout();
};