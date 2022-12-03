#include "DrivingScreen.h"

#include "Animator.h"
#include "DrivingScreenBlip.h"
#include "GameProgress.h"
#include "GEngine.h"
#include "IniParser.h"
#include "LocationManager.h"
#include "Scene.h"
#include "SoundtrackPlayer.h"
#include "StringTokenizer.h"
#include "TextAsset.h"
#include "Texture.h"
#include "UIButton.h"
#include "UICanvas.h"
#include "UIImage.h"
#include "Window.h"

DrivingScreen::DrivingScreen() : Actor(TransformType::RectTransform)
{
    // Driving screen should draw above scene transitioner so it doesn't appear on this screen.
    AddComponent<UICanvas>(4);

    // Canvas takes up entire screen.
    RectTransform* rectTransform = GetComponent<RectTransform>();
    rectTransform->SetSizeDelta(0.0f, 0.0f);
    rectTransform->SetAnchorMin(Vector2::Zero);
    rectTransform->SetAnchorMax(Vector2::One);
    
    // Add black background that eats input.
    UIImage* background = AddComponent<UIImage>();
    background->SetTexture(&Texture::Black);
    background->SetReceivesInput(true);

    // Add map background image.
    mMapActor = new Actor(TransformType::RectTransform);
    mMapActor->GetTransform()->SetParent(GetTransform());
    UIImage* mapImage = mMapActor->AddComponent<UIImage>();
    mMapTexture = Services::GetAssets()->LoadTexture("DM_BASE.BMP");
    mapImage->SetTexture(mMapTexture, true);
    mMapImage = mapImage;
    
    // Add location buttons.
    AddLocation("VGR", "ARM", Vector2(458.0f, -225.0f));
    AddLocation("BEC", "BEC", Vector2(94.0f, -400.0f));
    AddLocation("PL3", "BMB", Vector2(499.0f, -137.0f));
    AddLocation("PL2", "CSD", Vector2(396.0f, -187.0f));
    AddLocation("PL6", "CSE", Vector2(520.0f, -4.0f));
    AddLocation("PL4", "LER", Vector2(387.0f, -258.0f));
    AddLocation("LHE", "LHE", Vector2(454.0f, -65.0f));
    AddLocation("PL1", "LHM", Vector2(442.0f, -155.0f));
    AddLocation("MCB", "MCB", Vector2(555.0f, -72.0f));
    AddLocation("PLO", "PLO", Vector2(447.0f, -91.0f));
    AddLocation("POU", "POU", Vector2(578.0f, -22.0f));
    AddLocation("RL1", "RL1", Vector2(487.0f, -174.0f));
    AddLocation("MOP", "RLC", Vector2(193.0f, -119.0f));
    AddLocation("TR1", "TR1", Vector2(57.0f, -131.0f));
    AddLocation("TRE", "TRE", Vector2(506.0f, -89.0f)); //TODO: This one doesn't work when you click it!
    AddLocation("PL5", "WOD", Vector2(44.0f, -218.0f));

    // Load paths for blip movements.
    LoadPaths();

    // Precreate max number of blips we'd need at one time.
    for(int i = 0; i < 3; ++i)
    {
        mBlips.push_back(CreateBlip());
    }
}

DrivingScreen::~DrivingScreen()
{
    // We created this, so we must delete it.
    delete mBlipTexture;
}

void DrivingScreen::Show(FollowMode followMode)
{
    // Set which locations are present depending on game state.
    // Some locations are always available, some are only present after a certain timeblock, some require following someone.
    // LHE (Larry's Place) is always available.
    // PLO (Chataeu de Blanchefort Parking Lot) is always available.
    // RL1 (The Bar) is always available.
    // RLC (Rennes-le-Chataeu) is always available.
    // TR1 (Train Station) is always available.

    // ARM (Devil's Armchair) is only available on or after Day 2, 2pm.
    const Timeblock& currentTimeblock = Services::Get<GameProgress>()->GetTimeblock();
    mLocationButtons["ARM"]->SetEnabled(currentTimeblock >= Timeblock(2, 2, Timeblock::PM));

    // CSE (Chateau de Serras) only available after going on the tour on Day 2, Noon.
    mLocationButtons["CSE"]->SetEnabled(currentTimeblock >= Timeblock(2, 12, Timeblock::PM));

    // POU (Poussin's Tomb) is available after the tour on Day 2, 7am.
    mLocationButtons["POU"]->SetEnabled(currentTimeblock >= Timeblock(2, 7, Timeblock::AM));

    // BEC (Bottom of Hexagram) is only available after discovering it via Le Serpent Rouge.
    // MCB (Top of Hexagram) is the same.
    // TRE (Treasure) as well.
    // TODO!
    mLocationButtons["BEC"]->SetEnabled(false);
    mLocationButtons["MCB"]->SetEnabled(false);
    mLocationButtons["TRE"]->SetEnabled(false);

    // BMB (Red Rock) is only available after spying on Buchelli from Blanchefort.
    //TODO!
    mLocationButtons["BMB"]->SetEnabled(false);

    // CSD (Coume Sourde) only available after following Madeline (or on/after Day 1, 4PM).
    // LHM (Lhomme Mort) is in the same boat.
    bool showCSD_LHM = Services::Get<GameProgress>()->GetNounVerbCount("BUTHANE", "FOLLOW") > 0 && followMode != FollowMode::Buthane;
    showCSD_LHM = showCSD_LHM || currentTimeblock >= Timeblock(1, 4, Timeblock::PM);
    mLocationButtons["CSD"]->SetEnabled(showCSD_LHM);
    mLocationButtons["LHM"]->SetEnabled(showCSD_LHM);

    // LER (L'Ermitage) only available after following Wilkes (or on/after Day 1, 4PM).
    bool showLER = Services::Get<GameProgress>()->GetNounVerbCount("WILKES", "FOLLOW") > 0 && followMode != FollowMode::Wilkes;
    showLER = showLER || currentTimeblock >= Timeblock(1, 4, Timeblock::PM);
    mLocationButtons["LER"]->SetEnabled(showLER);

    // WOD (Woods) only available after following Estelle on Day 2.
    bool showWOD = Services::Get<GameProgress>()->GetNounVerbCount("ESTELLE", "FOLLOW") > 0 && followMode != FollowMode::Estelle;
    mLocationButtons["WOD"]->SetEnabled(showWOD);

    // Make sure the map fits snugly in the window area, with aspect ratio preserved.
    // We do this every time the UI shows in case resolution has changed.
    mMapImage->ResizeToFitPreserveAspect(Window::GetSize());
    
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
        soundtrackPlayer->Play(Services::GetAssets()->LoadSoundtrack("MAPGABEGENDAY1.STK"));
    }
    else
    {
        soundtrackPlayer->Play(Services::GetAssets()->LoadSoundtrack("MAPFOLLOWGEN.STK"));
    }

    // Also play motorcycle driving away SFX.
    soundtrackPlayer->Play(Services::GetAssets()->LoadSoundtrack("MAPHARLEYAWAY.STK"));

    // The map shows as a special location called "Map".
    // Unload the current scene and set the location as such.
    Services::Get<LocationManager>()->ChangeLocation("MAP");

    // Actually show the UI.
    SetActive(true);
}

void DrivingScreen::Hide()
{
    SetActive(false);

    // Stop map music, but play motorcycle arrival sound.
    SoundtrackPlayer* soundtrackPlayer = GetComponent<SoundtrackPlayer>();
    if(soundtrackPlayer != nullptr)
    {
        soundtrackPlayer->StopAll();
        soundtrackPlayer->Play(Services::GetAssets()->LoadSoundtrack("MAPHARLEYARRIVE.STK"));
    }
}

void DrivingScreen::SetLocationButtonsInteractive(bool interactive)
{
    // This doesn't hide the buttons; it just makes them interactive or not.
    for(auto& button : mLocationButtons)
    {
        button.second->SetCanInteract(interactive);
    }
}

void DrivingScreen::ExitToLocation(const std::string& locationCode)
{
    // Change to the desired location.
    Services::Get<LocationManager>()->ChangeLocation(locationCode, [this]() {

        // Hide the map screen after scene load is done.
        Hide();
    });
}

void DrivingScreen::AddLocation(const std::string& locationCode, const std::string& buttonId, const Vector2& buttonPos)
{
    // Create button actor & widget.
    Actor* buttonActor = new Actor(TransformType::RectTransform);
    buttonActor->GetTransform()->SetParent(mMapActor->GetTransform());
    UIButton* button = buttonActor->AddComponent<UIButton>();

    // Set textures.
    Texture* upTexture = Services::GetAssets()->LoadTexture("DM_" + buttonId + "_UL.BMP");
    button->SetUpTexture(upTexture);
    button->SetDownTexture(Services::GetAssets()->LoadTexture("DM_" + buttonId + ".BMP"));
    button->SetHoverTexture(Services::GetAssets()->LoadTexture("DM_" + buttonId + ".BMP"));
    
    // The map scales up or down depending on window resolution.
    // In order for the buttons to appear at the correct positions and scales, we need to do some fancy anchoring...
    {
        float mapWidth = mMapTexture->GetWidth();
        float mapHeight = mMapTexture->GetHeight();

        float buttonWidth = upTexture->GetWidth();
        float buttonHeight = upTexture->GetHeight();

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
    button->SetHoverSound(Services::GetAssets()->LoadAudio("MAPSWOOSH.WAV"));

    // On press, go to the map location.
    button->SetPressCallback([this, locationCode](UIButton* button) {
        Services::GetAudio()->PlaySFX(Services::GetAssets()->LoadAudio("MAPBUTTON.WAV"));

        // Check conditions under which we would NOT allow going to this location.
        // Don't allow going to Larry's place during timeblock 106P.
        if(locationCode == "LHE" && Services::Get<GameProgress>()->GetTimeblock() == Timeblock(1, 6, Timeblock::PM))
        {
            Services::Get<ActionManager>()->ExecuteSheepAction("wait StartDialogue(\"21F4F625S1\", 1)");
            return;
        }

        // Whenever you go to a location on the driving map, it's assumed that your bike also moves there.
        // Each location has it's own integer code (of course) that is rather arbitrary.
        int bikeLocation = -1;
        if(locationCode == "PL5")
        {
            bikeLocation = 0;
        }
        else if(locationCode == "PL4")
        {
            bikeLocation = 1;
        }
        else if(locationCode == "VGR")
        {
            bikeLocation = 2;
        }
        else if(locationCode == "PL2")
        {
            bikeLocation = 3;
        }
        else if(locationCode == "PL1")
        {
            bikeLocation = 4;
        }
        else if(locationCode == "PL3")
        {
            bikeLocation = 5;
        }
        else if(locationCode == "BEC")
        {
            bikeLocation = 6;
        }
        else if(locationCode == "MCB")
        {
            bikeLocation = 7;
        }
        else if(locationCode == "POU")
        {
            bikeLocation = 8;
        }
        else if(locationCode == "PL6")
        {
            bikeLocation = 9;
        }
        else if(locationCode == "MOP")
        {
            bikeLocation = 10;
        }
        else if(locationCode == "LHE")
        {
            bikeLocation = 11;
        }
        else if(locationCode == "PLO")
        {
            bikeLocation = 12;
        }
        else if(locationCode == "RL1")
        {
            bikeLocation = 13;
        }
        else if(locationCode == "TR1")
        {
            bikeLocation = 14;
        }
        if(bikeLocation != -1)
        {
            Services::Get<GameProgress>()->SetGameVariable("BikeLocation", bikeLocation);
        }

        // Change to the desired location.
        ExitToLocation(locationCode);
    });

    // Cache button for later.
    mLocationButtons[buttonId] = button;
}

void DrivingScreen::LoadPaths()
{
    // Load path data.
    TextAsset* pathData = Services::GetAssets()->LoadText("PATHDATA.TXT");

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
    Services::GetAssets()->UnloadText(pathData);
}

DrivingScreenBlip* DrivingScreen::CreateBlip()
{
    // Create blip texture. This is a 6x6 white dot, corner pixels alpha'd.
    if(mBlipTexture == nullptr)
    {
        mBlipTexture = new Texture(6, 6, Color32::White);
        mBlipTexture->SetPixelColor32(0, 0, Color32::Clear);
        mBlipTexture->SetPixelColor32(1, 0, Color32::Clear);
        mBlipTexture->SetPixelColor32(0, 1, Color32::Clear);

        mBlipTexture->SetPixelColor32(5, 0, Color32::Clear);
        mBlipTexture->SetPixelColor32(4, 0, Color32::Clear);
        mBlipTexture->SetPixelColor32(5, 1, Color32::Clear);

        mBlipTexture->SetPixelColor32(5, 5, Color32::Clear);
        mBlipTexture->SetPixelColor32(4, 5, Color32::Clear);
        mBlipTexture->SetPixelColor32(5, 4, Color32::Clear);

        mBlipTexture->SetPixelColor32(0, 5, Color32::Clear);
        mBlipTexture->SetPixelColor32(1, 5, Color32::Clear);
        mBlipTexture->SetPixelColor32(0, 4, Color32::Clear);
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
    mBlips[kEgoIndex]->SetMapPosition(Services::Get<LocationManager>()->GetLocation());

    // The other blips are only present in some circumstances. Deactivate them by default.
    mBlips[kNpc1Index]->SetActive(false);
    mBlips[kNpc2Index]->SetActive(false);
    mBlips[kNpc1Index]->ClearPath();
    mBlips[kNpc2Index]->ClearPath();

    // "None" follow mode - we aren't actively following anyone.
    // The only blips that appear in this case are the idle loops of some characters depending on the timeblock.
    if(followMode == FollowMode::None)
    {
        if(Services::Get<GameProgress>()->GetTimeblock() == Timeblock(1, 2, Timeblock::PM))
        {
            // Buthane drives in circles, starting at PL3.
            // But she no longer appears on the map after LHM is enabled.
            if(!mLocationButtons["LHM"]->IsEnabled())
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
            if(!mLocationButtons["LER"]->IsEnabled())
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
        else if(Services::Get<GameProgress>()->GetTimeblock() == Timeblock(1, 4, Timeblock::PM))
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
        else if(Services::Get<GameProgress>()->GetTimeblock() == Timeblock(1, 6, Timeblock::PM))
        {
            // Mallory & MacDougall drive in circles, COUNTER-CLOCKWISE, starting at LHE.
            // They no longer appear after they've been followed.
            if(Services::Get<GameProgress>()->GetNounVerbCount("TWO_MEN", "FOLLOW") == 0)
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
        else if(Services::Get<GameProgress>()->GetTimeblock() == Timeblock(2, 2, Timeblock::PM))
        {
            // Lady Howard & Estelle drive in circles, starting at PL3. 
            if(Services::Get<LocationManager>()->IsActorAtLocation("ESTELLE", "MAP"))
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
            mBlips[kNpc1Index]->SetPathCompleteCallback([this]() {
                OnFollowDone();
                mLocationButtons["CSD"]->SetEnabled(true);
                mLocationButtons["LHM"]->SetEnabled(true);
                Services::Get<ActionManager>()->ExecuteSheepAction("wait StartDialogue(\"2196L3WL71\", 1)");
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
            mBlips[kNpc1Index]->SetPathCompleteCallback([this]() {
                OnFollowDone();
                mLocationButtons["LER"]->SetEnabled(true);
                Services::Get<ActionManager>()->ExecuteSheepAction("wait StartDialogue(\"2196L3WLM1\", 1)");
            });
        }
        else if(followMode == FollowMode::LadyHoward)
        {
            mBlips[kNpc1Index]->SetColor(kEstelleColor);

            // So, there are two spots we can follow Lady Howard from, and in both cases, we want to do a complete circuit :P
            // Let's write this out...
            std::string currLocation = Services::Get<LocationManager>()->GetLocation();
            if(StringUtil::EqualsIgnoreCase(currLocation, "PLO"))
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
            mBlips[kNpc1Index]->SetPathCompleteCallback([this, currLocation]() {
                OnFollowDone();

                // Play dialogue ("they're just driving around the valley - forget that").
                Services::Get<ActionManager>()->ExecuteSheepAction("wait StartDialogue(\"21A6L3WLX1\", 1)", [this, currLocation](const Action* action){
                    // This one automatically puts you back where you started.
                    ExitToLocation(currLocation);
                });
            });
        }
        else if(followMode == FollowMode::PrinceJamesMen)
        {
            mBlips[kNpc1Index]->SetColor(kJamesMenColor);

            // This one's got a couple possible follow locations:
            // If from PLO or LER, we use one path.
            // If from RLC, we use another path. We can assume this is the "else" case.
            std::string currLocation = Services::Get<LocationManager>()->GetLocation();
            if(StringUtil::EqualsIgnoreCase(currLocation, "PLO") || StringUtil::EqualsIgnoreCase(currLocation, "LER"))
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
            mBlips[kNpc1Index]->SetPathCompleteCallback([this]() {
                OnFollowDone();
                Services::Get<ActionManager>()->ExecuteSheepAction("wait StartDialogue(\"21F6L3WBH1\", 1)");
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
            mBlips[kNpc1Index]->SetPathCompleteCallback([this]() {
                OnFollowDone();
                mLocationButtons["WOD"]->SetEnabled(true);
                Services::Get<ActionManager>()->ExecuteSheepAction("wait StartDialogue(\"21K6L3WJI1\", 1)");
            });
        }

        // On the path defined for the NPC, always skip to wherever we are along that route, so it looks like we followed from the correct spot.
        mBlips[kNpc1Index]->SkipToPathNode(Services::Get<LocationManager>()->GetLocation());

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
