//
// Clark Kromenaker
//
// In addition to the "Files" UI for Sidney, this class also acts as the "file system" for Sidney.
//
#pragma once
#include <functional>
#include <string>
#include <vector>

#include "Actor.h"
#include "InventoryManager.h"
#include "SidneyUtil.h"
#include "StringUtil.h"

class Sidney;
class UIButton;
class UILabel;

enum class SidneyFileType
{
    Image,
    Audio,
    Text,
    Fingerprint,
    License,
    Shape
};

// A single file in the system.
struct SidneyFile
{
    // Type of this file (is it an image, audio, text, etc?).
    SidneyFileType type;

    // This file's unique index. This is what's used to get the localized name.
    int index = -1;

    // Unique *internal* name for this file. Not displayed to player!
    // This is used by Sheep to identify files it wants to add.
    std::string name;

    // Name of inventory item that corresponds to this file.
    // Since all files are scanned in, they always have an inventory item scanned from.
    std::string invItemName;

    // A score name associated with adding this file.
    std::string scoreName;

    // Has the file ever been analyzed before? Affects the behavior of opening a file in the Analyze view.
    bool hasBeenAnalyzed = false;
    
    SidneyFile() = default;
    SidneyFile(const SidneyFile&) = default;
    SidneyFile(SidneyFile&&) = default;

    SidneyFile(SidneyFileType type, const std::string& name) : type(type), name(name) { }
    SidneyFile(SidneyFileType type, const std::string& name, const std::string& invItemName) : type(type), name(name), invItemName(invItemName) { }
    SidneyFile(SidneyFileType type, const std::string& name, const std::string& invItemName, const std::string& scoreName) : type(type), name(name), invItemName(invItemName), scoreName(scoreName) { }

    std::string GetDisplayName()
    {
        return SidneyUtil::GetAddDataLocalizer().GetText("ScanItem" + std::to_string(index + 1));
    }

    Texture* GetIcon()
    {
        // We just reuse the inventory list texture for this.
        return gInventoryManager.GetInventoryItemListTexture(invItemName);
    }
};

// A directory containing files.
// In Sidney, each directory is a category - Images, Audio, Fingerprints, etc.
struct SidneyDirectory
{
    // Display name of directory. This is a key to be fed through the localizer.
    std::string name;

    // Type of file that goes in this directory.
    SidneyFileType type;

    // All the files contained in this directory.
    std::vector<SidneyFile*> files;

    bool HasFile(const std::string& fileName) const
    {
        for(auto& file : files)
        {
            if(StringUtil::EqualsIgnoreCase(file->name, fileName)) { return true; }
        }
        return false;
    }
};

class SidneyFiles
{
public:
    void Init(Sidney* sidney);

    void Show(std::function<void(SidneyFile*)> selectFileCallback = nullptr);
    void ShowShapes(std::function<void(SidneyFile*)> selectFileCallback = nullptr);
    void ShowCustom(const std::string& title, const std::vector<std::string>& choices, std::function<void(size_t)> selectCallback);

    void AddFile(size_t fileIndex);
    bool HasFile(size_t fileIndex);
    int GetMaxFileIndex() const { return mAllFiles.size() - 1; }

    bool HasFile(const std::string& fileName) const;
    bool HasFileOfType(SidneyFileType type) const;

private:
    // The data in the file system. This is stuff that has been scanned into Sidney by the player.
    std::vector<SidneyDirectory> mData;

    // A list of ALL files the game knows about that *can* be scanned into Sidney.
    // The index is important, as that's how the "Add Data" & localization mechanisms identify a file.
    std::vector<SidneyFile> mAllFiles;

    // Files UI root.
    Actor* mRoot = nullptr;
    
    // A button used for each entry in the file list window.
    struct FileListButton
    {
        UIButton* button = nullptr;
        UILabel* label = nullptr;
    };

    // The game supports two different file list windows.
    // One for most files, and then a separate one for shapes.
    class FileListWindow
    {
    public:
        void Init(Actor* parent, bool forShapes);

        void Show(const std::vector<SidneyDirectory>& data, std::function<void(SidneyFile*)> selectCallback);
        void Show(const std::string& title, const std::vector<std::string>& choices, std::function<void(size_t)> selectCallback);

    private:
        // The shapes list works a bit differently.
        bool mForShapes = false;

        // The root of the window - use to show/hide it.
        Actor* mWindowRoot = nullptr;

        // The title label.
        UILabel* mTitleLabel = nullptr;

        // Each button is a selection in the window.
        std::vector<FileListButton> mButtons;
        size_t mButtonIndex = 0;

        FileListButton& GetFileListButton();
    };
    FileListWindow mFileList;
    FileListWindow mShapeList;

    FileListWindow mCustomList;
};