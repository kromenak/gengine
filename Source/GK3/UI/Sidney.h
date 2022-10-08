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
    bool HasFile(const std::string& type, const std::string& fileName);
    void AddFile(const std::string& type, const std::string& fileName);

protected:
    void OnUpdate(float deltaTime) override;

private:
    // New email label.
    UILabel* mNewEmailLabel = nullptr;

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

    // The "hard drive" directory & file structure for Sidney.
    struct SidneyFile
    {
        std::string type;
        std::string name;
        SidneyFile(const std::string& type, const std::string& name) : type(type), name(name) { }
    };
    struct SidneyDirectory
    {
        std::string name;
        std::vector<SidneyFile> files;

        bool HasFile(const std::string& fileName)
        {
            for(auto& file : files)
            {
                if(StringUtil::EqualsIgnoreCase(file.name, fileName)) { return true; }
            }
            return false;
        }
    };
    std::vector<SidneyDirectory> mData;

    // A list of files that *can* be scanned into Sidney.
    // The index is important, as that's how the "Add Data" & localization mechanisms identify a file.
    std::vector<SidneyFile> mKnownFiles;
};