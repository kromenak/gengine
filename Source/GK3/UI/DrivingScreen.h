//
// Clark Kromenaker
//
// The driving screen shows destinations to travel to, as well as other people who are driving.
//
#pragma once
#include "Actor.h"

#include <string>
#include <unordered_map>
#include <vector>

#include "Color32.h"
#include "Vector2.h"

class DrivingScreenBlip;
class UIButton;
class UIImage;

class DrivingScreen : public Actor
{
public:
    // The Driving Map contains a set of Nodes representing locations.
    // Between nodes, there are connections, with each connection being made of of a set of intermediate points.
    // This data is primarily used for positioning blips and having them follow paths.
    struct PathData
    {
        struct Segment
        {
            std::string name;
            std::vector<Vector2> points;
        };
        std::vector<Segment> segments;

        struct Node;
        struct Connection
        {
            Node* to = nullptr;
            Segment* segment = nullptr;
            bool forward = true;

            Vector2 GetPoint(int index) const
            {
                return forward ? segment->points[index] : segment->points[segment->points.size() - 1 - index];
            }
        };

        struct Node
        {
            std::string name;
            Vector2 point;
            std::vector<Connection> connections;
        };
        std::vector<Node> nodes;
    };

    enum class FollowMode
    {
        None,
        Buthane,            // Follow Buthan 102P
        Wilkes,             // Follow Wilkes 102P
        LadyHoward,         // Follow Lady Howard & Estelle 104P
        PrinceJamesMen,     // Follow Prince James' Men 106P
        Estelle,            // Follow Estelle 202P
    };

    DrivingScreen();
    ~DrivingScreen();

    void Show(FollowMode followMode = FollowMode::None);
    void Hide();

    bool FollowingSomeone() const { return mFollowMode != FollowMode::None; }

private:
    // The map is a child of the screen, and then all the location buttons are children of the map.
    UIImage* mMapImage = nullptr;
    Actor* mMapActor = nullptr;

    // Texture used by the map; used for resizing to fit screen calcs.
    Texture* mMapTexture = nullptr;

    // Maps each location to its button.
    std::unordered_map<std::string, UIButton*> mLocationButtons;

    // Texture used to represent a "blip" on the map.
    // Can't seem to find this in the game's assets, but we can make our own pretty easily.
    Texture* mBlipTexture = nullptr;

    // Path data for the map.
    PathData mPathData;

    // Blips that exist on the map.
    std::vector<DrivingScreenBlip*> mBlips;

    // If we're following someone, this will be set appropriately.
    FollowMode mFollowMode = FollowMode::None;

    // Colors for blips.
    const Color32 kEgoColor = Color32::Green;
    const Color32 kButhaneColor = Color32(198, 182, 255); // Pink
    const Color32 kWilkesColor = Color32(247, 150, 57); // Orange
    const Color32 kEstelleColor = Color32(33, 56, 140); // Purple
    const Color32 kJamesMenColor = Color32(24, 146, 49); // Dark Green

    // Indexes for blips.
    // Better for Ego to be a larger index so it draws on top of other blips.
    const int kNpc1Index = 0;
    const int kNpc2Index = 1;
    const int kEgoIndex = 2;

    void SetLocationButtonsInteractive(bool interactive);
    void ExitToLocation(const std::string& locationCode);

    void AddLocation(const std::string& locationCode, const std::string& buttonId, const Vector2& buttonPos);
    void LoadPaths();

    DrivingScreenBlip* CreateBlip();
    void PlaceBlips(FollowMode followMode);
    void OnFollowDone();
};
