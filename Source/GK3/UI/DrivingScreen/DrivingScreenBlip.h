#pragma once
#include "Actor.h"

#include <functional>
#include <string>
#include <vector>

#include "DrivingScreen.h"
#include "UIImage.h"

class DrivingScreenBlip : public Actor
{
public:
    DrivingScreenBlip(const DrivingScreen::PathData& pathData);

    // Appearance
    void SetImage(UIImage* image) { mImage = image; }
    void SetColor(const Color32& color) { mImage->SetColor(color); }
    void SetBlinkEnabled(bool enabled) { mBlinkInterval = enabled ? 0.5f : 0.0f; }

    // Pathing
    void AddPathNode(const std::string& nodeName);
    void ClearPath();
    void SetLoopPath(bool loop) { mLoopPath = loop; }
    void SkipToPathNode(const std::string& nodeName);
    void SetPathCompleteCallback(std::function<void()> callback) { mPathCompleteCallback = callback; }

    // Following
    void SetFollow(DrivingScreenBlip* other) { mFollowBlip = other; }

    // Positioning
    void SetMapScale(const Vector2& mapScale);
    void SetMapPosition(const std::string& nodeName);
    void SetMapPosition(const Vector2& position);

protected:
    void OnUpdate(float deltaTime) override;

private:
    // The blip's image.
    UIImage* mImage = nullptr;

    // The scale of the map. Needed to properly position blips if map is scaled up/down.
    Vector2 mMapScale = Vector2::One;

    // Path data for the map screen. Used to follow paths between locations on the map.
    const DrivingScreen::PathData& mPathData;

    // The current path and our index along it.
    std::vector<const DrivingScreen::PathData::Node*> mPath;
    int mPathIndex = 0;

    // Path nodes are connected by a "dotted line" of sub-positions.
    // This is the current connection between nodes we are traveling along, and our index in it.
    const DrivingScreen::PathData::Connection* mConnection = nullptr;
    int mConnectionIndex = 0;

    // Amount of time to wait at each node/segment position along a path being followed.
    const float kPathNodeWaitTime = 0.2f;
    float mPathFollowTimer = 0.0f;

    // If true, the path loops once reaching the end.
    bool mLoopPath = false;

    // Callback to fire when reached end of path.
    std::function<void()> mPathCompleteCallback = nullptr;

    // If set, this blip should follow this other blip's movement.
    DrivingScreenBlip* mFollowBlip = nullptr;

    // Whoever is following us will use this point for their position.
    Vector2 mFollowPoint;

    // For tracking whether the blip blinks. By default, no blinking.
    float mBlinkInterval = 0.0f;
    float mBlinkTimer = 0.0f;

    void UpdatePathing(float deltaTime);
};