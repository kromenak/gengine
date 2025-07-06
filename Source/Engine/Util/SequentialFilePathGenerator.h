//
// Clark Kromenaker
//
// A helper for generating file names that contain a sequential numeric identifier.
// Sometimes, you want to save files to the disk following a sequential numeric pattern, such as:
//     screenshot_000.png
//     screenshot_001.png
//     screenshot_002.png
//
// Each time you save a file, you need to figure out what the next number should be.
// Additionally, if one of the files is deleted, you might want the next file to fill the empty spot!
// So, this helper class tries to do that in a centralized/reusable way.
//
#pragma once
#include <string>

class SequentialFilePathGenerator
{
public:
    SequentialFilePathGenerator(const std::string& directoryPath, const std::string& fileNameTemplate);
    std::string GenerateFilePath(bool fullRescan = false);

private:
    // The path to the directory we want to create files in.
    std::string mDirectoryPath;

    // A template for the expected filename format. Should include a "%d" somewhere.
    // Some valid examples: "screenshot_03%d.png", "save04%d.gk3", etc.
    std::string mFileNameTemplate;

    // The first number in the sequence. This will usually be 0 or 1.
    int mFirstFileNumber = 0;

    // The current number in the sequence.
    // This'll increment if we attempt to generate a file path, but the file with this number already exists.
    int mCurrentFileNumber = 0;

    std::string GeneratePath(int fileNumber);
    void EnsureFileNameTemplateIsValid();
};