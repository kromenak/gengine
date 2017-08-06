//
//  BarnFile.cpp
//  GEngine
//
//  Created by Clark Kromenaker on 8/4/17.
//

#include "BarnFile.h"
#include "BinaryReader.h"
#include <iostream>
#include <vector>

using namespace std;

BarnFile::BarnFile(const char* filePath)
{
    BinaryReader reader(filePath);
    
    // 8-bytes:  file identifier (GK3!Barn)
    char fileIdentifier[9];
    reader.Read(fileIdentifier, 8);
    fileIdentifier[8] = '\0';
    if(strcmp(fileIdentifier, "GK3!Barn") != 0)
    {
        cout << "Invalid file type!" << endl;
    }
    
    // 4-bytes: unknown constant value (65536)
    reader.ReadUInt();
    
    // 4-bytes: unknown constant value (65536)
    reader.ReadUInt();
    
    // 4-bytes: appears to be file size, or size of assets in BRN bundle.
    reader.ReadUInt();
    
    // 4-bytes: unknown constant value (268)
    unsigned int directoryOffset = reader.ReadUInt();
    cout << "Offset: " << directoryOffset << endl;

    // 4-bytes: EXE/Content build # (119 in both cases)
    reader.ReadUInt();
    reader.ReadUInt();
    
    // 4-bytes: unknown value
    reader.ReadUInt();
    
    // Two dates, 2-bytes per element.
    // The dates are both on the same day, just a few minutes apart.
    // Maybe like a build start/end time for the bundles?
    short year, month, day, hour, minute, second;
    year = reader.ReadShort();
    month = reader.ReadShort();
    reader.ReadShort();
    day = reader.ReadShort();
    hour = reader.ReadShort();
    minute = reader.ReadShort();
    second = reader.ReadShort();
    cout << year << "/" << month << "/" << day << ", " << hour << ":" << minute << ":" << second << endl;
    
    // 2-bytes: unknown variable value.
    reader.ReadShort();
    
    year = reader.ReadShort();
    month = reader.ReadShort();
    reader.ReadShort();
    day = reader.ReadShort();
    hour = reader.ReadShort();
    minute = reader.ReadShort();
    second = reader.ReadShort();
    cout << year << "/" << month << "/" << day << ", " << hour << ":" << minute << ":" << second << endl;
    
    // 2-bytes: unknown variable value.
    reader.ReadShort();
    
    // Copyright notice!
    char copyright[65];
    reader.Read(copyright, 64);
    copyright[64] = '\0';
    cout << copyright << endl;
    
    
    
    // Seek to directories offset.
    reader.Seek(directoryOffset);
    
    unsigned int directoryCount = reader.ReadUInt();
    cout << "Directory count: " << directoryCount << endl;
    
    vector<unsigned int> headerOffsets;
    headerOffsets.reserve(directoryCount);
    
    vector<unsigned int> dataOffsets;
    dataOffsets.reserve(directoryCount);
    
    // A certain number of "directories".
    // Each directory has a header and data section.
    for(int i = 0; i < directoryCount; ++i)
    {
        unsigned int type = reader.ReadUInt();
        reader.ReadUInt();
        reader.ReadUInt();
        reader.ReadUInt();
        reader.ReadUInt();
        
        unsigned int headerOffset = reader.ReadUInt();
        unsigned int dataOffset = reader.ReadUInt();
        
        if(type == kDirectoryIdentifier)
        {
            headerOffsets.push_back(headerOffset);
            dataOffsets.push_back(dataOffset);
        }
        else if(type == kDataIdentifier)
        {
            mDataOffset = headerOffset;
        }
    }
    
    for(int i = 0; i < headerOffsets.size(); ++i)
    {
        reader.Seek(headerOffsets[i]);
        
        char name[33];
        reader.Read(name, 32);
        name[32] = '\0';
        
        reader.ReadUInt();
        char dummy[40];
        reader.Read(dummy, 40);
        reader.ReadUInt();
        
        int numAssets = reader.ReadUInt();
        
        reader.Seek(dataOffsets[i]);
        
        for(int j = 0; j < numAssets; j++)
        {
            unsigned int assetSize = reader.ReadUInt();
            unsigned int assetOffset = reader.ReadUInt();
            
            reader.ReadUInt();
            reader.ReadUByte();
            
            unsigned short compressionType = reader.ReadUByte();
            
            unsigned char assetNameLength = reader.ReadUByte();
            char assetName[257];
            reader.Read(assetName, assetNameLength + 1);
            assetName[256] = '\0';
            
            cout << assetName << "(" << compressionType << ")" << endl;
        }
    }
    
    cout << "Done reading ToC" << endl;
}

