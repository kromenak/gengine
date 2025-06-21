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
#include "AssetManager.h"
#include "InventoryManager.h"
#include "SidneyUtil.h"
#include "Texture.h"

class PersistState;
class Sidney;
class UIButton;
class UILabel;
class UIScrollRect;

namespace SidneyFileIds
{
    const int kMoselyFingerprint = 7;
    const int kManuscriptPrint1 = 11;
    const int kManuscriptPrint2 = 12;
    const int kManuscriptPrint3 = 13;
    const int kUnknownLSRFingerprint = 18;

    const int kMap = 19;
    const int kParchment1 = 20;
    const int kParchment2 = 21;
    const int kPoussinPostcard = 22;
    const int kTeniersPostcard1 = 23;
    const int kTeniersPostcard2 = 24;

    const int kSerresHermeticSymbols = 25;
    const int kSumNote = 26;
    const int kAbbeTape = 27;
    const int kBuchelliTape = 28;
    const int kArcadiaText = 29;

    const int kTriangleShape = 37;
    const int kCircleShape = 38;
    const int kSquareShape = 39;
    const int kHexagramShape = 40;
}

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
    // This file's unique identifier.
    // The ID is important, as that's how the "Add Data" & localization mechanisms identify a file.
    int id = -1;

    // Type of this file (is it an image, audio, text, etc?).
    SidneyFileType type = SidneyFileType::Image;

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

    SidneyFile(int id, SidneyFileType type, const std::string& name) : id(id), type(type), name(name) { }
    SidneyFile(int id, SidneyFileType type, const std::string& name, const std::string& invItemName) : id(id), type(type), name(name), invItemName(invItemName) { }
    SidneyFile(int id, SidneyFileType type, const std::string& name, const std::string& invItemName, const std::string& scoreName) : id(id), type(type), name(name), invItemName(invItemName), scoreName(scoreName) { }

    std::string GetDisplayName()
    {
        // Localized display names are stored as ScanItemX, where X is ID+1.
        return SidneyUtil::GetAddDataLocalizer().GetText("ScanItem" + std::to_string(id + 1));
    }

    Texture* GetIcon()
    {
        // We just reuse the inventory list texture for this.
        Texture* texture = gInventoryManager.GetInventoryItemListTexture(invItemName);
        if(texture == nullptr || StringUtil::StartsWithIgnoreCase(texture->GetName(), "Undefined"))
        {
            // Some files don't have an inventory item - just fall back on a generic file icon.
            texture = gAssetManager.LoadTexture("SIDFILE_9.BMP");
        }
        return texture;
    }

    void OnPersist(PersistState& ps)
    {
        // Even though only "hasBeenAnalyzed" is dynamic game state data, we need to save the whole struct.
        // This is because Xfer will clear and rebuild the ENTIRE list of files.
        ps.Xfer(PERSIST_VAR(id));
        ps.Xfer<SidneyFileType, int>(PERSIST_VAR(type));
        ps.Xfer(PERSIST_VAR(name));
        ps.Xfer(PERSIST_VAR(invItemName));
        ps.Xfer(PERSIST_VAR(scoreName));
        ps.Xfer(PERSIST_VAR(hasBeenAnalyzed));
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

    // IDs of all files contained in this directory.
    // These IDs correspond to files stored in mAllFiles.
    std::vector<int> fileIds;

    bool HasFile(int fileId) const
    {
        return std::find(fileIds.begin(), fileIds.end(), fileId) != fileIds.end();
    }

    void OnPersist(PersistState& ps)
    {
        ps.Xfer(PERSIST_VAR(name));
        ps.Xfer<SidneyFileType, int>(PERSIST_VAR(type));
        ps.Xfer(PERSIST_VAR(fileIds));
    }
};

class SidneyFiles
{
public:
    void Init(Sidney* sidney);

    void Show(std::function<void(SidneyFile*)> selectFileCallback = nullptr);
    void ShowShapes(std::function<void(SidneyFile*)> selectFileCallback = nullptr);
    void ShowCustom(const std::string& title, const std::vector<std::string>& choices, std::function<void(size_t)> selectCallback);
    void HideAllFileWindows();
    Actor* GetShowingFileWindow();

    void AddFile(size_t fileId);
    bool HasFile(size_t fileId) const;
    SidneyFile* GetFile(size_t fileId);
    int GetMaxFileIndex() const { return mAllFiles.size() - 1; }

    bool HasFile(const std::string& fileName) const;
    bool HasFileOfType(SidneyFileType type) const;

    void OnPersist(PersistState& ps);

private:
    // The data in the file system. This is stuff that has been scanned into Sidney by the player.
    std::vector<SidneyDirectory> mData;

    // A list of ALL files the game knows about that *can* be scanned into Sidney.
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

        void Show(std::vector<SidneyFile>& files, const std::vector<SidneyDirectory>& data, std::function<void(SidneyFile*)> selectCallback);
        void Show(const std::string& title, const std::vector<std::string>& choices, std::function<void(size_t)> selectCallback);

        bool IsShowing() const { return mWindowRoot != nullptr && mWindowRoot->IsActive(); }
        Actor* GetWindowRoot() const { return mWindowRoot; }

    private:
        // The shapes list works a bit differently.
        bool mForShapes = false;

        // The root of the window - use to show/hide it.
        Actor* mWindowRoot = nullptr;

        // The title label.
        UILabel* mTitleLabel = nullptr;

        // A scroll rect containing the file list (in case it overflows the available space).
        UIScrollRect* mScrollRect = nullptr;

        // Each button is a selection in the window.
        std::vector<FileListButton> mButtons;
        size_t mButtonIndex = 0;

        FileListButton& GetFileListButton();
    };
    FileListWindow mFileList;
    FileListWindow mShapeList;

    // A custom list window used to select from a choice of arbitrary options.
    // This is used during analysis to select grid size and fill options, for example.
    FileListWindow mCustomList;
};