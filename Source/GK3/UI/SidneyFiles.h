//
// Clark Kromenaker
//
// In addition to the "Files" UI for Sidney, this class also acts as the "file system" for Sidney.
//
#pragma once
#include <string>
#include <vector>

#include "StringUtil.h"

class Sidney;

// A single file in the system.
struct SidneyFile
{
    std::string type;
    std::string name;
    int index = -1;
    SidneyFile() = default;
    SidneyFile(const std::string& type, const std::string& name) : type(type), name(name) { }
    SidneyFile(const SidneyFile&) = default;
    SidneyFile(SidneyFile&&) = default;
};

// A directory containing files.
// In Sidney, each directory is a category - Images, Audio, Fingerprints, etc.
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

class SidneyFiles
{
public:
    void Init(Sidney* parent);

    void AddFile(int fileIndex);

    bool HasFile(const std::string& fileName) const;

    int GetMaxFileIndex() const { return mAllFiles.size() - 1; }

private:
    // The data in the file system. This is stuff that has been scanned into Sidney by the player.
    std::vector<SidneyDirectory> mData;

    // A list of files that *can* be scanned into Sidney.
    // The index is important, as that's how the "Add Data" & localization mechanisms identify a file.
    std::vector<SidneyFile> mAllFiles;

    SidneyDirectory* FindDirectory(const std::string& name);
};