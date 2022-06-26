#include "DrivingScreen.h"

#include "GameProgress.h"
#include "GEngine.h"
#include "LocationManager.h"
#include "SoundtrackPlayer.h"
#include "Texture.h"
#include "UIButton.h"
#include "UICanvas.h"
#include "UIImage.h"

DrivingScreen::DrivingScreen() : Actor(Actor::TransformType::RectTransform)
{
    mCanvas = AddComponent<UICanvas>(0);

    // Canvas takes up entire screen.
    RectTransform* rectTransform = GetComponent<RectTransform>();
    rectTransform->SetSizeDelta(0.0f, 0.0f);
    rectTransform->SetAnchorMin(Vector2::Zero);
    rectTransform->SetAnchorMax(Vector2::One);
    
    // Add black background that eats input.
    UIImage* background = AddComponent<UIImage>();
    mCanvas->AddWidget(background);
    background->SetTexture(&Texture::Black);
    background->SetReceivesInput(true);

    // Add map background image.
    mMapActor = new Actor(Actor::TransformType::RectTransform);
    mMapActor->GetTransform()->SetParent(GetTransform());
    UIImage* mapImage = mMapActor->AddComponent<UIImage>();
    mCanvas->AddWidget(mapImage);
    mapImage->SetTexture(Services::GetAssets()->LoadTexture("DM_BASE.BMP"), true);
    
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
}

void DrivingScreen::Show()
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

    // CSD (Coume Sourde) only available after following Madeline.
    // LHM (Lhomme Mort) is in the same boat.
    //TODO!
    mLocationButtons["CSD"]->SetEnabled(false);
    mLocationButtons["LHM"]->SetEnabled(false);

    // LER (L'Ermitage) only available after following Wilkes.
    //TODO!
    mLocationButtons["LER"]->SetEnabled(false);

    // WOD (Woods) only available after following Lady Howard & Estelle.
    mLocationButtons["WOD"]->SetEnabled(false);

    // Actually show the UI.
    SetActive(true);

    // Add soundtrack player, if not yet present.
    SoundtrackPlayer* soundtrackPlayer = GetComponent<SoundtrackPlayer>();
    if(soundtrackPlayer == nullptr)
    {
        soundtrackPlayer = AddComponent<SoundtrackPlayer>();
    }

    // Play general map music.
    soundtrackPlayer->Play(Services::GetAssets()->LoadSoundtrack("MAPGABEGENDAY1.STK"));

    // Also play motorcycle driving away SFX.
    soundtrackPlayer->Play(Services::GetAssets()->LoadSoundtrack("MAPHARLEYAWAY.STK"));

    // The map shows as a special location called "Map".
    // Unload the current scene and set the location as such.
    Services::Get<LocationManager>()->ChangeLocation("MAP");
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

void DrivingScreen::AddLocation(const std::string& locationCode, const std::string& buttonId, const Vector2& buttonPos)
{
    Actor* buttonActor = new Actor(Actor::TransformType::RectTransform);
    buttonActor->GetTransform()->SetParent(mMapActor->GetTransform());
    UIButton* button = buttonActor->AddComponent<UIButton>();
    mCanvas->AddWidget(button);

    // Set textures.
    button->SetUpTexture(Services::GetAssets()->LoadTexture("DM_" + buttonId + "_UL.BMP"));
    button->SetDownTexture(Services::GetAssets()->LoadTexture("DM_" + buttonId + ".BMP"));
    button->SetHoverTexture(Services::GetAssets()->LoadTexture("DM_" + buttonId + ".BMP"));

    // Anchor to top-left, and position from center-point.
    button->GetRectTransform()->SetAnchor(0.0f, 1.0f);
    button->GetRectTransform()->SetPivot(0.0f, 1.0f);
    button->GetRectTransform()->SetAnchoredPosition(buttonPos);

    button->SetHoverSound(Services::GetAssets()->LoadAudio("MAPSWOOSH.WAV"));

    button->SetPressCallback([this, locationCode](UIButton* button) {
        Services::GetAudio()->PlaySFX(Services::GetAssets()->LoadAudio("MAPBUTTON.WAV"));

        Services::Get<LocationManager>()->ChangeLocation(locationCode);

        //TODO: Should hide AFTER scene and location change is done.
        //TODO: That just requires adding a callback mechanism to ChangeLocation & LoadScene.
        Hide();
    });

    // Cache button for later.
    mLocationButtons[buttonId] = button;
}