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
    
    SidneyFile() = default;
    SidneyFile(const SidneyFile&) = default;
    SidneyFile(SidneyFile&&) = default;

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
    void Hide();

    void AddFile(size_t fileIndex);
    bool HasFile(size_t fileIndex);
    int GetMaxFileIndex() const { return mAllFiles.size() - 1; }

    bool HasFile(const std::string& fileName) const;

private:
    // The data in the file system. This is stuff that has been scanned into Sidney by the player.
    std::vector<SidneyDirectory> mData;

    // A list of ALL files the game knows about that *can* be scanned into Sidney.
    // The index is important, as that's how the "Add Data" & localization mechanisms identify a file.
    std::vector<SidneyFile> mAllFiles;

    // A callback for other screens to know when a file is selected by the user.
    std::function<void(SidneyFile*)> mSelectFileCallback;

    // Files dialog box UI root.
    Actor* mRoot = nullptr;
    Actor* mDialogRoot = nullptr;

    // Labels used to populate the file list. Can be reused/repurposed as the list changes.
    struct FileListButton
    {
        UIButton* button = nullptr;
        UILabel* label = nullptr;
    };
    std::vector<FileListButton> mFileListButtons;
    size_t mFileListLabelIndex = 0;
    
    FileListButton GetFileListButton();
    void RefreshFileListUI();
};