#include "SceneFunctions.h"

#include "ActionManager.h"
#include "Chessboard.h"
#include "DialogueManager.h"
#include "GameProgress.h"
#include "GK3UI.h"
#include "GKObject.h"
#include "LaserHead.h"
#include "LocationManager.h"
#include "Pendulum.h"
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

namespace
{
    // CHU
    GKObject* angelDots[4] = { 0 }; // top, right, bottom, left
    GKObject* angelEdges[6] = { 0 }; // left-to-top, top-to-right, right-to-bottom, bottom-to-left, top-to-bottom, left-to-right
    int lastDotIndex = -1;
    bool saidInitialDialogue = false;

    void CHU_Erase()
    {
        lastDotIndex = -1;
        for(GKObject* dot : angelDots)
        {
            if(dot != nullptr)
            {
                dot->SetActive(false);
            }
        }
        for(GKObject* edge : angelEdges)
        {
            if(edge != nullptr)
            {
                edge->SetActive(false);
            }
        }
        gGameProgress.SetNounVerbCount("Four_Angels", "ERASE", 0);
    }

    void ActivateEdge(int dotIndex)
    {
        // No edge yet.
        if(lastDotIndex == -1) { return; }

        // See what dot was last, and which is current.
        // And activate the appropriate edge.
        if(lastDotIndex == 0) // top
        {
            if(dotIndex == 1) // right
            {
                angelEdges[1]->SetActive(true);
            }
            else if(dotIndex == 2) // bottom
            {
                angelEdges[4]->SetActive(true);
            }
            else if(dotIndex == 3) // left
            {
                angelEdges[0]->SetActive(true);
            }
        }
        else if(lastDotIndex == 1) // right
        {
            if(dotIndex == 0) // top
            {
                angelEdges[1]->SetActive(true);
            }
            else if(dotIndex == 2) // bottom
            {
                angelEdges[2]->SetActive(true);
            }
            else if(dotIndex == 3) // left
            {
                angelEdges[5]->SetActive(true);
            }
        }
        else if(lastDotIndex == 2) // bottom
        {
            if(dotIndex == 0) // top
            {
                angelEdges[4]->SetActive(true);
            }
            else if(dotIndex == 1) // right
            {
                angelEdges[2]->SetActive(true);
            }
            else if(dotIndex == 3) // left
            {
                angelEdges[3]->SetActive(true);
            }
        }
        else // left
        {
            if(dotIndex == 0) // top
            {
                angelEdges[0]->SetActive(true);
            }
            else if(dotIndex == 1) // right
            {
                angelEdges[5]->SetActive(true);
            }
            else if(dotIndex == 2) // bottom
            {
                angelEdges[3]->SetActive(true);
            }
        }
    }

    void AddDot(int dotIndex)
    {
        // Activate the appropriate dot.
        angelDots[dotIndex]->SetActive(true);

        // Activate the edge between this dot and the previous dot.
        ActivateEdge(dotIndex);

        // Remember this dot as the previous going forward.
        lastDotIndex = dotIndex;

        // Since we added a dot, it is valid to do an erase action.
        // The NVC listens for this noun/verb to be non-zero to enable the erase action.
        gGameProgress.SetNounVerbCount("Four_Angels", "ERASE", 1);

        // Grace says a piece of dialogue when you lay down the first dot.
        // She says it again each time you re-enter the scene, so it isn't controlled by a game state flag.
        if(!saidInitialDialogue)
        {
            if(StringUtil::EqualsIgnoreCase(Scene::GetEgoName(), "Grace"))
            {
                //TODO: Can't execute a dialogue action here, since another action may be ongoing.
                //TODO: The original game seems to maybe have this problem too...but they do a fake "wait" period instead.
                //gActionManager.ExecuteDialogueAction("18P1F0M021");
                gDialogueManager.StartDialogue("18P1F0M021", 1, false, nullptr);
            }
            saidInitialDialogue = true;
        }

        // See if we successfully created the tilted square.
        if(angelEdges[0]->IsActive() && angelEdges[1]->IsActive() && angelEdges[2]->IsActive() &&
           angelEdges[3]->IsActive() && !angelEdges[4]->IsActive() && !angelEdges[5]->IsActive())
        {
            // This lets sheep code know that we did it.
            gGameProgress.SetNounVerbCount("Four_Angels", "Trace", 1);

            if(StringUtil::EqualsIgnoreCase(Scene::GetEgoName(), "Gabriel"))
            {
                //TODO: Same problem as mentioned above.
                //gActionManager.ExecuteDialogueAction("18L9M0MZ81");
                gDialogueManager.StartDialogue("18L9M0MZ81", 1, false, [](){
                    CHU_Erase();
                });
            }
            else
            {
                //TODO: Same problem as mentioned above.
                //gActionManager.ExecuteDialogueAction("18P9M0MCE1");
                gDialogueManager.StartDialogue("18P9M0MCE1", 1, false, [](){
                    CHU_Erase();
                });
            }
        }
    }

    void CHU_Init()
    {
        // Find and save dots.
        for(int i = 0; i < 4; ++i)
        {
            GKObject* dot = gSceneManager.GetScene()->GetSceneObjectByModelName("chu_laserdot0" + std::to_string(i + 1));
            if(dot != nullptr)
            {
                angelDots[i] = dot;
            }
        }

        // Find and save edges.
        for(int i = 0; i < 6; ++i)
        {
            GKObject* edge = gSceneManager.GetScene()->GetSceneObjectByModelName("chu_laser0" + std::to_string(i + 1));
            if(edge != nullptr)
            {
                angelEdges[i] = edge;
            }
        }

        saidInitialDialogue = false;
    }

    void CHU_Angel1()
    {
        AddDot(0);
    }

    void CHU_Angel2()
    {
        AddDot(1);
    }

    void CHU_Angel3()
    {
        AddDot(2);
    }

    void CHU_Angel4()
    {
        AddDot(3);
    }
}

namespace
{
    // Strangely, the GPS interface is not shown via a dedicated Sheep API call.
    // Instead, it hooks into the SceneFunctions system in a few scenes.
    void GPS_On()
    {
        gGK3UI.ShowGPSOverlay();
    }

    void GPS_Off()
    {
        gGK3UI.HideGPSOverlay();
    }
}

namespace
{
    // TE1 has a giant chessboard puzzle. All the logic is encompassed in this Chessboard class.
    Chessboard* chessboard = nullptr;

    void TE1_Init() { chessboard = new Chessboard(); }
    void TE1_ClearTiles() { chessboard->Reset(false); }
    void TE1_Reset() { chessboard->Reset(true); }
    void TE1_Takeoff() { chessboard->Takeoff(); }
    void TE1_Landed() { chessboard->Landed(); }
    void TE1_HideCurrentTile() { chessboard->HideCurrentTile(); } 
    void TE1_Fell() { /*TODO: Does this actually do anything? Maybe reset state?*/ }
    void TE1_CenterMe() { chessboard->CenterEgo(); }
    void TE1_BadLand() { chessboard->BadLand(); }
}

namespace
{
    void TE3_Init()
    {
        GKObject* pendulumActor = gSceneManager.GetScene()->GetSceneObjectByModelName("te3_pendulum_center_code");
        if(pendulumActor != nullptr)
        {
            pendulumActor->AddComponent<Pendulum>();
        }
    }
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

        // CHU
        sSceneFunctions["chu-init"] = CHU_Init;
        sSceneFunctions["chu-angel1"] = CHU_Angel1;
        sSceneFunctions["chu-angel2"] = CHU_Angel2;
        sSceneFunctions["chu-angel3"] = CHU_Angel3;
        sSceneFunctions["chu-angel4"] = CHU_Angel4;
        sSceneFunctions["chu-erase"] = CHU_Erase;

        // LER
        sSceneFunctions["ler-on"] = GPS_On;
        sSceneFunctions["ler-off"] = GPS_Off;

        // MCF
        sSceneFunctions["mcf-on"] = GPS_On;
        sSceneFunctions["mcf-off"] = GPS_Off;

        // BEC
        sSceneFunctions["bec-on"] = GPS_On;
        sSceneFunctions["bec-off"] = GPS_Off;

        // TE1
        sSceneFunctions["te1-init"] = TE1_Init;
        sSceneFunctions["te1-cleartiles"] = TE1_ClearTiles;
        sSceneFunctions["te1-reset"] = TE1_Reset;
        sSceneFunctions["te1-landed"] = TE1_Landed;
        sSceneFunctions["te1-takeoff"] = TE1_Takeoff;
        sSceneFunctions["te1-hidecurrenttile"] = TE1_HideCurrentTile;
        sSceneFunctions["te1-fell"] = TE1_Fell;
        sSceneFunctions["te1-centerme"] = TE1_CenterMe;
        sSceneFunctions["te1-badland"] = TE1_BadLand;

        // TE3
        sSceneFunctions["te3-init"] = TE3_Init;
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