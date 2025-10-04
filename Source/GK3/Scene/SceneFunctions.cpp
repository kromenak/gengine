#include "SceneFunctions.h"

#include "ActionManager.h"
#include "Animation.h"
#include "AnimationNodes.h"
#include "AssetManager.h"
#include "Bridge.h"
#include "Chessboard.h"
#include "DemonFight.h"
#include "GameProgress.h"
#include "GK3UI.h"
#include "GKActor.h"
#include "GKObject.h"
#include "LaserHead.h"
#include "LocationManager.h"
#include "Pendulum.h"
#include "SceneManager.h"
#include "SoundtrackPlayer.h"
#include "VertexAnimation.h"
#include "WalkerBoundary.h"

std::string_map_ci<std::function<void(const std::function<void()>&)>> SceneFunctions::sSceneFunctions;

namespace
{
    void LBY_Init(const std::function<void()>& callback)
    {
        // This bit of code fixes Buchelli's wine glass floating mid-air if you go to the lobby near the end of Day 2, 5PM timeblock (after he goes to dining room to play Bridge).
        // This functions correctly in the original game, BUT I just don't understand how!
        // The init anims for bglass/bourbon (on lines 45-46 of LBY205P.SIF) do not actually animation the glass at all. So...how does the glass get to the correct spot?
        // Until that mystery can be resolved, this bit of code alleviates the problem by sampling an anim that DOES involve the glass.
        if(gGameProgress.GetTimeblock() == Timeblock(2, 17)) // 205P
        {
            // This variable ensures we only run the anim after Buchelli has gone to the Dining Room.
            if(gGameProgress.GetGameVariable("LSRState") > 2)
            {
                // This anim involves Buchelli putting the wine glass down on the table.
                // Get the vertex anims for the glass and contained liquid.
                Animation* anim = gAssetManager.LoadAnimation("VITLBYSTANDWBRB.ANM", AssetScope::Scene);
                VertexAnimNode* glassAnim = anim->GetFirstVertexAnimationForModel("bglass");
                VertexAnimNode* wineAnim = anim->GetFirstVertexAnimationForModel("bourbon");

                // Sample anims on their last frames, which is when Buchelli has placed the glass on the table.
                glassAnim->Sample(glassAnim->vertexAnimation->GetFrameCount() - 1);
                wineAnim->Sample(wineAnim->vertexAnimation->GetFrameCount() - 1);
            }
        }
        if(callback != nullptr) { callback(); }
    }
}

namespace
{
    void MS3_Init(const std::function<void()>& callback)
    {
        // This fixes a bug that could soft lock the game when eavesdropping on Lady Howard & Estelle during Day 1, 10AM.
        // The eavesdrop cutscene code has an infinite loop, waiting for the TE6Topics flag to be cleared.
        //
        // This flag is meant to indicate when LH & E are playing a "turn to face museum display" animation.
        // Problem is, if you exit the scene during that animation, this flag is never cleared. Eavesdropping after that will soft lock.
        //
        // This bug occurs in the original game! To fix, the scene enter NVC/SHP should clear this flag.
        // But since I can't modify the game's data files, I will have to clear it via code.
        gGameProgress.ClearFlag("TE6Topics");
        if(callback != nullptr) { callback(); }
    }
}

namespace
{
    // CSE
    void CSE_Init(const std::function<void()>& callback)
    {
        // In Day 2, 2PM, Gabriel visits Montreaux posing as a journalist. I encountered a bug where Montreaux walks through a door!
        // On closer inspection, this is because the walker boundary has a 1-pixel spot that technically counts as a valid path, but it looks bad to take it.
        // Though I do think it'd be nice if the pathfinding system took "walker size" into account - for now, I can fix this bug by simply disallowing walking on region 6 (cyan) on walker bounds.
        if(gGameProgress.GetTimeblock() == Timeblock(2, 14) || gGameProgress.GetTimeblock() == Timeblock(3, 15))
        {
            gSceneManager.GetScene()->GetSceneData()->GetWalkerBoundary()->SetRegionBlocked(6, 6, true);
        }
        if(callback != nullptr) { callback(); }
    }
}

namespace
{
    // CS2
    LaserHead* cs2LaserHeads[5] = { 0 };

    void CS2_OnPersist(PersistState& ps)
    {
        for(LaserHead* laserHead : cs2LaserHeads)
        {
            laserHead->OnPersist(ps);
        }
    }

    void CS2_Init(const std::function<void()>& callback)
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

        gSceneManager.GetScene()->AddPersistCallback(CS2_OnPersist);

        if(callback != nullptr) { callback(); }
    }

    void CS2_ToggleLasers(const std::function<void()>& callback)
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
        if(callback != nullptr) { callback(); }
    }

    void CS2_Head1TurnLeft(const std::function<void()>& callback)
    {
        cs2LaserHeads[0]->TurnLeft(callback);
    }

    void CS2_Head1TurnRight(const std::function<void()>& callback)
    {
        cs2LaserHeads[0]->TurnRight(callback);
    }

    void CS2_Head2TurnLeft(const std::function<void()>& callback)
    {
        cs2LaserHeads[1]->TurnLeft(callback);
    }

    void CS2_Head2TurnRight(const std::function<void()>& callback)
    {
        cs2LaserHeads[1]->TurnRight(callback);
    }

    void CS2_Head3TurnLeft(const std::function<void()>& callback)
    {
        cs2LaserHeads[2]->TurnLeft(callback);
    }

    void CS2_Head3TurnRight(const std::function<void()>& callback)
    {
        cs2LaserHeads[2]->TurnRight(callback);
    }

    void CS2_Head4TurnLeft(const std::function<void()>& callback)
    {
        cs2LaserHeads[3]->TurnLeft(callback);
    }

    void CS2_Head4TurnRight(const std::function<void()>& callback)
    {
        cs2LaserHeads[3]->TurnRight(callback);
    }

    void CS2_Head5TurnLeft(const std::function<void()>& callback)
    {
        cs2LaserHeads[4]->TurnLeft(callback);
    }

    void CS2_Head5TurnRight(const std::function<void()>& callback)
    {
        cs2LaserHeads[4]->TurnRight(callback);
    }

}

namespace
{
    // CHU
    GKObject* angelDots[4] = { 0 }; // top, right, bottom, left
    GKObject* angelEdges[6] = { 0 }; // left-to-top, top-to-right, right-to-bottom, bottom-to-left, top-to-bottom, left-to-right
    int lastDotIndex = -1;
    bool saidInitialDialogue = false;

    void CHU_OnPersist(PersistState& ps)
    {
        ps.Xfer(PERSIST_VAR(lastDotIndex));
        ps.Xfer(PERSIST_VAR(saidInitialDialogue));
    }

    void CHU_Erase(const std::function<void()>& callback)
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
        if(callback != nullptr) { callback(); }
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
                // The "interact with angel" action is still ongoing here, so we need to wait for it to complete before playing dialogue.
                gActionManager.WaitForActionsToComplete([](){
                    gActionManager.ExecuteDialogueAction("18P1F0M021");
                });
            }
            saidInitialDialogue = true;
        }

        // See if we successfully created the tilted square.
        if(angelEdges[0]->IsActive() && angelEdges[1]->IsActive() && angelEdges[2]->IsActive() &&
           angelEdges[3]->IsActive() && !angelEdges[4]->IsActive() && !angelEdges[5]->IsActive())
        {
            // This lets sheep code know that we did it.
            gGameProgress.SetNounVerbCount("Four_Angels", "Trace", 1);

            // Finish dialogue differs based on who's doing this.
            std::string dialogueLicensePlate;
            if(StringUtil::EqualsIgnoreCase(Scene::GetEgoName(), "Gabriel"))
            {
                dialogueLicensePlate = "18L9M0MZ81";
            }
            else
            {
                dialogueLicensePlate = "18P9M0MCE1";
            }

            // As above, "interact with angel" action is ongoing, so must wait until it completes before playing dialogue.
            gActionManager.WaitForActionsToComplete([dialogueLicensePlate](){
                gActionManager.ExecuteDialogueAction(dialogueLicensePlate, 1, [](const Action* action){

                    // The original game also keeps the action active a bit longer, I guess so you can see the tilted square before it disappears.
                    // So, let's do that too.
                    gActionManager.ExecuteSheepAction("wait SetTimerSeconds(2)", [](const Action* action){
                        CHU_Erase(nullptr);
                    });
                });
            });
        }
    }

    void CHU_Init(const std::function<void()>& callback)
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

        gSceneManager.GetScene()->AddPersistCallback(CHU_OnPersist);
        if(callback != nullptr) { callback(); }
    }

    void CHU_Angel1(const std::function<void()>& callback)
    {
        AddDot(0);
        if(callback != nullptr) { callback(); }
    }

    void CHU_Angel2(const std::function<void()>& callback)
    {
        AddDot(1);
        if(callback != nullptr) { callback(); }
    }

    void CHU_Angel3(const std::function<void()>& callback)
    {
        AddDot(2);
        if(callback != nullptr) { callback(); }
    }

    void CHU_Angel4(const std::function<void()>& callback)
    {
        AddDot(3);
        if(callback != nullptr) { callback(); }
    }
}

namespace
{
    // Strangely, the GPS interface is not shown via a dedicated Sheep API call.
    // Instead, it hooks into the SceneFunctions system in a few scenes.
    void GPS_On(const std::function<void()>& callback)
    {
        gGK3UI.ShowGPSOverlay();
        if(callback != nullptr) { callback(); }
    }

    void GPS_Off(const std::function<void()>& callback)
    {
        gGK3UI.HideGPSOverlay();
        if(callback != nullptr) { callback(); }
    }
}

namespace
{
    void CD1_Init(const std::function<void()>& callback)
    {
        // HACK: For some reason, Emilio's position isn't correct (compared to the original game) when he's sitting at Chateau de Blanchfort during Day 1, 4PM.
        // HACK: The really weird thing is...the game data tells him to sit at a specific position; in the original game, he IS NOT at that position. In G-Engine, he does go exactly to the specified position.
        // HACK: So, I don't understand why Emilio is in the spot he's in in the original game, since the game data says he should be elsewhere.
        // HACK: As a workaround, I'll just force his position to something that looks correct here.
        if(gGameProgress.GetTimeblock() == Timeblock(1, 4, Timeblock::PM))
        {
            GKActor* actor = gSceneManager.GetScene()->GetActorByNoun("Emilio");
            if(actor != nullptr)
            {
                actor->SetPosition(Vector3(1272.0f, 723.0f, -616.0f));
            }
        }
        if(callback != nullptr) { callback(); }
    }
}

namespace
{
    // TE1 has a giant chessboard puzzle. All the logic is encompassed in this Chessboard class.
    Chessboard* chessboard = nullptr;

    void TE1_OnPersist(PersistState& ps)
    {
        chessboard->OnPersist(ps);
    }

    void TE1_Init(const std::function<void()>& callback)
    {
        chessboard = new Chessboard();
        gSceneManager.GetScene()->AddPersistCallback(TE1_OnPersist);
        if(callback != nullptr) { callback(); }
    }

    void TE1_ClearTiles(const std::function<void()>& callback)
    {
        chessboard->Reset(false);
        if(callback != nullptr) { callback(); }
    }

    void TE1_Reset(const std::function<void()>& callback)
    {
        chessboard->Reset(true);
        if(callback != nullptr) { callback(); }
    }

    void TE1_Takeoff(const std::function<void()>& callback)
    {
        chessboard->Takeoff();
        if(callback != nullptr) { callback(); }
    }

    void TE1_Landed(const std::function<void()>& callback)
    {
        chessboard->Landed();
        if(callback != nullptr) { callback(); }
    }

    void TE1_HideCurrentTile(const std::function<void()>& callback)
    {
        chessboard->HideCurrentTile();
        if(callback != nullptr) { callback(); }
    }

    void TE1_Fell(const std::function<void()>& callback)
    {
        /*TODO: Does this actually do anything? Maybe reset state?*/
        if(callback != nullptr) { callback(); }
    }

    void TE1_CenterMe(const std::function<void()>& callback)
    {
        chessboard->CenterEgo();
        if(callback != nullptr) { callback(); }
    }

    void TE1_BadLand(const std::function<void()>& callback)
    {
        chessboard->BadLand();
        if(callback != nullptr) { callback(); }
    }
}

namespace
{
    Pendulum* pendulum = nullptr;

    void TE3_OnPersist(PersistState& ps)
    {
        pendulum->OnPersist(ps);
    }

    void TE3_Init(const std::function<void()>& callback)
    {
        pendulum = new Pendulum();
        gSceneManager.GetScene()->AddPersistCallback(TE3_OnPersist);
        if(callback != nullptr) { callback(); }
    }
}

namespace
{
    Bridge* bridge = nullptr;

    void TE5_OnPersist(PersistState& ps)
    {
        bridge->OnPersist(ps);
    }

    void TE5_Init(const std::function<void()>& callback)
    {
        bridge = new Bridge();
        gSceneManager.GetScene()->AddPersistCallback(TE5_OnPersist);
        if(callback != nullptr) { callback(); }
    }
}

namespace
{
    void TE6_Init(const std::function<void()>& callback)
    {
        new DemonFight();
        if(callback != nullptr) { callback(); }
    }
}

void SceneFunctions::Execute(const std::string& functionName, const std::function<void()>& callback)
{
    // If haven't initialized the function map, do it now.
    static bool initialized = false;
    if(!initialized)
    {
        // LBY
        sSceneFunctions["lby-init"] = LBY_Init;

        // MS3
        sSceneFunctions["ms3-init"] = MS3_Init;

        // CD1
        sSceneFunctions["cd1-init"] = CD1_Init;

        // CSE
        sSceneFunctions["cse-init"] = CSE_Init;

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

        // TE5
        sSceneFunctions["te5-init"] = TE5_Init;

        // TE6
        sSceneFunctions["te6-init"] = TE6_Init;
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
        it->second(callback);
    }
    else
    {
        if(callback != nullptr)
        {
            callback();
        }
    }
}