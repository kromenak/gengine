#include "SceneFunctions.h"

#include "GKObject.h"
#include "LaserHead.h"
#include "LocationManager.h"
#include "SceneManager.h"
#include "SoundtrackPlayer.h"

std::string_map_ci<std::function<void()>> SceneFunctions::sSceneFunctions;

namespace
{
    // CS2
    LaserHead* cs2LaserHeads[5] = { 0 };

    void CS2_Init()
    {
        // Find each head in the scene and add the LaserHead component to each.
        for(int i = 0; i < 5; ++i)
        {
            GKObject* head = gSceneManager.GetScene()->GetSceneObjectByModelName("cs2head0" + std::to_string(i + 1));
            if(head != nullptr)
            {
                // Save a reference for use in other functions.
                cs2LaserHeads[i] = head->AddComponent<LaserHead>(i);
            }
        }
    }

    void CS2_ToggleLasers()
    {
        // Toggle the lasers on/off.
        for(int i = 0; i < 5; ++i)
        {
            if(cs2LaserHeads[i] != nullptr)
            {
                cs2LaserHeads[i]->SetLaserEnabled(!cs2LaserHeads[i]->IsLaserEnabled());
            }
        }

        // Somewhat randomly, this function is also responsible for stopping the current soundtrack.
        gSceneManager.GetScene()->GetSoundtrackPlayer()->Stop("NocturneSlow");
    }

    void CS2_Head1TurnLeft() { cs2LaserHeads[0]->TurnLeft(); }
    void CS2_Head1TurnRight() { cs2LaserHeads[0]->TurnRight(); }
    void CS2_Head2TurnLeft() { cs2LaserHeads[1]->TurnLeft(); }
    void CS2_Head2TurnRight() { cs2LaserHeads[1]->TurnRight(); }
    void CS2_Head3TurnLeft() { cs2LaserHeads[2]->TurnLeft(); }
    void CS2_Head3TurnRight() { cs2LaserHeads[2]->TurnRight(); }
    void CS2_Head4TurnLeft() { cs2LaserHeads[3]->TurnLeft(); }
    void CS2_Head4TurnRight() { cs2LaserHeads[3]->TurnRight(); }
    void CS2_Head5TurnLeft() { cs2LaserHeads[4]->TurnLeft(); }
    void CS2_Head5TurnRight() { cs2LaserHeads[4]->TurnRight(); }
}

void SceneFunctions::Execute(const std::string& functionName)
{
    // If haven't initialized the function map, do it now.
    static bool initialized = false;
    if(!initialized)
    {
        // CS2
        sSceneFunctions["cs2-init"] = CS2_Init;
        sSceneFunctions["cs2-togglelasers"] = CS2_ToggleLasers;
        sSceneFunctions["cs2-turnl1"] = CS2_Head1TurnLeft;
        sSceneFunctions["cs2-turnr1"] = CS2_Head1TurnRight;
        sSceneFunctions["cs2-turnl2"] = CS2_Head2TurnLeft;
        sSceneFunctions["cs2-turnr2"] = CS2_Head2TurnRight;
        sSceneFunctions["cs2-turnl3"] = CS2_Head3TurnLeft;
        sSceneFunctions["cs2-turnr3"] = CS2_Head3TurnRight;
        sSceneFunctions["cs2-turnl4"] = CS2_Head4TurnLeft;
        sSceneFunctions["cs2-turnr4"] = CS2_Head4TurnRight;
        sSceneFunctions["cs2-turnl5"] = CS2_Head5TurnLeft;
        sSceneFunctions["cs2-turnr5"] = CS2_Head5TurnRight;

        initialized = true;
    }

    // Get current location.
    const std::string& location = gLocationManager.GetLocation();

    // Generate a function key and see if it exists in the map.
    std::string key = location + "-" + functionName;
    auto it = sSceneFunctions.find(key);
    if(it != sSceneFunctions.end())
    {
        // If it exists, call the function!
        it->second();
    }
}