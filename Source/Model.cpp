//
// Model.cpp
//
// Clark Kromenaker
// 

#include "Model.h"
#include "BinaryReader.h"
#include <iostream>
#include "Vector3.h"

using namespace std;

Model::Model(string name, char* data, int dataLength) :
    mName(name)
{
    ParseFromModFileData(data, dataLength);
}

void Model::ParseFromModFileData(char *data, int dataLength)
{
    BinaryReader reader(data, dataLength);
    
    // First 4 bytes: file identifier "LDOM" (MODL backwards).
    char identifier[4];
    reader.Read(identifier, 4);
    if(!strcmp(identifier, "LDOM"))
    {
        cout << "MOD file does not have MODL identifier!" << endl;
        return;
    }
    
    // 4 bytes: unknown - has thus far always been 265
    reader.ReadUInt();
    
    // 4 bytes: number of meshes in this model.
    unsigned int numMeshes = reader.ReadUInt();
    cout << "Number of meshes: " << numMeshes << endl;
    
    // 4 bytes: size of the model data in bytes.
    // Always 48 bytes LESS than the total size (b/c header data is 48 bytes).
    unsigned int modelSize = reader.ReadUInt();
    cout << "Model file size: " << modelSize << endl;
    
    // 4 bytes: unknown - usually zero, but not always (GAB.MOD had 0x0000C842).
    // Could maybe be a floating-point value? 0x000C842 = 100.0f
    reader.ReadUInt();
    
    // 24 bytes: unknown - all files have had zeros here thus far.
    reader.Skip(24);
    
    // 4 bytes: unknown - has thus far always been 8.
    reader.ReadUInt();
    
    // Now, we iterate over each mesh in the file.
    int meshGroupCount = 0;
    for(int i = 0; i < numMeshes; i++)
    {
        // 4 bytes: mesh block identifier "HSEM" (MESH backwards).
        reader.Read(identifier, 4);
        if(!strcmp(identifier, "HSEM"))
        {
            cout << "Expected MESH identifier." << endl;
            return;
        }

        // The next 48 bytes appear to be floating-point values.
        // Perhaps 4 Vector3(X, Y, Z) values?
        Vector3 val0(reader.ReadFloat(), reader.ReadFloat(), reader.ReadFloat());
        Vector3 val1(reader.ReadFloat(), reader.ReadFloat(), reader.ReadFloat());
        Vector3 val2(reader.ReadFloat(), reader.ReadFloat(), reader.ReadFloat());
        Vector3 val3(reader.ReadFloat(), reader.ReadFloat(), reader.ReadFloat());
        cout << "Val0: " << val0 << endl;
        cout << "Val1: " << val1 << endl;
        cout << "Val2: " << val2 << endl;
        cout << "Val3: " << val3 << endl;
        
        // 4 bytes: number of mesh group blocks in this mesh.
        unsigned int numMeshGroups = reader.ReadUInt();
        cout << "Number of mesh groups in mesh: " << numMeshGroups << endl;
        
        // 24 bytes: two more sets of floating point values.
        // These feel like they might be min/max bounds? Not totally sure.
        Vector3 val4(reader.ReadFloat(), reader.ReadFloat(), reader.ReadFloat());
        Vector3 val5(reader.ReadFloat(), reader.ReadFloat(), reader.ReadFloat());
        cout << "Val4: " << val4 << endl;
        cout << "Val5: " << val5 << endl;
        
        // Now, we iterate over each mesh group in this mesh.
        for(int j = 0; j < numMeshGroups; j++)
        {
            // 4 bytes: mesh group block identifier "PRGM" (MGRP backwards).
            reader.Read(identifier, 4);
            if(!strcmp(identifier, "PRGM"))
            {
                cout << "Expected MGRP identifier." << endl;
                return;
            }
            
            // 32 bytes: the name of the mesh group.
            char meshGroupName[32];
            reader.Read(meshGroupName, 32);
            cout << "Mesh group name: " << meshGroupName << endl;
            
            // 4 bytes: always the same value (0x00FFFFFF).
            unsigned int val = reader.ReadUInt();
            if(val != 0x00FFFFFF)
            {
                cout << "Expected 0x00FFFFFF" << endl;
                return;
            }

            // 4 bytes: unknown - seems to always be 1.
            reader.ReadUInt();
            
            // 4 bytes: unknown - seems to be a count value.
            // This count seems to indicate groupings of 8 pieces of data later.
            unsigned int unknownCount1 = reader.ReadUInt();
            
            // 4 bytes: unknown - seems to be a count value.
            // This count seems to indicate groupings of 2 pieces of data later.
            unsigned int unknownCount2 = reader.ReadUInt();
            
            // 4 bytes: number of LODK blocks in this mesh group. Not uncommon to be 0.
            unsigned int numLodkBlocks = reader.ReadUInt();
            cout << "Number of LODK blocks: " << numLodkBlocks << endl;
            
            // 4 bytes: unknown
            reader.ReadUInt();
            
            // The next n bytes represent a certain number of floating point values.
            // My guess is that this is the (X, Y, Z) vertex data for the mesh.
            // The number of bytes is always equal to:
            //  n = (unknownCount1 * 8 * 4) + (unknownCount2 * 2 * 4)
            // So, unknownCount1 indicates some number of 8-values, 32-bytes elements,
            // and unknownCount2 indicates some number of 2-values, 8-bytes elements
            reader.Skip(unknownCount1 * 8 * 4);
            reader.Skip(unknownCount2 * 2 * 4);
            
            //TODO: Parse LODK blocks. What do they mean?
        }
        
        meshGroupCount += numMeshGroups;
    }
    
    // After all meshes and mesh groups, there is some additional data.
    // 4 bytes: identifier "XDOM" (MODX backwards).
    reader.Read(identifier, 4);
    if(!strcmp(identifier, "XDOM"))
    {
        cout << "Expected MODX identifier." << endl;
        return;
    }
    
    // There seems to always be exactly one GRPX block for each MGRP block earlier.
    // Each GRPX block's size correlates in some way to the size of the earlier MGRP block...
    /*
    for(int i = 0; i < meshGroupCount; i++)
    {
        // 4 bytes: identifier "XPRG" (GRPX backwards).
        reader.Read(identifier, 4);
        if(!strcmp(identifier, "XPRG"))
        {
            cout << "Expected GRPX identifier." << endl;
            return;
        }
        
        // The next n bytes represent a certain number of ushort values.
        // Not yet clear how to derive the size of this block, however.
    }
    */
}
