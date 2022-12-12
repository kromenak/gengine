//
// Clark Kromenaker
//
// UI for the "Add Data" dialog box and scanning cutscene.
//
#pragma once
#include <unordered_map>

#include "Vector2.h"

class Actor;
class Font;
class Sidney;
class SidneyFiles;
class UIButton;
class UILabel;

class SidneyAddData
{
public:
    void Init(Sidney* sidney, SidneyFiles* sidneyFiles);

    void Start();

    void OnUpdate(float deltaTime);

    bool AddingData() const { return mAddingData; }

private:
    // Other Sidney modules this module needs to interface with.
    Sidney* mSidney = nullptr;
    SidneyFiles* mSidneyFiles = nullptr;

    // If true, we're actively adding data.
    bool mAddingData = false;

    // "Add Data" dialog box.
    Actor* mAddDataBox = nullptr;
    UILabel* mAddDataLabel = nullptr;

    // In one case, the "add data label" is supposed to blink Yellow/Green.
    const float kAddDataColorToggleInterval = 0.5f;
    float mAddDataColorTimer = -1.0f;
    Font* mYellowFont = nullptr;
    Font* mGreenFont = nullptr;

    // "Input complete" dialog box.
    Actor* mInputCompleteBox = nullptr;
    UILabel* mFileNameLabel = nullptr;
    UIButton* mInputCompleteOKButton = nullptr;

    // The in-game character "types" the name of the file before adding it.
    // We use these variables to track the state of the text being typed.
    std::string mTextToType;
    int mTextToTypeIndex = -1;

    const Vector2 kMinMaxTypeInterval = Vector2(0.2f, 0.5f);
    float mTextToTypeTimer = 0.0f;
};