#include "SidneyFiles.h"

void SidneyFiles::Init(Sidney* parent)
{
    // Build directory structure.
    mData.emplace_back();
    mData.back().name = "Images";
    mData.emplace_back();
    mData.back().name = "Audio";
    mData.emplace_back();
    mData.back().name = "Text";
    mData.emplace_back();
    mData.back().name = "Fingerprints";
    mData.emplace_back();
    mData.back().name = "Licenses";
    mData.emplace_back(); // TODO: this one doesn't appear in the list until you find your first shape
    mData.back().name = "Shapes";

    // Build list of known files.
    //TODO: Would be cool if this was data-driven?
    mAllFiles.emplace_back("Fingerprints", "fileAbbePrint"); // 1
    mAllFiles.emplace_back("Fingerprints", "fileBuchelliPrint");
    mAllFiles.emplace_back("Fingerprints", "fileButhanePrint");
    mAllFiles.emplace_back("Fingerprints", "fileEstellePrint");
    mAllFiles.emplace_back("Fingerprints", "fileLadyHowardPrint");
    mAllFiles.emplace_back("Fingerprints", "fileMontreauxPrint");
    mAllFiles.emplace_back("Fingerprints", "fileWilkesPrint");
    mAllFiles.emplace_back("Fingerprints", "fileMoselyPrint");
    mAllFiles.emplace_back("Fingerprints", "fileLarryPrint");
    mAllFiles.emplace_back("Fingerprints", "fileWilkesPrint2"); // 10
    mAllFiles.emplace_back("Fingerprints", "fileBuchelliPrint2");
    mAllFiles.emplace_back("Fingerprints", "fileUnknownPrint1");
    mAllFiles.emplace_back("Fingerprints", "fileUnknownPrint2");
    mAllFiles.emplace_back("Fingerprints", "fileUnknownPrint3");
    mAllFiles.emplace_back("Fingerprints", "fileUnknownPrint4");
    mAllFiles.emplace_back("Fingerprints", "fileUnknownPrint5");
    mAllFiles.emplace_back("Fingerprints", "fileUnknownPrint6");
    mAllFiles.emplace_back("Fingerprints", "fileLSR1Print"); // Unused?
    mAllFiles.emplace_back("Fingerprints", "fileEstellesLSRPrint");
    mAllFiles.emplace_back("Images", "fileMap"); // 20
    mAllFiles.emplace_back("Images", "fileParchment1");
    mAllFiles.emplace_back("Images", "fileParchment2");
    mAllFiles.emplace_back("Images", "filePoussinPostcard");
    mAllFiles.emplace_back("Images", "fileTeniersPostcard1");
    mAllFiles.emplace_back("Images", "fileTeniersPostcard2");
    mAllFiles.emplace_back("Images", "fileHermeticSymbols");
    mAllFiles.emplace_back("Images", "fileSUMNote");
    mAllFiles.emplace_back("Audio", "fileAbbeTape");
    mAllFiles.emplace_back("Audio", "fileBuchelliTape");
    mAllFiles.emplace_back("Text", "fileArcadiaText"); // 30
    mAllFiles.emplace_back("Text", "fileTempleOfSolomonText"); //TODO: Is this a text type?
    mAllFiles.emplace_back("Images", "fileHermeticSymbols"); //TODO: Seems doubled up and unused?
    mAllFiles.emplace_back("Licenses", "fileBuchelliLicense");
    mAllFiles.emplace_back("Licenses", "fileEmilioLicense");
    mAllFiles.emplace_back("Licenses", "fileLadyHowardLicense");
    mAllFiles.emplace_back("Licenses", "fileMoselyLicense");
    mAllFiles.emplace_back("Licenses", "fileWilkesLicense");

    for(int i = 0; i < mAllFiles.size(); ++i)
    {
        mAllFiles[i].index = i;
    }
}

void SidneyFiles::AddFile(int fileIndex)
{
    if(fileIndex < 0 || fileIndex >= mAllFiles.size()) { return; }

    // Find appropriate directory.
    SidneyDirectory* dir = FindDirectory(mAllFiles[fileIndex].type);
    if(dir != nullptr && !dir->HasFile(mAllFiles[fileIndex].name))
    {
        // Add file if not already present.
        dir->files.emplace_back(mAllFiles[fileIndex]);
    }
}

bool SidneyFiles::HasFile(const std::string& fileName) const
{
    for(auto& dir : mData)
    {
        for(auto& file : dir.files)
        {
            if(StringUtil::EqualsIgnoreCase(file.name, fileName))
            {
                return true;
            }
        }
    }
    return false;
}

SidneyDirectory* SidneyFiles::FindDirectory(const std::string& name)
{
    for(auto& dir : mData)
    {
        if(StringUtil::EqualsIgnoreCase(dir.name, name))
        {
            return &dir;
        }
    }
    return nullptr;
}