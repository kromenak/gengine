#include "SheepAPI_Assets.h"

#include "AssetManager.h"

using namespace std;

shpvoid AddPath(const std::string& pathName)
{
    gAssetManager.AddSearchPath(pathName);
    return 0;
}
RegFunc1(AddPath, void, string, IMMEDIATE, DEV_FUNC);

/*
shpvoid FullScanPaths()
{
    // Scans and indexes assets on all search paths.
    // Really only useful when dealing with loose files.
    return 0;
}
RegFunc0(FullScanPaths, void, IMMEDIATE, DEV_FUNC);

shpvoid RescanPaths()
{
    // Same as full scan paths, but dumps any existing indexes as well.
    return 0;
}
RegFunc0(RescanPaths, void, IMMEDIATE, DEV_FUNC);
*/

shpvoid Extract(const std::string& fileSpec, const std::string& outputPath)
{
    //TODO: fileSpec should support regex if surrounded by { }.
    //TODO: fileSpec prefixed with @ indicates a "listing file" - what's a listing file? Perhaps a file that lists assets to extract?

    // An empty output path defaults to EXE directory.
    // If a path is provided, absolute or relative paths will work.
    if(outputPath.empty())
    {
        gAssetManager.WriteBarnAssetToFile(fileSpec);
    }
    else
    {
        gAssetManager.WriteBarnAssetToFile(fileSpec, outputPath);
    }
    return 0;
}
RegFunc2(Extract, void, string, string, IMMEDIATE, REL_FUNC);

shpvoid NeedDiscResources(int discNum)
{
    // In the original game, this would check that all the resources from disk 1/2/3 were loaded.
    // If not, it would show the prompt to "Please Insert CD X".
    // This isn't needed right now, but could be implemented for completeness if we really wanted.
    return 0;
}
RegFunc1(NeedDiscResources, void, int, IMMEDIATE, REL_FUNC);