//
// Clark Kromenaker
//
// UI for the Sidney computer system in GK3.
//
#pragma once
#include "Actor.h"

#include <string>

#include "SidneyAddData.h"
#include "SidneyAnalyze.h"
#include "SidneyEmail.h"
#include "SidneyFiles.h"
#include "SidneyMakeId.h"
#include "SidneySearch.h"
#include "SidneySuspects.h"
#include "SidneyTranslate.h"

class PersistState;
class UIButton;
class UIImage;
class UILabel;

class Sidney : public Actor
{
public:
    Sidney();

    void Show();
    void Hide();

    bool HasFile(const std::string& fileName);

    void OnPersist(PersistState& ps);

protected:
    void OnUpdate(float deltaTime) override;

private:
    // "Add Data" dialog box.
    Actor* mAddDataBox = nullptr;
    UILabel* mAddDataLabel = nullptr;
    bool mAddingData = false;

    // When playing at higher resolutions, Sidney has some decorative textures around it, to make it feel like you're in R25.
    // One of those textures (a photo of the Dalai Lama) overlaps the screen.
    // This is cool, but we also don't want to show that texture at lower resolutions, as it would look strange in that case.
    UIImage* mLamaImage = nullptr;

    // Various subscreens.
    SidneySearch mSearch;
    SidneyEmail mEmail;
    SidneyFiles mFiles;
    SidneyAnalyze mAnalyze;
    SidneyTranslate mTranslate;
    SidneyAddData mAddData;
    SidneyMakeId mMakeId;
    SidneySuspects mSuspects;
};
