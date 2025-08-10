#include "DrivingScreen.h"

#include <cassert>

#include "ActionManager.h"
#include "AssetManager.h"
#include "DrivingScreenBlip.h"
#include "GameProgress.h"
#include "GKPrefs.h"
#include "IniParser.h"
#include "LocationManager.h"
#include "SoundtrackPlayer.h"
#include "StringTokenizer.h"
#include "TextAsset.h"
#include "Texture.h"
#include "UIButton.h"
#include "UICanvas.h"
#include "UIImage.h"
#include "UIUtil.h"
#include "Window.h"

DrivingScreen::DrivingScreen() : Actor("DrivingScreen", TransformType::RectTransform)
{
    // Add fullscreen canvas with black background.
    // Driving screen should draw above scene transitioner so it doesn't appear on this screen.
    mCanvas = UI::AddCanvas(this, 5, Color32::Black);

    /*
    //TODO: This puts a blurry image behind the main map image for higher resolutions with black bars. Good or bad?
    UIImage* blurryBG = UI::CreateWidgetActor<UIImage>("MapBlurry", this);
    blurryBG->SetTexture(gAssetManager.LoadTexture("DM_BASE.BMP"));
    blurryBG->GetRectTransform()->SetAnchor(AnchorPreset::CenterStretch);
    blurryBG->SetColor(Color32(255, 255, 255, 50));
    */

    // Add map background image.
    mMapTexture = gAssetManager.LoadTexture("DM_BASE.BMP");
    mMapImage = UI::CreateWidgetActor<UIImage>("Map", this);
    mMapImage->SetTexture(mMapTexture, true);
    mMapActor = mMapImage->GetOwner();

    // Add location buttons.
    AddLocation("VGR", "ARM", Vector2(458.0f, -225.0f)); // Devil's Armchair
    AddLocation("BEC", "BEC", Vector2(94.0f, -400.0f));  // Southwest Arm of Hexagram
    AddLocation("PL3", "BMB", Vector2(499.0f, -137.0f)); // Orange Rock
    AddLocation("PL2", "CSD", Vector2(396.0f, -187.0f)); // Coumme Sourde
    AddLocation("PL6", "CSE", Vector2(520.0f, -4.0f));   // Chateau de Serres
    AddLocation("PL4", "LER", Vector2(387.0f, -258.0f)); // L'Ermitage
    AddLocation("LHE", "LHE", Vector2(454.0f, -65.0f));  // Larry's House
    AddLocation("PL1", "LHM", Vector2(442.0f, -155.0f)); // L'Homme Mort
    AddLocation("MCB", "MCB", Vector2(555.0f, -72.0f));  // Northeast Arm of Hexagram
    AddLocation("PLO", "PLO", Vector2(447.0f, -91.0f));  // Chateau de Blanchfort
    AddLocation("POU", "POU", Vector2(578.0f, -22.0f));  // Poussin's Tomb
    AddLocation("RL1", "RL1", Vector2(487.0f, -174.0f)); // Ren Le Bains
    AddLocation("MOP", "RLC", Vector2(193.0f, -119.0f)); // Rennes Le Chateau
    AddLocation("TR1", "TR1", Vector2(57.0f, -131.0f));  // Train Station
    AddLocation("TRE", "TRE", Vector2(506.0f, -89.0f));  // "The Site"
    AddLocation("PL5", "WOD", Vector2(44.0f, -218.0f));  // Estelle's & Lady Howard's Site

    // Load paths for blip movements.
    LoadPaths();

    // Precreate max number of blips we'd need at one time.
    for(int i = 0; i < 3; ++i)
    {
        mBlips.push_back(CreateBlip());
    }

    // Hide by default.
    SetActive(false);
}

DrivingScreen::~DrivingScreen()
{
    // We created this, so we must delete it.
    delete mBlipTexture;
}

void DrivingScreen::Show(FollowMode followMode)
{
    // Wherever we are right now, that's where we are driving from.
    // One exception is if location is already MAP (which occurs if timeblock screen shows over the map like in Day 1, 2PM).
    std::string location = gLocationManager.GetLocation();
    if(!StringUtil::EqualsIgnoreCase(location, "MAP"))
    {
        mDrivingFromLocation = gLocationManager.GetLocation();
    }

    // The map shows as a special location called "Map".
    // Unload the current scene and set the location as such.
    gLocationManager.ChangeLocation("MAP", [this, followMode](){

        // If changing to the map causes a timeblock change (such as at end of 102P), early out here.
        // The timeblock screen will show - we'll come back to the map screen after that!
        if(gGameProgress.IsChangingTimeblock())
        {
            return;
        }

        // Set which locations are present depending on game state.
        // Some locations are always available, some are only present after a certain timeblock, some require following someone.
        // LHE (Larry's Place) is always available.
        // PLO (Chataeu de Blanchefort Parking Lot) is always available.
        // RL1 (The Bar) is always available.
        // RLC (Rennes-le-Chataeu) is always available.
        // TR1 (Train Station) is always available.

        // ARM (Devil's Armchair) is only available on or after Day 2, 2pm.
        const Timeblock& currentTimeblock = gGameProgress.GetTimeblock();
        mLocationButtons["ARM"].button->SetEnabled(currentTimeblock >= Timeblock(2, 2, Timeblock::PM) || currentTimeblock == Timeblock(2, 2));

        // CSE (Chateau de Serras) only available after going on the tour on Day 2, Noon.
        mLocationButtons["CSE"].button->SetEnabled(currentTimeblock >= Timeblock(2, 12, Timeblock::PM) || currentTimeblock == Timeblock(2, 2));

        // POU (Poussin's Tomb) is available after the tour on Day 2, 7am.
        mLocationButtons["POU"].button->SetEnabled(currentTimeblock >= Timeblock(2, 7, Timeblock::AM) || currentTimeblock == Timeblock(2, 2));

        // BEC (Bottom of Hexagram) is only at the beginning of Day 3, 12PM (even though you discover the coordinates in Day 3, 7AM).
        // MCB (Top of Hexagram) is the same.
        mLocationButtons["BEC"].button->SetEnabled(currentTimeblock >= Timeblock(3, 12));
        mLocationButtons["MCB"].button->SetEnabled(currentTimeblock >= Timeblock(3, 12));

        // TRE (Treasure) is only available after you've marked it on the map in Sidney.
        mLocationButtons["TRE"].button->SetEnabled(gGameProgress.GetFlag("MarkedTheSite"));

        // BMB (Red Rock) is only available after spying on Buchelli from Blanchefort.
        mLocationButtons["BMB"].button->SetEnabled(gGameProgress.GetNounVerbCount("VIEW_OF_ORANGE_ROCK", "BINOCULARS") > 0 || currentTimeblock >= Timeblock(3, 3, Timeblock::PM));

        // CSD (Coume Sourde) only available after following Madeline (or on/after Day 1, 4PM).
        // LHM (Lhomme Mort) is in the same boat.
        bool showCSD_LHM = gGameProgress.GetNounVerbCount("BUTHANE", "FOLLOW") > 0 && followMode != FollowMode::Buthane;
        showCSD_LHM = showCSD_LHM || currentTimeblock >= Timeblock(1, 4, Timeblock::PM);
        mLocationButtons["CSD"].button->SetEnabled(showCSD_LHM);
        mLocationButtons["LHM"].button->SetEnabled(showCSD_LHM);

        // LER (L'Ermitage) only available after following Wilkes (or on/after Day 1, 4PM).
        bool showLER = gGameProgress.GetNounVerbCount("WILKES", "FOLLOW") > 0 && followMode != FollowMode::Wilkes;
        showLER = showLER || currentTimeblock >= Timeblock(1, 4, Timeblock::PM);
        mLocationButtons["LER"].button->SetEnabled(showLER);

        // WOD (Woods) only available after following Estelle on Day 2.
        bool showWOD = gGameProgress.GetNounVerbCount("ESTELLE", "FOLLOW") > 0 && followMode != FollowMode::Estelle;
        showWOD = showWOD || currentTimeblock >= Timeblock(2, 5, Timeblock::PM);
        mLocationButtons["WOD"].button->SetEnabled(showWOD);

        // Actually show the UI.
        SetActive(true);
        RefreshUIScaling();

        // Put all blips in starting positions, with paths set if needed.
        mFollowMode = followMode;
        PlaceBlips(followMode);

        // Add soundtrack player, if not yet present.
        SoundtrackPlayer* soundtrackPlayer = GetComponent<SoundtrackPlayer>();
        if(soundtrackPlayer == nullptr)
        {
            soundtrackPlayer = AddComponent<SoundtrackPlayer>();
        }

        // Play map music.
        // The game's files define a number of map soundtrack variants, based on day and ego and who you're following.
        // But at the end of the day, only these two appear to have ever been used.
        if(followMode == FollowMode::None)
        {
            if(currentTimeblock == Timeblock(2, 2))
            {
                soundtrackPlayer->Play(gAssetManager.LoadSoundtrack("MAP302A.STK"));
            }
            else
            {
                soundtrackPlayer->Play(gAssetManager.LoadSoundtrack("MAPGABEGENDAY1.STK"));
            }
        }
        else
        {
            soundtrackPlayer->Play(gAssetManager.LoadSoundtrack("MAPFOLLOWGEN.STK"));
        }

        // Also play motorcycle driving away SFX.
        soundtrackPlayer->Play(gAssetManager.LoadSoundtrack("MAPHARLEYAWAY.STK"));



        // When Gabe attempts to follow the Black Sedan during 202A timeblock, some dialogue plays over the map screen.
        // This doesn't seem to be accounted for in any NVC or Sheepscript, so it must be hardcoded...
        if(currentTimeblock == Timeblock(2, 2) && gGameProgress.GetNounVerbCount("CAR", "FOLLOW") == 1)
        {
            // Wait for any existing action or action skip to finish before executing this dialogue.
            gActionManager.WaitForActionsToComplete([](){
                gActionManager.ExecuteDialogueAction("2136L3W3K1", 2);

                // Increment this noun/verb count to avoid playing this dialogue on the map again.
                //TODO: This is not how the original game accomplished this...I don't know how they did it!
                gGameProgress.IncNounVerbCount("CAR", "FOLLOW");
            });
        }
    });
}

void DrivingScreen::Hide()
{
    if(!IsActive()) { return; }
    SetActive(false);

    // Stop map music, but play motorcycle arrival sound.
    SoundtrackPlayer* soundtrackPlayer = GetComponent<SoundtrackPlayer>();
    if(soundtrackPlayer != nullptr)
    {
        soundtrackPlayer->StopAll();
        soundtrackPlayer->Play(gAssetManager.LoadSoundtrack("MAPHARLEYARRIVE.STK"));
    }

    // Clear follow mode.
    mFollowMode = FollowMode::None;

    // Clear flash state.
    for(LocationButton* lb : mFlashingButtons)
    {
        lb->button->SetUpTexture(lb->upTexture);
    }
    mFlashingButtons.clear();
    mFlashActive = false;
    mFlashCounter = 0;
    mFlashTimer = 0.0f;
}

void DrivingScreen::Cancel()
{
    // Unlike simply hiding, this goes back to the last location you were at - cancel driving anywhere.
    ExitToLocation(gLocationManager.GetLastLocation());
}

void DrivingScreen::FlashLocation(const std::string& locationCode)
{
    auto it = mLocationButtons.find(locationCode);
    if(it != mLocationButtons.end())
    {
        bool addedFirstItem = mFlashingButtons.empty();
        mFlashingButtons.push_back(&it->second);

        if(addedFirstItem)
        {
            mFlashTimer = kFlashInterval;
            mFlashCounter = 10;
            mFlashActive = true;
        }

        if(mFlashActive)
        {
            it->second.button->SetUpTexture(mFlashActive ? it->second.hoverTexture : it->second.upTexture);
        }
    }
}

void DrivingScreen::OnUpdate(float deltaTime)
{
    // If the flash timer has time on it, decrement based on delta time.
    if(mFlashTimer > 0.0f)
    {
        mFlashTimer -= deltaTime;
        if(mFlashTimer <= 0.0f)
        {
            // When the timer expires, flip the flash and decrement counter for how many times to do a flash.
            mFlashActive = !mFlashActive;
            --mFlashCounter;

            // If out of flashes, turn of the flash system.
            // Otherwise, refresh the flash timer.
            if(mFlashCounter <= 0)
            {
                mFlashActive = false;
            }
            else
            {
                mFlashTimer = kFlashInterval;
            }

            // Update each flashing button's state based on whether flash is active or not.
            for(LocationButton* lb : mFlashingButtons)
            {
                lb->button->SetUpTexture(mFlashActive ? lb->hoverTexture : lb->upTexture);
            }

            // Once done flashing, clear the set of flashing buttons.
            if(mFlashCounter <= 0)
            {
                mFlashingButtons.clear();
            }
        }
    }
}

void DrivingScreen::SetLocationButtonsInteractive(bool interactive)
{
    // This doesn't hide the buttons; it just makes them interactive or not.
    for(auto& button : mLocationButtons)
    {
        button.second.button->SetCanInteract(interactive);
    }
}

void DrivingScreen::ExitToLocation(const std::string& locationCode)
{
    // Change to the desired location.
    gLocationManager.ChangeLocation(locationCode, [this](){

        // Hide the map screen after scene load is done.
        Hide();
    });
}

void DrivingScreen::AddLocation(const std::string& locationCode, const std::string& buttonId, const Vector2& buttonPos)
{
    // Create button.
    UIButton* button = UI::CreateWidgetActor<UIButton>(buttonId, mMapActor);

    // Due to how the map image scales with resolution, this seems to avoid some "off by one" pixel errors at some scales.
    button->GetRectTransform()->SetPixelPerfect(false);

    // Set textures.
    Texture* upTexture = gAssetManager.LoadTexture("DM_" + buttonId + "_UL.BMP");
    Texture* hoverTexture = gAssetManager.LoadTexture("DM_" + buttonId + ".BMP");
    button->SetUpTexture(upTexture);
    button->SetDownTexture(hoverTexture);
    button->SetHoverTexture(hoverTexture);

    // The map scales up or down depending on window resolution.
    // In order for the buttons to appear at the correct positions and scales, we need to do some fancy anchoring...
    {
        float mapWidth = static_cast<float>(mMapTexture->GetWidth());
        float mapHeight = static_cast<float>(mMapTexture->GetHeight());

        float buttonWidth = static_cast<float>(upTexture->GetWidth());
        float buttonHeight = static_cast<float>(upTexture->GetHeight());

        // Min anchors are at bottom-left of screen, max anchors top-right.
        // Unfortunately, the passed-in position is top-left corner...
        float buttonPosBottomLeft = mapHeight - Math::Abs(buttonPos.y) - buttonHeight;

        // Calculate & set anchors.
        float anchorMinX = buttonPos.x / mapWidth;
        float anchorMinY = buttonPosBottomLeft / mapHeight;

        float anchorMaxX = (buttonPos.x + buttonWidth) / mapWidth;
        float anchorMaxY = (buttonPosBottomLeft + buttonHeight) / mapHeight;
        button->GetRectTransform()->SetAnchorMin(Vector2(anchorMinX, anchorMinY));
        button->GetRectTransform()->SetAnchorMax(Vector2(anchorMaxX, anchorMaxY));

        // Min/max anchors control the size, so we don't want any extra x/y size padding.
        button->GetRectTransform()->SetSizeDelta(Vector2::Zero);
    }

    // Play sound effect when hovering these buttons.
    button->SetHoverSound(gAssetManager.LoadAudio("MAPSWOOSH.WAV"));

    // Add tooltip.
    button->SetTooltipText("dm_" + buttonId);

    // On press, go to the map location.
    button->SetPressCallback([this, locationCode](UIButton* button){
        OnLocationButtonPressed(locationCode);
    });

    // Cache button for later.
    LocationButton lb;
    lb.button = button;
    lb.upTexture = upTexture;
    lb.hoverTexture = hoverTexture;
    mLocationButtons[buttonId] = lb;
}

void DrivingScreen::LoadPaths()
{
    // Load path data.
    TextAsset* pathData = gAssetManager.LoadText("PATHDATA.TXT", AssetScope::Manual);

    // Pass 1: Read in nodes and segments.
    {
        IniKeyValue kv;
        IniParser parser(pathData->GetText(), pathData->GetTextLength());
        while(parser.ReadLine())
        {
            StringTokenizer tokenizer(parser.GetLine(), { ' ' });
            if(!tokenizer.HasNext()) { continue; }

            const std::string& token = tokenizer.GetNext();
            if(StringUtil::EqualsIgnoreCase(token, "NodeBegin"))
            {
                assert(tokenizer.HasNext());
                mPathData.nodes.emplace_back();
                mPathData.nodes.back().name = tokenizer.GetNext();
            }
            else if(StringUtil::EqualsIgnoreCase(token, "Location"))
            {
                assert(tokenizer.HasNext());
                kv.value = tokenizer.GetNext();
                mPathData.nodes.back().point = kv.GetValueAsVector2();
            }
            else if(StringUtil::EqualsIgnoreCase(token, "SegmentBegin"))
            {
                assert(tokenizer.HasNext());
                mPathData.segments.emplace_back();
                mPathData.segments.back().name = tokenizer.GetNext();
            }
            else if(StringUtil::EqualsIgnoreCase(token, "Point"))
            {
                assert(tokenizer.HasNext());
                kv.value = tokenizer.GetNext();
                mPathData.segments.back().points.emplace_back(kv.GetValueAsVector2());
            }
        }
    }

    // Pass 2: Read in connections.
    {
        int nodeIndex = 0;
        bool inLinks = false;

        IniParser parser(pathData->GetText(), pathData->GetTextLength());
        while(parser.ReadLine())
        {
            StringTokenizer tokenizer(parser.GetLine(), { ' ' });
            if(!tokenizer.HasNext()) { continue; }

            const std::string& token = tokenizer.GetNext();
            if(StringUtil::EqualsIgnoreCase(token, "LinksBegin"))
            {
                inLinks = true;
            }
            else if(StringUtil::EqualsIgnoreCase(token, "LinksEnd"))
            {
                inLinks = false;
                ++nodeIndex;
            }
            else if(inLinks)
            {
                mPathData.nodes[nodeIndex].connections.emplace_back();
                for(auto& node : mPathData.nodes)
                {
                    if(node.name == token)
                    {
                        mPathData.nodes[nodeIndex].connections.back().to = &node;
                        break;
                    }
                }

                assert(tokenizer.HasNext());
                const std::string& segmentName = tokenizer.GetNext();
                for(auto& segment : mPathData.segments)
                {
                    if(segment.name == segmentName)
                    {
                        mPathData.nodes[nodeIndex].connections.back().segment = &segment;
                        break;
                    }
                }

                assert(tokenizer.HasNext());
                mPathData.nodes[nodeIndex].connections.back().forward = StringUtil::ToBool(tokenizer.GetNext());
            }
        }
    }

    // Don't need path data asset anymore.
    delete pathData;
}

DrivingScreenBlip* DrivingScreen::CreateBlip()
{
    // Create blip texture. This is a 6x6 white dot, corner pixels alpha'd.
    if(mBlipTexture == nullptr)
    {
        mBlipTexture = new Texture(6, 6, Color32::White);
        mBlipTexture->SetPixelColor(0, 0, Color32::Clear);
        mBlipTexture->SetPixelColor(1, 0, Color32::Clear);
        mBlipTexture->SetPixelColor(0, 1, Color32::Clear);

        mBlipTexture->SetPixelColor(5, 0, Color32::Clear);
        mBlipTexture->SetPixelColor(4, 0, Color32::Clear);
        mBlipTexture->SetPixelColor(5, 1, Color32::Clear);

        mBlipTexture->SetPixelColor(5, 5, Color32::Clear);
        mBlipTexture->SetPixelColor(4, 5, Color32::Clear);
        mBlipTexture->SetPixelColor(5, 4, Color32::Clear);

        mBlipTexture->SetPixelColor(0, 5, Color32::Clear);
        mBlipTexture->SetPixelColor(1, 5, Color32::Clear);
        mBlipTexture->SetPixelColor(0, 4, Color32::Clear);
    }

    // Create blip as child of the map.
    DrivingScreenBlip* blip = new DrivingScreenBlip(mPathData);
    blip->GetTransform()->SetParent(mMapActor->GetTransform());

    // Add image and use the blip texture.
    UIImage* blipImage = blip->AddComponent<UIImage>();
    blipImage->SetTexture(mBlipTexture, true);
    blip->SetImage(blipImage);
    return blip;
}

void DrivingScreen::PlaceBlips(FollowMode followMode)
{
    // Determine scale factor for the map image, now that it's been scaled up/down...
    Vector2 mapImageSize = mMapImage->GetRectTransform()->GetSize();
    Vector2 mapScale(mapImageSize.x / mMapTexture->GetWidth(), mapImageSize.y / mMapTexture->GetHeight());

    // Let all the blips know the map scale.
    for(auto& blip : mBlips)
    {
        blip->SetMapScale(mapScale);
    }

    // Blip 0 will always be the player.
    mBlips[kEgoIndex]->SetActive(true);
    mBlips[kEgoIndex]->SetColor(kEgoColor);
    mBlips[kEgoIndex]->SetBlinkEnabled(true);
    mBlips[kEgoIndex]->ClearPath();

    // Start player at current location. During normal play, this'll always be a node that exists on the map.
    // When debugging and such, you might show the driving interface at some unusual spot. We'll fall back on showing player at MOP in that case.
    mBlips[kEgoIndex]->SetMapPosition("MOP");
    mBlips[kEgoIndex]->SetMapPosition(mDrivingFromLocation);

    // The other blips are only present in some circumstances. Deactivate them by default.
    mBlips[kNpc1Index]->SetActive(false);
    mBlips[kNpc2Index]->SetActive(false);
    mBlips[kNpc1Index]->ClearPath();
    mBlips[kNpc2Index]->ClearPath();

    // "None" follow mode - we aren't actively following anyone.
    // The only blips that appear in this case are the idle loops of some characters depending on the timeblock.
    if(followMode == FollowMode::None)
    {
        if(gGameProgress.GetTimeblock() == Timeblock(1, 2, Timeblock::PM))
        {
            // Buthane drives in circles, starting at PL3.
            // But she no longer appears on the map after LHM is enabled.
            if(!mLocationButtons["LHM"].button->IsEnabled())
            {
                mBlips[kNpc1Index]->SetActive(true);
                mBlips[kNpc1Index]->SetColor(kButhaneColor);
                mBlips[kNpc1Index]->AddPathNode("PL3");
                mBlips[kNpc1Index]->AddPathNode("RL1");
                mBlips[kNpc1Index]->AddPathNode("IN4");
                mBlips[kNpc1Index]->AddPathNode("VGR");
                mBlips[kNpc1Index]->AddPathNode("PL4");
                mBlips[kNpc1Index]->AddPathNode("BEC");
                mBlips[kNpc1Index]->AddPathNode("PL5");
                mBlips[kNpc1Index]->AddPathNode("TR1");
                mBlips[kNpc1Index]->AddPathNode("IN1");
                mBlips[kNpc1Index]->AddPathNode("LHE");
                mBlips[kNpc1Index]->AddPathNode("PLO");
                mBlips[kNpc1Index]->SetLoopPath(true);
            }

            // Wilkes drives in circles, starting at LHE.
            // But he no longer appears on the map after LER is enabled.
            if(!mLocationButtons["LER"].button->IsEnabled())
            {
                mBlips[kNpc2Index]->SetActive(true);
                mBlips[kNpc2Index]->SetColor(kWilkesColor);
                mBlips[kNpc2Index]->AddPathNode("LHE");
                mBlips[kNpc2Index]->AddPathNode("PLO");
                mBlips[kNpc2Index]->AddPathNode("PL3");
                mBlips[kNpc2Index]->AddPathNode("RL1");
                mBlips[kNpc2Index]->AddPathNode("IN4");
                mBlips[kNpc2Index]->AddPathNode("VGR");
                mBlips[kNpc2Index]->AddPathNode("PL4");
                mBlips[kNpc2Index]->AddPathNode("BEC");
                mBlips[kNpc2Index]->AddPathNode("PL5");
                mBlips[kNpc2Index]->AddPathNode("TR1");
                mBlips[kNpc2Index]->AddPathNode("IN1");
                mBlips[kNpc2Index]->SetLoopPath(true);
            }
        }
        else if(gGameProgress.GetTimeblock() == Timeblock(1, 4, Timeblock::PM))
        {
            // Lady Howard & Estelle drive in circles, starting at PLO.
            // They never disappear during the timeblock.
            mBlips[kNpc1Index]->SetActive(true);
            mBlips[kNpc1Index]->SetColor(kEstelleColor);
            mBlips[kNpc1Index]->AddPathNode("PLO");
            mBlips[kNpc1Index]->AddPathNode("PL3");
            mBlips[kNpc1Index]->AddPathNode("RL1");
            mBlips[kNpc1Index]->AddPathNode("IN4");
            mBlips[kNpc1Index]->AddPathNode("VGR");
            mBlips[kNpc1Index]->AddPathNode("PL4");
            mBlips[kNpc1Index]->AddPathNode("BEC");
            mBlips[kNpc1Index]->AddPathNode("PL5");
            mBlips[kNpc1Index]->AddPathNode("TR1");
            mBlips[kNpc1Index]->AddPathNode("IN1");
            mBlips[kNpc1Index]->AddPathNode("LHE");
            mBlips[kNpc1Index]->SetLoopPath(true);
        }
        else if(gGameProgress.GetTimeblock() == Timeblock(1, 6, Timeblock::PM))
        {
            // Mallory & MacDougall drive in circles, COUNTER-CLOCKWISE, starting at LHE.
            // They no longer appear after they've been followed.
            if(gGameProgress.GetNounVerbCount("TWO_MEN", "FOLLOW") == 0)
            {
                mBlips[kNpc1Index]->SetActive(true);
                mBlips[kNpc1Index]->SetColor(kJamesMenColor);
                mBlips[kNpc1Index]->AddPathNode("LHE");
                mBlips[kNpc1Index]->AddPathNode("IN1");
                mBlips[kNpc1Index]->AddPathNode("TR1");
                mBlips[kNpc1Index]->AddPathNode("PL5");
                mBlips[kNpc1Index]->AddPathNode("BEC");
                mBlips[kNpc1Index]->AddPathNode("PL4");
                mBlips[kNpc1Index]->AddPathNode("VGR");
                mBlips[kNpc1Index]->AddPathNode("IN4");
                mBlips[kNpc1Index]->AddPathNode("RL1");
                mBlips[kNpc1Index]->AddPathNode("PL3");
                mBlips[kNpc1Index]->AddPathNode("PLO");
                mBlips[kNpc1Index]->SetLoopPath(true);
            }
        }
        else if(gGameProgress.GetTimeblock() == Timeblock(2, 2, Timeblock::PM))
        {
            // Lady Howard & Estelle drive in circles, starting at PL3.
            if(gLocationManager.IsActorAtLocation("ESTELLE", "MAP"))
            {
                mBlips[kNpc1Index]->SetActive(true);
                mBlips[kNpc1Index]->SetColor(kEstelleColor);
                mBlips[kNpc1Index]->AddPathNode("PL3");
                mBlips[kNpc1Index]->AddPathNode("RL1");
                mBlips[kNpc1Index]->AddPathNode("IN4");
                mBlips[kNpc1Index]->AddPathNode("VGR");
                mBlips[kNpc1Index]->AddPathNode("PL4");
                mBlips[kNpc1Index]->AddPathNode("BEC");
                mBlips[kNpc1Index]->AddPathNode("PL5");
                mBlips[kNpc1Index]->AddPathNode("TR1");
                mBlips[kNpc1Index]->AddPathNode("IN1");
                mBlips[kNpc1Index]->AddPathNode("LHE");
                mBlips[kNpc1Index]->AddPathNode("PLO");
                mBlips[kNpc1Index]->SetLoopPath(true);
            }
        }
    }
    else // We're following someone!
    {
        // Don't allow pressing buttons until we're done following.
        SetLocationButtonsInteractive(false);

        // This does not match the original game, but I really prefer if the ego blip DOES NOT blink during follow mode.
        mBlips[kEgoIndex]->SetBlinkEnabled(false);

        // Whoever we follow is always NPC 1.
        mBlips[kNpc1Index]->SetActive(true);

        // Set up the particular follow path based on who we are following...
        if(followMode == FollowMode::Buthane)
        {
            // Buthane will stop at PL2. Define her ENTIRE route here...
            mBlips[kNpc1Index]->SetColor(kButhaneColor);
            mBlips[kNpc1Index]->AddPathNode("VGR");
            mBlips[kNpc1Index]->AddPathNode("PL4");
            mBlips[kNpc1Index]->AddPathNode("BEC");
            mBlips[kNpc1Index]->AddPathNode("PL5");
            mBlips[kNpc1Index]->AddPathNode("TR1");
            mBlips[kNpc1Index]->AddPathNode("IN1");
            mBlips[kNpc1Index]->AddPathNode("LHE");
            mBlips[kNpc1Index]->AddPathNode("PLO");
            mBlips[kNpc1Index]->AddPathNode("PL3");
            mBlips[kNpc1Index]->AddPathNode("RL1");
            mBlips[kNpc1Index]->AddPathNode("IN4");
            mBlips[kNpc1Index]->AddPathNode("IN3");
            mBlips[kNpc1Index]->AddPathNode("PL2");

            // Expose two new locations on the map once we're done following Buthane.
            // Also play a little piece of dialogue.
            mBlips[kNpc1Index]->SetPathCompleteCallback([this](){
                OnFollowDone();
                mLocationButtons["CSD"].button->SetEnabled(true);
                mLocationButtons["LHM"].button->SetEnabled(true);
                gActionManager.ExecuteSheepAction("wait StartDialogue(\"2196L3WL71\", 1)");
                gLocationManager.SetActorLocation("BUTHANE", "CSD");
            });
        }
        else if(followMode == FollowMode::Wilkes)
        {
            // Wilkes will stop at PL4. But he always starts from PL0.
            mBlips[kNpc1Index]->SetColor(kWilkesColor);
            mBlips[kNpc1Index]->AddPathNode("PLO");
            mBlips[kNpc1Index]->AddPathNode("PL3");
            mBlips[kNpc1Index]->AddPathNode("RL1");
            mBlips[kNpc1Index]->AddPathNode("IN4");
            mBlips[kNpc1Index]->AddPathNode("VGR");
            mBlips[kNpc1Index]->AddPathNode("PL4");

            // Expose one new location on the map once we're done following Wilkes.
            // Also play a little piece of dialogue.
            mBlips[kNpc1Index]->SetPathCompleteCallback([this](){
                OnFollowDone();
                mLocationButtons["LER"].button->SetEnabled(true);
                gActionManager.ExecuteSheepAction("wait StartDialogue(\"2196L3WLM1\", 1)");
                gLocationManager.SetActorLocation("WILKES", "LER");
            });
        }
        else if(followMode == FollowMode::LadyHoward)
        {
            mBlips[kNpc1Index]->SetColor(kEstelleColor);

            // So, there are two spots we can follow Lady Howard from, and in both cases, we want to do a complete circuit :P
            // Let's write this out...
            if(StringUtil::EqualsIgnoreCase(mDrivingFromLocation, "PLO"))
            {
                mBlips[kNpc1Index]->AddPathNode("PLO");
                mBlips[kNpc1Index]->AddPathNode("PL3");
                mBlips[kNpc1Index]->AddPathNode("RL1");
                mBlips[kNpc1Index]->AddPathNode("IN4");
                mBlips[kNpc1Index]->AddPathNode("VGR");
                mBlips[kNpc1Index]->AddPathNode("PL4");
                mBlips[kNpc1Index]->AddPathNode("BEC");
                mBlips[kNpc1Index]->AddPathNode("PL5");
                mBlips[kNpc1Index]->AddPathNode("TR1");
                mBlips[kNpc1Index]->AddPathNode("IN1");
                mBlips[kNpc1Index]->AddPathNode("LHE");
                mBlips[kNpc1Index]->AddPathNode("PLO");
            }
            else // This should be from PL4, but let's use it as a general fallback.
            {
                mBlips[kNpc1Index]->AddPathNode("PL4");
                mBlips[kNpc1Index]->AddPathNode("BEC");
                mBlips[kNpc1Index]->AddPathNode("PL5");
                mBlips[kNpc1Index]->AddPathNode("TR1");
                mBlips[kNpc1Index]->AddPathNode("IN1");
                mBlips[kNpc1Index]->AddPathNode("LHE");
                mBlips[kNpc1Index]->AddPathNode("PLO");
                mBlips[kNpc1Index]->AddPathNode("PL3");
                mBlips[kNpc1Index]->AddPathNode("RL1");
                mBlips[kNpc1Index]->AddPathNode("IN4");
                mBlips[kNpc1Index]->AddPathNode("VGR");
                mBlips[kNpc1Index]->AddPathNode("PL4");
            }

            // Just play a piece of dialogue after following the path and go back to previous location.
            mBlips[kNpc1Index]->SetPathCompleteCallback([this](){
                OnFollowDone();

                // Play dialogue ("they're just driving around the valley - forget that").
                gActionManager.ExecuteSheepAction("wait StartDialogue(\"21A6L3WLX1\", 1)", [this](const Action* action){
                    // This one automatically puts you back where you started.
                    ExitToLocation(mDrivingFromLocation);
                });
            });
        }
        else if(followMode == FollowMode::PrinceJamesMen)
        {
            mBlips[kNpc1Index]->SetColor(kJamesMenColor);

            // This one's got a couple possible follow locations:
            // If from PLO or LER, we use one path.
            // If from RLC, we use another path. We can assume this is the "else" case.
            if(StringUtil::EqualsIgnoreCase(mDrivingFromLocation, "PLO") || StringUtil::EqualsIgnoreCase(mDrivingFromLocation, "LER"))
            {
                // Do a complete circuit starting at LHE.
                mBlips[kNpc1Index]->AddPathNode("LHE");
                mBlips[kNpc1Index]->AddPathNode("IN1");
                mBlips[kNpc1Index]->AddPathNode("TR1");
                mBlips[kNpc1Index]->AddPathNode("PL5");
                mBlips[kNpc1Index]->AddPathNode("BEC");
                mBlips[kNpc1Index]->AddPathNode("PL4");
                mBlips[kNpc1Index]->AddPathNode("VGR");
                mBlips[kNpc1Index]->AddPathNode("IN4");
                mBlips[kNpc1Index]->AddPathNode("RL1");
                mBlips[kNpc1Index]->AddPathNode("PL3");
                mBlips[kNpc1Index]->AddPathNode("PLO");
                mBlips[kNpc1Index]->AddPathNode("LHE");
            }
            else
            {
                // Do a circuit starting at RLC.
                mBlips[kNpc1Index]->AddPathNode("MOP");
                mBlips[kNpc1Index]->AddPathNode("IN2");
                mBlips[kNpc1Index]->AddPathNode("IN1");
                mBlips[kNpc1Index]->AddPathNode("TR1");
                mBlips[kNpc1Index]->AddPathNode("PL5");
                mBlips[kNpc1Index]->AddPathNode("BEC");
                mBlips[kNpc1Index]->AddPathNode("PL4");
                mBlips[kNpc1Index]->AddPathNode("VGR");
                mBlips[kNpc1Index]->AddPathNode("IN4");
                mBlips[kNpc1Index]->AddPathNode("RL1");
                mBlips[kNpc1Index]->AddPathNode("PL3");
                mBlips[kNpc1Index]->AddPathNode("PLO");
                mBlips[kNpc1Index]->AddPathNode("LHE");
            }

            // Play dialogue once we reach LHE.
            mBlips[kNpc1Index]->SetPathCompleteCallback([this](){
                OnFollowDone();
                gActionManager.ExecuteSheepAction("wait StartDialogue(\"21F6L3WBH1\", 1)");
            });
        }
        else if(followMode == FollowMode::Estelle)
        {
            mBlips[kNpc1Index]->SetColor(kEstelleColor);

            // Estelle follows a circuit from TR1 to PL5.
            mBlips[kNpc1Index]->AddPathNode("TR1");
            mBlips[kNpc1Index]->AddPathNode("IN1");
            mBlips[kNpc1Index]->AddPathNode("LHE");
            mBlips[kNpc1Index]->AddPathNode("PLO");
            mBlips[kNpc1Index]->AddPathNode("PL3");
            mBlips[kNpc1Index]->AddPathNode("RL1");
            mBlips[kNpc1Index]->AddPathNode("IN4");
            mBlips[kNpc1Index]->AddPathNode("VGR");
            mBlips[kNpc1Index]->AddPathNode("PL4");
            mBlips[kNpc1Index]->AddPathNode("BEC");
            mBlips[kNpc1Index]->AddPathNode("PL5");

            // Play dialogue and show new location once we reach PL5.
            mBlips[kNpc1Index]->SetPathCompleteCallback([this](){
                OnFollowDone();
                mLocationButtons["WOD"].button->SetEnabled(true);
                gActionManager.ExecuteSheepAction("wait StartDialogue(\"21K6L3WJI1\", 1)");
                gLocationManager.SetActorLocation("ESTELLE", "WOD");
            });
        }

        // On the path defined for the NPC, always skip to wherever we are along that route, so it looks like we followed from the correct spot.
        mBlips[kNpc1Index]->SkipToPathNode(mDrivingFromLocation);

        // Ego follows this blip.
        mBlips[kEgoIndex]->SetFollow(mBlips[kNpc1Index]);
    }
}

void DrivingScreen::OnFollowDone()
{
    // We can interact with buttons again.
    SetLocationButtonsInteractive(true);

    // Blink ego blip again.
    mBlips[kEgoIndex]->SetBlinkEnabled(true);
}

void DrivingScreen::OnLocationButtonPressed(const std::string& locationCode)
{
    // Grace doesn't like to go to the train station. Says there's nothing of interest there.
    if(locationCode == "TR1")
    {
        if(gGameProgress.GetTimeblock() == Timeblock(3, 7) ||
           gGameProgress.GetTimeblock() == Timeblock(3, 12))
        {
            gActionManager.ExecuteDialogueAction("21LP362PF1", 1);
            return;
        }
    }

    // Play button sound effect.
    gAudioManager.PlaySFX(gAssetManager.LoadAudio("MAPBUTTON.WAV"));

    // Check conditions under which we would NOT allow going to this location.
    // Don't allow going to Larry's place during timeblock 106P and 202A.
    if(locationCode == "LHE")
    {
        if(gGameProgress.GetTimeblock() == Timeblock(1, 6, Timeblock::PM) ||
           gGameProgress.GetTimeblock() == Timeblock(2, 2, Timeblock::AM))
        {
            gActionManager.ExecuteSheepAction("wait StartDialogue(\"21F4F625S1\", 1)");
            return;
        }
    }

    // The location code assigned to the map button is not always the location we actually go to.
    // For one, attempting to go to the "Treasure Site" actually goes to PLO!
    std::string realLocationCode = locationCode;
    if(locationCode == "TRE")
    {
        realLocationCode = "PLO";
    }

    // Whenever you go to a location on the driving map, it's assumed that your bike also moves there.
    // Each location has it's own integer code (of course) that is rather arbitrary.
    int bikeLocation = -1;
    if(realLocationCode == "PL5")
    {
        bikeLocation = 0;
    }
    else if(realLocationCode == "PL4")
    {
        bikeLocation = 1;
    }
    else if(realLocationCode == "VGR")
    {
        bikeLocation = 2;
    }
    else if(realLocationCode == "PL2")
    {
        bikeLocation = 3;
    }
    else if(realLocationCode == "PL1")
    {
        bikeLocation = 4;
    }
    else if(realLocationCode == "PL3")
    {
        bikeLocation = 5;
    }
    else if(realLocationCode == "BEC")
    {
        bikeLocation = 6;
    }
    else if(realLocationCode == "MCB")
    {
        bikeLocation = 7;
    }
    else if(realLocationCode == "POU")
    {
        bikeLocation = 8;
    }
    else if(realLocationCode == "PL6")
    {
        // One complication: Cheateau de Serres sometimes has its gate opened.
        // In which case, the bike is parked inside the gate.
        if(gGameProgress.GetTimeblock() == Timeblock("202P") || gGameProgress.GetTimeblock() == Timeblock("212P"))
        {
            bikeLocation = 15; // inside the gate
            realLocationCode = "CSE";
        }
        else
        {
            bikeLocation = 9; // outside the gate
        }
    }
    else if(realLocationCode == "MOP")
    {
        bikeLocation = 10;
    }
    else if(realLocationCode == "LHE")
    {
        bikeLocation = 11;
    }
    else if(realLocationCode == "PLO")
    {
        bikeLocation = 12;
    }
    else if(realLocationCode == "RL1")
    {
        bikeLocation = 13;
    }
    else if(realLocationCode == "TR1")
    {
        bikeLocation = 14;
    }
    if(bikeLocation != -1)
    {
        gGameProgress.SetGameVariable("BikeLocation", bikeLocation);
    }

    // Change to the desired location.
    ExitToLocation(realLocationCode);
}

void DrivingScreen::RefreshUIScaling()
{
    // The original game actually does scale this UI up to match the current resolution.
    // The logic is similar to the title screen, though this screen's a lot simpler because it has no buttons to position.
    if(Prefs::UseOriginalUIScalingLogic())
    {
        // Turn off canvas autoscaling. This sets canvas scale to 1, and width/height equal to window width/height.
        mCanvas->SetAutoScale(false);

        // Due to how the map image scales with resolution, this seems to avoid some "off by one" pixel errors at some scales.
        mMapImage->GetRectTransform()->SetPixelPerfect(false);

        // Resize background image to fit within window size, preserving aspect ratio.
        mMapImage->ResizeToFitPreserveAspect(Window::GetSize());

        //HACK: Applying a slight "fudge" to the y-size fixes some issues with button positioning at high resolutions (where the image is way scaled up).
        mMapImage->GetRectTransform()->SetSizeDeltaY(mMapImage->GetRectTransform()->GetSizeDelta().y + 0.5f);
    }
    else // not using original game's logic.
    {
        // In this case, just use 640x480 and have it auto-scale when the resolution gets too big.
        mCanvas->SetAutoScale(true);
        mMapImage->GetRectTransform()->SetPixelPerfect(true);
        mMapImage->ResizeToTexture();
    }
}