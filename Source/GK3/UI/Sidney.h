//
// Clark Kromenaker
//
// UI for the Sidney computer system in GK3.
//
#pragma once
#include "Actor.h"

#include <string>
#include <vector>

#include "SidneyAddData.h"
#include "SidneyEmail.h"
#include "SidneyFiles.h"
#include "SidneySearch.h"
#include "StringUtil.h"

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

protected:
    void OnUpdate(float deltaTime) override;

private:
    // New email label.
    UILabel* mNewEmailLabel = nullptr;

    // If true, play "New Email" SFX on next update.
    // Doing this avoids issue where this SFX won't play when you use Action Skip.
    bool mPlayNewEmailSfx = false;

    // Controls email label blink behavior.
    const float kNewEmailBlinkInterval = 0.5f;
    float mNewEmailBlinkTimer = -1.0f;

    // "Add Data" dialog box.
    Actor* mAddDataBox = nullptr;
    UILabel* mAddDataLabel = nullptr;
    bool mAddingData = false;

    // Various subscreens.
    SidneySearch mSearch;
    SidneyEmail mEmail;
    SidneyFiles mFiles;
    SidneyAddData mAddData;
};
