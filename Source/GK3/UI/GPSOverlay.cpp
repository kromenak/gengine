#include "GPSOverlay.h"

#include "AssetManager.h"
#include "Font.h"
#include "GKActor.h"
#include "GK3UI.h"
#include "IniParser.h"
#include "LocationManager.h"
#include "SceneManager.h"
#include "TextAsset.h"
#include "UIButton.h"
#include "UICanvas.h"
#include "UIImage.h"
#include "UILabel.h"
#include "UIUtil.h"
#include "Window.h"

bool GPSOverlay::sShowing = false;

GPSOverlay::GPSOverlay() : Actor("GPSOverlay", TransformType::RectTransform)
{
    // The data file GPS.TXT provides two important types of data:
    // 1) The UI layout to use for different screen resolutions.
    // 2) The mapping from world space to the GPS image and GPS coordinates for different locations in the game.
    {
        TextAsset* textAsset = gAssetManager.LoadText("GPS.TXT", AssetScope::Manual);
        IniParser parser(textAsset->GetText(), textAsset->GetTextLength());
        parser.SetMultipleKeyValuePairsPerLine(false);

        // Read each section in turn.
        IniSection section;
        while(parser.ReadNextSection(section))
        {
            // If a section isn't a layout, it's a location.
            if(!ParseLayout(section))
            {
                ParseLocation(section);
            }
        }
        delete textAsset;
    }

    // Next, build the UI. The GPS always appears in top-left corner.
    UICanvas* canvas = AddComponent<UICanvas>(0);
    canvas->GetRectTransform()->SetAnchor(AnchorPreset::TopLeft);

    // Create base map image in top-left.
    // No image yet, since that's affected by both the location and layout used.
    mMapImage = UI::CreateWidgetActor<UIImage>("Map", this);
    mMapImage->GetRectTransform()->SetAnchor(AnchorPreset::TopLeft);
    mMapImage->GetRectTransform()->SetAnchoredPosition(2.0f, -2.0f);

    // Add latitude/longitude labels.
    // Not yet positioned, since that changes based on the layout used.
    mLatitudeLabel = UI::CreateWidgetActor<UILabel>("Latitude", mMapImage);
    mLatitudeLabel->GetRectTransform()->SetAnchor(AnchorPreset::TopLeft);

    mLongitudeLabel = UI::CreateWidgetActor<UILabel>("Longitude", mMapImage);
    mLongitudeLabel->GetRectTransform()->SetAnchor(AnchorPreset::TopLeft);

    // Add target reticule square and vertical/horizontal lines.
    // No image/position is yet set, since those are affected by which layout we use.
    mTargetSquareImage = UI::CreateWidgetActor<UIImage>("TargetSquare", mMapImage);
    mTargetSquareImage->GetRectTransform()->SetAnchor(AnchorPreset::TopLeft);

    mVerticalLineImage = UI::CreateWidgetActor<UIImage>("TargetVertLine", mMapImage);
    mVerticalLineImage->GetRectTransform()->SetAnchor(AnchorPreset::TopLeft);

    mHorizontalLineImage = UI::CreateWidgetActor<UIImage>("TargetHorizLine", mMapImage);
    mHorizontalLineImage->GetRectTransform()->SetAnchor(AnchorPreset::TopLeft);

    // Add power button.
    // Again, position and exact textures aren't set because they depend on the layout used.
    mPowerButton = UI::CreateWidgetActor<UIButton>("PowerButton", mMapImage);
    mPowerButton->GetRectTransform()->SetAnchor(AnchorPreset::TopLeft);
    mPowerButton->SetPressCallback([this](UIButton* button) {
        Hide();
    });

    // For testing, apply one of the layouts and set the target reticule to a default spot.
    //ApplyLayout(1);
    //SetTargetReticuleTexturePos(Vector2::One * 10.0f);

    // Hide until shown.
    SetActive(false);
}

void GPSOverlay::Show()
{
    SetActive(true);
    sShowing = true;

    // Figure out our current location.
    std::string location = gLocationManager.GetLocation();
    auto it = mLocations.find(location);
    if(it != mLocations.end())
    {
        mCurrentLocation = &it->second;
    }
    else if(!mLocations.empty())
    {
        // Fall back on the first one in the list.
        mCurrentLocation = &mLocations.begin()->second;
    }

    // Refresh layout based on screen resolution.
    RefreshLayout();

    // When initially showing the device, it shows in a powered-off state.
    SetPoweredOnUIVisible(false);

    // But after a moment, we'll show the powered-on state.
    mPowerDelayTimer = kPowerDelay;
    mPoweringOn = true;
}

void GPSOverlay::Hide()
{
    // When hiding the device, we briefly show it in a powered-off state.
    SetPoweredOnUIVisible(false);

    // But after a moment, we hide the device entirely.
    mPowerDelayTimer = kPowerDelay;
    mPoweringOn = false;

    sShowing = false;
}

/*static*/ void GPSOverlay::OnPersist(PersistState& ps)
{
    // Save whether GPS is showing or not.
    ps.Xfer(PERSIST_VAR(sShowing));

    // If loading, and GPS should be showing, make sure it is shown.
    if(ps.IsLoading() && sShowing)
    {
        gGK3UI.ShowGPSOverlay();
    }
}

void GPSOverlay::OnUpdate(float deltaTime)
{
    // Handle decrementign the power delay timer.
    if(mPowerDelayTimer > 0.0f)
    {
        mPowerDelayTimer -= deltaTime;
        if(mPowerDelayTimer <= 0.0f)
        {
            // If powering on, show all the powered-on UI items.
            // If powering off, hide the GPS UI entirely.
            if(mPoweringOn)
            {
                SetPoweredOnUIVisible(true);
            }
            else
            {
                SetActive(false);
            }
        }
    }

    // Handle updating the GPS target reticule as Ego moves around the scene.
    if(mCurrentLocation != nullptr)
    {
        // Calculate ego's position in GPS texture space.
        Vector2 egoTexturePos = WorldPosToGPSTexturePos(gSceneManager.GetScene()->GetEgo()->GetPosition());

        // Position reticule at ego's texture position.
        // Also set latitude/longitude equal to ego's position.
        SetTargetReticuleTexturePos(egoTexturePos);
        SetLatLongFromTexturePos(egoTexturePos);
    }

    // Refresh layout based on screen resolution.
    // It's actually necessary to do this in Update, since players can change the resolution while this UI is displayed.
    RefreshLayout();
}

void GPSOverlay::SetPoweredOnUIVisible(bool visible)
{
    // Show or hide UI elements that only appear when the device is "powered on."
    mLongitudeLabel->SetEnabled(visible);
    mLatitudeLabel->SetEnabled(visible);

    mVerticalLineImage->SetEnabled(visible);
    mHorizontalLineImage->SetEnabled(visible);
    mTargetSquareImage->SetEnabled(visible);

    mPowerButton->SetCanInteract(visible);
}

Vector2 GPSOverlay::WorldPosToGPSTexturePos(const Vector3& worldPos)
{
    const float mapWidth = mLayouts[mLayoutIndex].mapWidth;

    // We can calculate which pixel, from the top-left corner of the map image, represents the world origin.
    Vector2 worldOriginInMapPixels = mCurrentLocation->worldOriginPixelPercent * mapWidth;
    worldOriginInMapPixels.x = Math::RoundToInt(worldOriginInMapPixels.x);
    worldOriginInMapPixels.y = Math::RoundToInt(worldOriginInMapPixels.y);

    // Take the passed in world pos and flatten it to 2D using just the x/z values.
    // The z-component must also be negated to properly match the expected directions in texture space.
    Vector2 originToPosPixels(worldPos.x, -worldPos.z);

    // This value is in world units, so multiply it by a conversion factor (pixels per world unit) to get it in texture space.
    const float kPixelsPerUnit = mapWidth / mCurrentLocation->worldWidth;
    originToPosPixels *= kPixelsPerUnit;

    // In the simplest case, the +x axis in the world correlates to the +x axis on the image. And the +z axis in the world correlates to the +y axis on the image.
    // But in some cases, that angle needs to be modified so the 3D world maps correctly to the 2D image.
    float rotAngle = mCurrentLocation->xAxisAngle;
    Vector2 rotatedOriginToPosPixels = Matrix3::MakeRotateZ(-rotAngle).TransformVector(originToPosPixels);

    // Put it all together to calculate a texture pos for this world pos.
    Vector2 result = (worldOriginInMapPixels + rotatedOriginToPosPixels);
    result.x = static_cast<int>(result.x);
    result.y = static_cast<int>(result.y);
    return result;
}

void GPSOverlay::SetTargetReticuleTexturePos(const Vector2& texturePos)
{
    // The reticule is positioned from the top-left corner of the map, NOT the top-left corner of the GPS image!
    // There's a border that must be taken into account.
    // Confusingly, this value also includes the pixels outside the GPS image (buffer to top-left edge of window) - subtract (2,2) to account for that.
    Vector2 borderSizeInPixels = mLayouts[mLayoutIndex].cornerSize - Vector2(2.0f, 2.0f);

    // Update passed in position to account for those border pixels.
    Vector2 actualTexturePos = texturePos + borderSizeInPixels;

    // The target square is positioned from top-left corner, but we want it to be *centered* on the passed in position.
    // Subtract half the texture width to achieve that!
    const float kTargetHalfWidth = mLayouts[mLayoutIndex].targetSquareWidth / 2.0f;
    Vector2 targetSquarePos = actualTexturePos - Vector2(kTargetHalfWidth, kTargetHalfWidth);

    // Set target square to desired position.
    mTargetSquareImage->GetRectTransform()->SetAnchoredPosition(targetSquarePos.x, -targetSquarePos.y + 1); // "+1" because this image has an extra empty line of pixels at top

    // Set vertical/horizontal lines to correct positions as well.
    mVerticalLineImage->GetRectTransform()->SetAnchoredPosition(actualTexturePos.x - 1, -borderSizeInPixels.y);
    mHorizontalLineImage->GetRectTransform()->SetAnchoredPosition(borderSizeInPixels.x, -actualTexturePos.y + 1);
}

void GPSOverlay::SetLatLongFromTexturePos(const Vector2& texturePos)
{
    // GENERAL NOTE: I don't feel incredibly confident that this math is correct. However, I think it's passable.
    // The latitude and longitudes match pretty closely with the three instances you use the GPS in the original game.
    // They aren't exact, but at least the values lead the player to the correct spots, and the values at the correct spots look very close to correct.

    // Convert reference world position to texture space.
    Vector2 refTexturePos = WorldPosToGPSTexturePos(mCurrentLocation->referenceWorldPosition);

    // Get an offset vector from the reference texture pos to the passed in texture pos.
    // This is how far away, in pixels, the passed in texture pos is from the reference latitude/longitude reading.
    Vector2 refToTargetTextureOffset = texturePos - refTexturePos;

    // It's easier to calculate latitude/longitude values in world space. So, let's convert our offset to world space.
    // The world width is supposedly in inches, but let's convert that to meters and get a "meters per pixel" value.
    float metersPerPixel = (mCurrentLocation->worldWidth * 0.0254f) / mLayouts[mLayoutIndex].mapWidth;

    // Convert offset to meters.
    Vector2 refToTargetWorldOffset = refToTargetTextureOffset * metersPerPixel;

    // These values are from wikipedia - approximate latitude/longitude seconds per meter in the real world.
    const float kMetersPerLatitudeSecond = 30.715f;
    const float kMetersPerLongitudeSecond = 30.92f;

    // Based on our offset, calculate how many latitude/longitude seconds that is.
    Vector2 seconds = Vector2(refToTargetWorldOffset.x / kMetersPerLatitudeSecond,
                              refToTargetWorldOffset.y / kMetersPerLongitudeSecond);

    // Convert any seconds over 60 into minutes.
    int minutesX = seconds.x / 60;
    int minutesY = seconds.y / 60;
    seconds.x -= minutesX * 60.0f;
    seconds.y -= minutesY * 60.0f;

    // So the position is some offset from the reference. And we've calculated what that offset is in minutes/seconds.
    // So, we can add that offset to the reference latitude and longitude to get the correct values.
    CoordinateAxis latitude;
    latitude.degree = mCurrentLocation->referenceLatitude.degree;
    latitude.minute = mCurrentLocation->referenceLatitude.minute + minutesY;
    latitude.second = mCurrentLocation->referenceLatitude.second + seconds.y;

    CoordinateAxis longitude;
    longitude.degree = mCurrentLocation->referenceLongitude.degree;
    longitude.minute = mCurrentLocation->referenceLongitude.minute + minutesX;
    longitude.second = mCurrentLocation->referenceLongitude.second + seconds.x;

    // Format the text and display it on the GPS (whew)!
    mLongitudeLabel->SetText(StringUtil::Format("%02d°%02d'%05.2f\"", longitude.degree, longitude.minute, longitude.second));
    mLatitudeLabel->SetText(StringUtil::Format("%02d°%02d'%05.2f\"", latitude.degree, latitude.minute, latitude.second));
}

bool GPSOverlay::ParseLayout(const IniSection& section)
{
    if(StringUtil::EqualsIgnoreCase(section.name, "small") ||
       StringUtil::EqualsIgnoreCase(section.name, "medium") ||
       StringUtil::EqualsIgnoreCase(section.name, "large"))
    {
        // Figure out which layout we're parsing.
        int index = 0;
        if(StringUtil::EqualsIgnoreCase(section.name, "medium"))
        {
            index = 1;
        }
        else if(StringUtil::EqualsIgnoreCase(section.name, "large"))
        {
            index = 2;
        }

        Layout& uiSize = mLayouts[index];
        for(const IniLine& line : section.lines)
        {
            const IniKeyValue& entry = line.entries[0];
            if(StringUtil::EqualsIgnoreCase(entry.key, "nameExt"))
            {
                uiSize.textureSuffix = entry.value;
            }
            else if(StringUtil::EqualsIgnoreCase(entry.key, "vLineFile"))
            {
                uiSize.verticalLineTexture = gAssetManager.LoadTexture(entry.value);
            }
            else if(StringUtil::EqualsIgnoreCase(entry.key, "hLineFile"))
            {
                uiSize.horizontalLineTexture = gAssetManager.LoadTexture(entry.value);
            }
            else if(StringUtil::EqualsIgnoreCase(entry.key, "targetFile"))
            {
                uiSize.targetSquareTexture = gAssetManager.LoadTexture(entry.value);
            }
            else if(StringUtil::EqualsIgnoreCase(entry.key, "fontFile"))
            {
                uiSize.font = gAssetManager.LoadFont(entry.value);
            }
            else if(StringUtil::EqualsIgnoreCase(entry.key, "cornerWidth"))
            {
                uiSize.cornerSize.x = entry.GetValueAsFloat();
            }
            else if(StringUtil::EqualsIgnoreCase(entry.key, "cornerHeight"))
            {
                uiSize.cornerSize.y = entry.GetValueAsFloat();
            }
            else if(StringUtil::EqualsIgnoreCase(entry.key, "mapWidth"))
            {
                uiSize.mapWidth = entry.GetValueAsFloat();
            }
            else if(StringUtil::EqualsIgnoreCase(entry.key, "targetWidth"))
            {
                uiSize.targetSquareWidth = entry.GetValueAsFloat();
            }
            else if(StringUtil::EqualsIgnoreCase(entry.key, "textStartWidth"))
            {
                uiSize.textStartX = entry.GetValueAsFloat();
            }
            else if(StringUtil::EqualsIgnoreCase(entry.key, "latHeight"))
            {
                uiSize.latitudeStartY = -entry.GetValueAsFloat();
            }
            else if(StringUtil::EqualsIgnoreCase(entry.key, "lngHeight"))
            {
                uiSize.longitudeStartY = -entry.GetValueAsFloat();
            }
            else if(StringUtil::EqualsIgnoreCase(entry.key, "powerBtnPos"))
            {
                uiSize.powerButtonPos = entry.GetValueAsVector2();
                uiSize.powerButtonPos.y *= -1.0f;
            }
            else if(StringUtil::EqualsIgnoreCase(entry.key, "powerBtnUpSprite"))
            {
                uiSize.powerButtonUpTexture = gAssetManager.LoadTexture(entry.value);
            }
            else if(StringUtil::EqualsIgnoreCase(entry.key, "powerBtnDownSprite"))
            {
                uiSize.powerButtonDownTexture = gAssetManager.LoadTexture(entry.value);
            }
            else if(StringUtil::EqualsIgnoreCase(entry.key, "powerBtnHoverSprite"))
            {
                uiSize.powerButtonHoverTexture = gAssetManager.LoadTexture(entry.value);
            }
            else if(StringUtil::EqualsIgnoreCase(entry.key, "powerBtnDisabledSprite"))
            {
                uiSize.powerButtonDisabledTexture = gAssetManager.LoadTexture(entry.value);
            }
        }

        // This IniSection did represent a layout.
        return true;
    }

    // This IniSection did not represent a layout.
    return false;
}

void GPSOverlay::ParseLocation(const IniSection& section)
{
    // Ignore the default section.
    if(section.name.empty())
    {
        return;
    }

    LocationData& locationData = mLocations[section.name];
    for(const IniLine& line : section.lines)
    {
        const IniKeyValue& entry = line.entries[0];
        if(StringUtil::EqualsIgnoreCase(entry.key, "name"))
        {
            locationData.mapTextureName = entry.value;
        }
        else if(StringUtil::EqualsIgnoreCase(entry.key, "angXtoN"))
        {
            locationData.xAxisAngle = Math::ToRadians(entry.GetValueAsFloat());
        }
        else if(StringUtil::EqualsIgnoreCase(entry.key, "worldWidth"))
        {
            locationData.worldWidth = entry.GetValueAsFloat();
        }
        else if(StringUtil::EqualsIgnoreCase(entry.key, "mapOriginXPct"))
        {
            locationData.worldOriginPixelPercent.x = entry.GetValueAsFloat();
        }
        else if(StringUtil::EqualsIgnoreCase(entry.key, "mapOriginYPct"))
        {
            locationData.worldOriginPixelPercent.y = 1.0f - entry.GetValueAsFloat();
        }
        else if(StringUtil::EqualsIgnoreCase(entry.key, "sigx"))
        {
            locationData.referenceWorldPosition.x = entry.GetValueAsFloat();
        }
        else if(StringUtil::EqualsIgnoreCase(entry.key, "sigz"))
        {
            locationData.referenceWorldPosition.z = entry.GetValueAsFloat();
        }
        else if(StringUtil::EqualsIgnoreCase(entry.key, "sigLatDeg"))
        {
            locationData.referenceLatitude.degree = entry.GetValueAsInt();
        }
        else if(StringUtil::EqualsIgnoreCase(entry.key, "sigLatMin"))
        {
            locationData.referenceLatitude.minute = entry.GetValueAsInt();
        }
        else if(StringUtil::EqualsIgnoreCase(entry.key, "sigLatSec"))
        {
            locationData.referenceLatitude.second = entry.GetValueAsFloat();
        }
        else if(StringUtil::EqualsIgnoreCase(entry.key, "sigLngDeg"))
        {
            locationData.referenceLongitude.degree = entry.GetValueAsInt();
        }
        else if(StringUtil::EqualsIgnoreCase(entry.key, "sigLngMin"))
        {
            locationData.referenceLongitude.minute = entry.GetValueAsInt();
        }
        else if(StringUtil::EqualsIgnoreCase(entry.key, "sigLngSec"))
        {
            locationData.referenceLongitude.second = entry.GetValueAsFloat();
        }
    }
}

void GPSOverlay::ApplyLayout(int index)
{
    mLayoutIndex = index;

    if(mCurrentLocation != nullptr)
    {
        mMapImage->SetTexture(gAssetManager.LoadTexture(mCurrentLocation->mapTextureName + mLayouts[index].textureSuffix, AssetScope::Scene), true);
    }

    mVerticalLineImage->SetTexture(mLayouts[index].verticalLineTexture, true);
    mHorizontalLineImage->SetTexture(mLayouts[index].horizontalLineTexture, true);
    mTargetSquareImage->SetTexture(mLayouts[index].targetSquareTexture, true);

    mLatitudeLabel->SetFont(mLayouts[index].font);
    mLongitudeLabel->SetFont(mLayouts[index].font);
    mLatitudeLabel->GetRectTransform()->SetAnchoredPosition(mLayouts[index].textStartX, mLayouts[index].latitudeStartY);
    mLatitudeLabel->GetRectTransform()->SetSizeDeltaY(mLayouts[index].font->GetGlyphHeight());
    mLongitudeLabel->GetRectTransform()->SetAnchoredPosition(mLayouts[index].textStartX, mLayouts[index].longitudeStartY);
    mLongitudeLabel->GetRectTransform()->SetSizeDeltaY(mLayouts[index].font->GetGlyphHeight());

    mPowerButton->GetRectTransform()->SetAnchoredPosition(mLayouts[index].powerButtonPos);
    mPowerButton->SetUpTexture(mLayouts[index].powerButtonUpTexture);
    mPowerButton->SetDownTexture(mLayouts[index].powerButtonDownTexture);
    mPowerButton->SetHoverTexture(mLayouts[index].powerButtonHoverTexture);
    mPowerButton->SetDisabledTexture(mLayouts[index].powerButtonDisabledTexture);
}

void GPSOverlay::RefreshLayout()
{
    // The devs created specific layouts for 640x480, 800x600, and 1024x768 resolutions.
    // For anything higher than 1024x768 that is not scaled, use the 1024x768 resolution.
    // However, once UI scaling starts to occur, it looks better to just use the 640x480 version.
    int layoutIndex = 2; // 1024x768
    if(Window::GetWidth() <= 640 || GetComponent<UICanvas>()->GetScaleFactor() > 1.0f)
    {
        layoutIndex = 0; // 640x480
    }
    else if(Window::GetWidth() <= 800)
    {
        layoutIndex = 1; // 800x600
    }

    // Apply the desired layout.
    ApplyLayout(layoutIndex);
}