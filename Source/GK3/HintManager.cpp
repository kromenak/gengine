#include "HintManager.h"

#include "GK3UI.h"
#include "GameProgress.h"
#include "InventoryManager.h"
#include "LocationManager.h"

HintManager gHintManager;

bool HintManager::IsHintAvailable() const
{
    return gGK3UI.IsOnDrivingScreen();
}

void HintManager::ShowHint()
{
    if(gGameProgress.GetTimeblock() == Timeblock(1, 14) || gGameProgress.GetTimeblock() == Timeblock(1, 16)) // 102P or 104P
    {
        // If haven't followed Wilkes and Buthane, hint at PLO (the parking lot at Cheateau de Blanchfort).
        // Only in 102P!
        if(gGameProgress.GetTimeblock() == Timeblock(1, 14))
        {
            if(gGameProgress.GetNounVerbCount("BUTHANE", "FOLLOW") == 0 || gGameProgress.GetNounVerbCount("WILKES", "FOLLOW") == 0)
            {
                gGK3UI.FlashDrivingScreenLocation("PLO");
            }
        }

        // If you haven't spied on Mosely and Butane at Chateau de Blanchfort, or introduced yourself to Wilkes, hint at PLO.
        // Only in 104P!
        bool stillHas104StuffToDo = false;
        if(gGameProgress.GetTimeblock() == Timeblock(1, 16))
        {
            if(gGameProgress.GetTopicCount("WILKES", "T_INTRODUCE") == 0 || gGameProgress.GetNounVerbCount("VIEW_OF_LHOMME_MORE", "BINOCULARS") == 0)
            {
                gGK3UI.FlashDrivingScreenLocation("PLO");
                stillHas104StuffToDo = true;
            }
        }

        // If you haven't talked to Larry about the Templars (x2), hint at his house.
        // For both 102P and 104P!
        if(gGameProgress.GetTopicCount("LARRY", "T_TEMPLARS") < 2)
        {
            gGK3UI.FlashDrivingScreenLocation("LHE");
            stillHas104StuffToDo = true;
        }

        // If you haven't investigated the train station yet, hint there too.
        // For both 102P and 104P!
        bool investigatedTrain = (gGameProgress.GetNounVerbCount("ARRIVALS_IN_CU", "THINK") > 0 || gGameProgress.GetTopicCount("MARCIE", "T_TRAIN_FROM_NAPLES") > 0);
        if(!investigatedTrain || gGameProgress.GetNounVerbCount("TAXI_DRIVER", "WALLET") < 2)
        {
            gGK3UI.FlashDrivingScreenLocation("TR1");
            stillHas104StuffToDo = true;
        }

        // If you've done everything for 104P, hint at Rennes-le-Chateau (you must return there to end the timeblock).
        if(gGameProgress.GetTimeblock() == Timeblock(1, 16) && !stillHas104StuffToDo)
        {
            gGK3UI.FlashDrivingScreenLocation("RLC");
        }
    }
    else if(gGameProgress.GetTimeblock() == Timeblock(1, 18)) // 106P
    {
        // Hints in this timeblock are all based on following Prince James' men.
        int twoMenState = gGameProgress.GetGameVariable("TwoMenState");
        if(twoMenState < 4)
        {
            // Men are still in RLC, flash there.
            gGK3UI.FlashDrivingScreenLocation("RLC");
        }
        else if(twoMenState == 4)
        {
            // You need to follow men's car, flash CDB parking lot.
            gGK3UI.FlashDrivingScreenLocation("PLO");
        }
        else if(twoMenState == 5)
        {
            // Men are at Larry's place - flash that.
            gGK3UI.FlashDrivingScreenLocation("LHE");
        }
        else if(twoMenState > 5)
        {
            // Men are in Larry's place - go back to RLC.
            gGK3UI.FlashDrivingScreenLocation("RLC");
        }
    }
    else if(gGameProgress.GetTimeblock() == Timeblock(2, 14)) // 202P
    {
        // This timeblock is split into a few distinct parts.
        // In the first part, you must investigate Devil's Armchair AND confront Larry about the dead men.

        // If haven't confronted Larry and discovered his alarm clock time, hint at Larry's house.
        // The game sets this variable to 6 when you've done it all.
        bool inFirstPartOfTimeblock = false;
        if(gGameProgress.GetGameVariable("FiveMinTimer202p") != 6)
        {
            gGK3UI.FlashDrivingScreenLocation("LHE");
            inFirstPartOfTimeblock = true;
        }

        // If haven't investigated Devil's Armchair, hint at that location.
        // The game considers it to be thoroughly investigated after you discover the knee prints and think on them a few times.
        if(gGameProgress.GetNounVerbCount("knee_indents", "think") < 2)
        {
            gGK3UI.FlashDrivingScreenLocation("ARM");
            inFirstPartOfTimeblock = true;
        }

        // Once you finish those two things, you can move on...
        if(!inFirstPartOfTimeblock)
        {
            // Estelle starts driving around the map at this time.
            // As long as she's there, hint at CDB parking lot so you can follow her.
            if(gLocationManager.IsActorAtLocation("ESTELLE", "MAP"))
            {
                gGK3UI.FlashDrivingScreenLocation("PLO");
            }

            // At this point, you need to go back to R25 to talk to Grace and Mosely. So, flash that spot.
            // The game sets this variable to 2 when you've finished talking with them.
            bool doneTalkingWithGraceAndMosely = true;
            if(gGameProgress.GetTopicCount("GRACE_N_MOSE", "T_BOOK") < 2)
            {
                gGK3UI.FlashDrivingScreenLocation("RLC");
                doneTalkingWithGraceAndMosely = false;
            }

            // Once done talking to Grace and Mosely, some more hints open up.
            if(doneTalkingWithGraceAndMosely)
            {
                // At this point, RLC is a hint until you ask Jean for a wake-up call and you print an ID.
                bool hasGoodFakeId = gInventoryManager.HasInventoryItem("FAKE_ID_REPORTER") && !gInventoryManager.HasInventoryItem("FAKE_ID_NYT_REP");
                if(gGameProgress.GetTopicCount("JEAN", "T_WAKEUPCALL") == 0 || !hasGoodFakeId)
                {
                    gGK3UI.FlashDrivingScreenLocation("RLC");
                }

                // Chateau de Serras is also a hint until you exhaust dialogue with Montreaux.
                bool finishedWithMontreaux = true;
                if(gGameProgress.GetTopicCount("MONTREAUX", "T_VITICULTURE") < 2)
                {
                    gGK3UI.FlashDrivingScreenLocation("CSE");
                    finishedWithMontreaux = false;
                }

                // If finished with Montreaux, the last thing to do is go back to R25 in RLC.
                if(finishedWithMontreaux)
                {
                    gGK3UI.FlashDrivingScreenLocation("RLC");
                }
            }
        }
    }
    else if(gGameProgress.GetTimeblock() == Timeblock(2, 2)) // 202A
    {
        // The hint logic is fairly straightforward for this timeblock.
        // If you don't have the manuscript, hint to go to CDB parking lot.
        // If you do, hint to go back to RLC.
        if(!gInventoryManager.HasInventoryItem("BLOODLINE_MANUSCRIPT"))
        {
            gGK3UI.FlashDrivingScreenLocation("PLO");
        }
        else
        {
            gGK3UI.FlashDrivingScreenLocation("RLC");
        }
    }
    else if(gGameProgress.GetTimeblock() == Timeblock(3, 7)) // 307A
    {
        // RLC is a hint for this entire timeblock.
        // This is because you must go back here to do the LSR puzzle pretty much for the entire timeblock.
        gGK3UI.FlashDrivingScreenLocation("RLC");

        // You also need to do some stuff at L'Ermitage. Use GPS and pick up clue note.
        // If you haven't done that, hint there.
        if(!gGameProgress.GetFlag("UseCoordLER") || gGameProgress.GetNounVerbCount("CLUE_NOTE_1", "PICKUP") == 0)
        {
            gGK3UI.FlashDrivingScreenLocation("LER");
        }
    }
    else if(gGameProgress.GetTimeblock() == Timeblock(3, 10)) // 310A
    {
        // This timeblock has two clear tasks to be done before you can move on.
        // First, you must talk to Larry.
        bool didTwoTasks = true;
        if(gGameProgress.GetTopicCount("LARRY", "T_MANUSCRIPT") == 0)
        {
            gGK3UI.FlashDrivingScreenLocation("LHE");
            didTwoTasks = false;
        }

        // Second, you must find Wilkes' dead body. :(
        if(gLocationManager.GetLocationCountForCurrentTimeblock("GABRIEL", "WDB") == 0)
        {
            gGK3UI.FlashDrivingScreenLocation("LHM");
            didTwoTasks = false;
        }

        // If did those two tasks, hint to go back to R25 and talk to Grace.
        if(didTwoTasks)
        {
            gGK3UI.FlashDrivingScreenLocation("RLC");
        }
    }
    else if(gGameProgress.GetTimeblock() == Timeblock(3, 12)) // 312P
    {
        // RLC is hinted for this entire timeblock.
        // You either must return here to continue LSR, or must return here to complete the timeblock by entering R25.
        gGK3UI.FlashDrivingScreenLocation("RLC");

        // The northeast & southwest hexagram points are hinted if you haven't picked up the notes at those locations yet.
        if(gGameProgress.GetNounVerbCount("CLUE_NOTE_2", "PICKUP") == 0)
        {
            gGK3UI.FlashDrivingScreenLocation("MCB");
        }
        if(gGameProgress.GetNounVerbCount("CLUE_NOTE_4", "PICKUP") == 0)
        {
            gGK3UI.FlashDrivingScreenLocation("BEC");
        }

        // The site is hinted if it's on the map and you haven't gotten the note yet.
        bool gotClueNoteFromSite = gGameProgress.GetNounVerbCount("CLUE_NOTE_3", "PICKUP") != 0;
        if(gGameProgress.GetFlag("MarkedTheSite") && !gotClueNoteFromSite)
        {
            gGK3UI.FlashDrivingScreenLocation("TRE");
        }

        // If got the clue note from the site, a few more hints open up.
        if(gotClueNoteFromSite)
        {
            // You need the binoculars at this point. If haven't gotten it, hint where you can get it.
            if(!gInventoryManager.HasInventoryItem("Binoculars"))
            {
                gGK3UI.FlashDrivingScreenLocation("WOD");
            }

            // If haven't spied on Buchelli yet, hint at going to CDB parking lot.
            if(gGameProgress.GetNounVerbCount("VIEW_OF_ORANGE_ROCK", "BINOCULARS") == 0)
            {
                gGK3UI.FlashDrivingScreenLocation("PLO");
            }
        }
    }
    else if(gGameProgress.GetTimeblock() == Timeblock(3, 15)) // 303P
    {
        // If you're playing well, Grace will have obtained the manuscript in the previous timeblock.
        // However, if you failed to do that, it's one of the first things Gabriel must do in this timeblock.
        bool hasBloodlineManuscript = gInventoryManager.HasInventoryItem("BLOODLINE_MANUSCRIPT");
        bool metWithPrinceJames = gLocationManager.GetLocationCountForCurrentTimeblock("GABRIEL", "BET") > 0;
        if(!hasBloodlineManuscript && !metWithPrinceJames)
        {
            gGK3UI.FlashDrivingScreenLocation("BMB");
        }

        // If you got the manuscript, but haven't yet talked to Prince James, that's your next destination. In RLC.
        if(hasBloodlineManuscript && !metWithPrinceJames)
        {
            gGK3UI.FlashDrivingScreenLocation("RLC");
        }

        // If you've met with Prince James, the last thing to do is confront Montreaux...
        if(metWithPrinceJames)
        {
            gGK3UI.FlashDrivingScreenLocation("CSE");
        }
    }
}
