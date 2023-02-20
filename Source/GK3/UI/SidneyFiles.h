//
// Clark Kromenaker
//
// In addition to the "Files" UI for Sidney, this class also acts as the "file system" for Sidney.
//
#pragma once
#include <string>
#include <vector>

#include "Actor.h"
#include "StringUtil.h"

class Sidney;
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
    // Unique *internal* name for this file. Not displayed to player!
    // This is used by Sheep to identify files it wants to add.
    std::string name;

    // This file's unique index. This is what's used to get the localized name.
    int index = -1;

    // Type of this file (is it an image, audio, text, etc?).
    SidneyFileType type;

    // A score name associated with adding this file.
    std::string scoreName;
    
    SidneyFile() = default;
    SidneyFile(SidneyFileType type, const std::string& name) : type(type), name(name) { }
    SidneyFile(SidneyFileType type, const std::string& name, const std::string& scoreName) : type(type), name(name), scoreName(scoreName) { }
    SidneyFile(const SidneyFile&) = default;
    SidneyFile(SidneyFile&&) = default;
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
    std::vector<SidneyFile> files;

    bool HasFile(const std::string& fileName) const
    {
        for(auto& file : files)
        {
            if(StringUtil::EqualsIgnoreCase(file.name, fileName)) { return true; }
        }
        return false;
    }
};

class SidneyFiles
{
public:
    void Init(Sidney* sidney);

    void Show();
    void Hide();

    void AddFile(int fileIndex);
    bool HasFile(int fileIndex);
    int GetMaxFileIndex() const { return mAllFiles.size() - 1; }

    bool HasFile(const std::string& fileName) const;

private:
    // The data in the file system. This is stuff that has been scanned into Sidney by the player.
    std::vector<SidneyDirectory> mData;

    // A list of ALL files the game knows about that *can* be scanned into Sidney.
    // The index is important, as that's how the "Add Data" & localization mechanisms identify a file.
    std::vector<SidneyFile> mAllFiles;

    // Files dialog box UI root.
    Actor* mRoot = nullptr;
    Actor* mDialogRoot = nullptr;

    // Labels used to populate the file list. Can be reused/repurposed as the list changes.
    std::vector<UILabel*> mFileListLabels;
    int mFileListLabelIndex = 0;
    
    UILabel* GetFileListLabel();
    void RefreshFileListUI();
};