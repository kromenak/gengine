//
// Clark Kromenaker
//
// Data stored at the beginning of a save file.
// Mostly bookkeeping, save file summary, and thumbnail.
//
#pragma once
#include <cstdint>
#include <memory>
#include <string>

#include "PersistState.h"
#include "PNGCodec.h"
#include "Texture.h"

struct SaveHeader
{
    // Constants for identifying the save file.
    char productId[4] = { 'G', 'K', '3', '!' };
    char saveId[4] = { 'S', 'A', 'V', 'E' };

    // Save file version.
    int32_t saveVersion = 4;

    // Size of this header (after this point); always 232.
    int32_t saveHeaderSize = 232;

    // A product version number; unused - always 65536
    int32_t productVersion = 65536;

    // An executable build number.
    int32_t exeBuildNumber = 1;

    // Config exe was built in. Known values are "Release" and "Production".
    std::string exeMode = "Production";

    // Screen width and height when save was made (why is this relevant?).
    int32_t screenWidth = 0;
    int32_t screenHeight = 0;

    // A number that starts at 1 and increments each time a save is made for the current game playthrough.
    int32_t sessionSerialId = 1;

    // The date/time the save was made.
    // It *appears* this was originally a Windows SYSTEMTIME struct.
    // There's an extra 8 bytes in the binary that I have no idea about...
    uint8_t unknown1[8];
    uint16_t year;
    uint16_t month;
    uint16_t dayOfWeek;
    uint16_t day;
    uint16_t hour;
    uint16_t minute;
    uint16_t second;
    uint16_t milliseconds;

    // Name of the machine the save was made on, 32 chars max.
    char machineName[32] = "UNKNOWN-MACHINE";

    // Name of the user logged in when the save as made, 32 chars max.
    char userName[32] = "UNKNOWN-USER";

    // Copyright message.
    char copyright[100] = "Copyright © 1999 Sierra Studios. All rights reserved.";

    void OnPersist(PersistState& ps)
    {
        ps.Xfer("Product ID", productId, 4);
        ps.Xfer("Save ID", saveId, 4);
        ps.Xfer("Save Version", saveVersion);
        ps.Xfer("Save Header Size", saveHeaderSize);

        ps.Xfer("ProductVersion", productVersion);
        ps.Xfer("Executable Build", exeBuildNumber);
        ps.Xfer("Executable Mode", exeMode);

        ps.Xfer("Screen Width", screenWidth);
        ps.Xfer("Screen Height", screenHeight);

        ps.Xfer("Session Serial ID", sessionSerialId);

        ps.Xfer("Year", year);
        ps.Xfer("Month", month);
        ps.Xfer("DayOfWeek", dayOfWeek);
        ps.Xfer("Day", day);
        ps.Xfer("Hour", hour);
        ps.Xfer("Minute", minute);
        ps.Xfer("Second", second);
        ps.Xfer("Milliseconds", milliseconds);

        ps.Xfer("Machine Name", machineName, 32);
        ps.Xfer("User Name", userName, 32);
        ps.Xfer("Copyright Text", copyright, 100);
    }
};

struct PersistHeader
{
    int32_t saveVersion = 1;

    // User-specified save description.
    std::string userDescription;

    // Location and timeblock for the save.
    std::string location;
    std::string timeblock;

    // Current and max score.
    int32_t score = 0;
    int32_t maxScore = 0;

    // Current CD number.
    int32_t cdNumber = 1;

    // The thumbnail. Owned by this struct.
    std::unique_ptr<Texture> thumbnailTexture = nullptr;

    void OnPersist(PersistState& ps)
    {
        ps.Xfer("GK3 Save Version", saveVersion);
        ps.Xfer("User Description", userDescription);
        ps.Xfer("Location", location);
        ps.Xfer("TimeBlock", timeblock);
        ps.Xfer("Score", score);
        ps.Xfer("Max Score", maxScore);
        ps.Xfer("Last CD", cdNumber);

        if(ps.IsSaving())
        {
            uint32_t thumbnailSize = 0;
            uint8_t* thumbnailBytes = nullptr;
            if(thumbnailTexture != nullptr)
            {
                PNG::ImageData imageData;
                imageData.width = thumbnailTexture->GetWidth();
                imageData.height = thumbnailTexture->GetHeight();
                imageData.bytesPerPixel = thumbnailTexture->GetBytesPerPixel();
                imageData.pixelData = thumbnailTexture->GetPixelData();

                static const size_t kPngScratchSize = 83886080;
                static uint8_t pngScratch[kPngScratchSize];

                PNG::Encode(imageData, pngScratch, kPngScratchSize, thumbnailSize);
                thumbnailBytes = pngScratch;
            }

            ps.Xfer("Thumbnail-size", thumbnailSize);
            ps.Xfer("Thumbnail", thumbnailBytes, thumbnailSize);
        }
        else if(ps.IsLoading())
        {
            uint32_t thumbnailSize = 0;
            ps.Xfer("Thumbnail-size", thumbnailSize);

            if(thumbnailSize > 0)
            {
                thumbnailTexture = std::unique_ptr<Texture>(new Texture(*ps.GetBinaryReader()));
            }
        }
    }
};