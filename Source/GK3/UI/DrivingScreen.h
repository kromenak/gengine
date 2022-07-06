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

#include "Vector2.h"

class DrivingScreenBlip;
class UIButton;
class UICanvas;
class UIImage;

class DrivingScreen : public Actor
{
public:
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
        Buthane,
        Wilkes,
        Mosely,
        LadyHowardEstelle1,
        LadyHowardEstelle2,
        PrinceJamesMen
    };

    DrivingScreen();

    void Show(FollowMode followMode = FollowMode::None);
    void Hide();

protected:
    void OnUpdate(float deltaTime) override;

private:
    // The canvas for this screen.
    UICanvas* mCanvas = nullptr;

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

    PathData mPathData;
    
    std::vector<DrivingScreenBlip*> mBlips;
    
    void AddLocation(const std::string& locationCode, const std::string& buttonId, const Vector2& buttonPos);
    void LoadPaths();

    DrivingScreenBlip* CreateBlip();
    void PlaceBlips(FollowMode followMode);
};