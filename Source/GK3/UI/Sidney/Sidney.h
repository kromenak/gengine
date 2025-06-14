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
