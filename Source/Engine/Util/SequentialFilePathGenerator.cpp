#include "SequentialFilePathGenerator.h"

#include "FileSystem.h"
#include "StringUtil.h"

SequentialFilePathGenerator::SequentialFilePathGenerator(const std::string& directoryPath, const std::string& fileNameTemplate) :
    mDirectoryPath(directoryPath),
    mFileNameTemplate(fileNameTemplate)
{
    // This system only works if the file name template contains a number format string.
    // Detect that, and if it not, force one in!
    EnsureFileNameTemplateIsValid();

    // Make sure the directory exists.
    if(!Directory::Exists(mDirectoryPath))
    {
        Directory::CreateAll(mDirectoryPath);
    }
}

std::string SequentialFilePathGenerator::GenerateFilePath(bool fullRescan)
{
    // If the directory doesn't exist, then we can't really do our job here...
    // But let's return what seems like it'd be a valid path if it did exist.
    if(!Directory::Exists(mDirectoryPath))
    {
        return GeneratePath(0);
    }

    // If a full rescan is desired, start back at the first file number.
    if(fullRescan)
    {
        mCurrentFileNumber = mFirstFileNumber;
    }

    // Keep generating file paths and checking if they exist until we find one that DOESN'T exist. That's the one to use!
    std::string filePath = GeneratePath(mCurrentFileNumber);
    while(File::Exists(filePath))
    {
        ++mCurrentFileNumber;
        filePath = GeneratePath(mCurrentFileNumber);
    }
    return filePath;
}

std::string SequentialFilePathGenerator::GeneratePath(int fileNumber)
{
    return Path::Combine({ mDirectoryPath, StringUtil::Format(mFileNameTemplate.c_str(), fileNumber) });
}

void SequentialFilePathGenerator::EnsureFileNameTemplateIsValid()
{
    // Find a string of characters that represents a numeric formatter.
    // Valid ones could be: %d, %i, %03d.
    bool foundStartChar = false;
    bool foundEndChar = false;
    for(char c : mFileNameTemplate)
    {
        if(!foundStartChar)
        {
            if(c == '%')
            {
                foundStartChar = true;
            }
        }
        else
        {
            if(c == 'd' || c == 'i')
            {
                foundEndChar = true;
                break;
            }
            else if(!isdigit(c)) // encountered a space or some other char, so this won't work.
            {
                foundStartChar = false;
                foundEndChar = false;
            }
        }
    }

    // If we didn't find a valid numeric formatter anywhere, force one in there.
    // Something like "blah.png" becomes "blah_%d.png".
    if(!foundEndChar)
    {
        mFileNameTemplate = Path::GetFileNameNoExtension(mFileNameTemplate) + "_%d" + Path::GetExtension(mFileNameTemplate, true);
    }
}