//
// Clark Kromenaker
//
// UI for the Sidney computer system in GK3.
//
#pragma once
#include "Actor.h"

#include <string>
#include <vector>

#include "SidneyEmail.h"
#include "SidneySearch.h"

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

    // Controls email label blink behavior.
    const float kNewEmailBlinkInterval = 0.5f;
    float mNewEmailBlinkTimer = -1.0f;

    // Various subscreens.
    SidneySearch mSearch;
    SidneyEmail mEmail;

    // The "hard drive" directory & file structure for Sidney.
    struct SidneyFile
    {
        std::string name;
    };
    struct SidneyDirectory
    {
        std::string name;
        std::vector<SidneyFile> files;
    };
    std::vector<SidneyDirectory> mData;
};