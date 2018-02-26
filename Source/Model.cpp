//
// Model.cpp
//
// Clark Kromenaker
// 
#include "Model.h"
#include "BinaryReader.h"
#include <iostream>
#include "Vector2.h"
#include "Vector3.h"
#include "Vector4.h"
#include "Mesh.h"
#include "Matrix3.h"
#include "Quaternion.h"

using namespace std;

Model::Model(string name, char* data, int dataLength) :
    Asset(name)
{
    ParseFromData(data, dataLength);
}

void Model::ParseFromData(char *data, int dataLength)
{
    BinaryReader reader(data, dataLength);
    
    // First 4 bytes: file identifier "LDOM" (MODL backwards).
    std::string identifier = reader.ReadString(4);
    if(identifier != "LDOM")
    {
        cout << "MOD file does not have MODL identifier!" << endl;
        return;
    }
    
    // 4 bytes: unknown - has thus far always been 265.
    reader.ReadUInt();
    
    // 4 bytes: number of meshes in this model.
    unsigned int numMeshes = reader.ReadUInt();
    //cout << "Number of meshes: " << numMeshes << endl;
    
    // 4 bytes: size of the model data in bytes.
    // Always 48 bytes LESS than the total size (b/c header data is 48 bytes).
    reader.ReadUInt();
    //cout << "Model file size: " << modelSize << endl;
    
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
        //cout << "Mesh " << i << endl;
        
        // 4 bytes: mesh block identifier "HSEM" (MESH backwards).
        identifier = reader.ReadString(4); //reader.Read(identifier, 4);
        if(identifier != "HSEM")
        {
            cout << "Expected MESH identifier. Instead got " << identifier << endl;
            return;
        }

        // These are in i,k,j order, rather than i,j,k, likely due to 3DS Max conventions.
        // 4 bytes: mesh's x-axis basis vector (i)
        // 4 bytes: mesh's z-axis basis vector (k)
        // 4 bytes: mesh's y-axis basis vector (j)
        Vector3 iBasis(reader.ReadFloat(), reader.ReadFloat(), reader.ReadFloat());
        Vector3 kBasis(reader.ReadFloat(), reader.ReadFloat(), reader.ReadFloat());
        Vector3 jBasis(reader.ReadFloat(), reader.ReadFloat(), reader.ReadFloat());
        //cout << "   i: " << iBasis << endl;
        //cout << "   j: " << jBasis << endl;
        //cout << "   k: " << kBasis << endl;
        
        // These basis vectors are orthogonal and represent a default rotation for the mesh.
        Matrix3 rotMat = Matrix3::MakeBasis(iBasis, jBasis, kBasis);
        Quaternion rotQat(rotMat);
        
        // 4 bytes: an (X, Y, Z) offset or position for placing this mesh.
        // Each mesh within the model has it's local offset from the model origin.
        // This if vital, for example, if a mesh contains a human's head, legs, arms...
        // want to position them all correctly relative to one another!
        Vector3 meshPos(reader.ReadFloat(), reader.ReadFloat(), reader.ReadFloat());
        //cout << "   Mesh Position: " << meshPos << endl;
        
        // 4 bytes: number of mesh group blocks in this mesh.
        unsigned int numMeshGroups = reader.ReadUInt();
        //cout << "   Number of mesh groups in mesh: " << numMeshGroups << endl;
        
        // 24 bytes: two more sets of floating point values.
        // Based on plot test, seems very likely these are min/max values for the mesh.
        Vector3 val4(reader.ReadFloat(), reader.ReadFloat(), reader.ReadFloat());
        Vector3 val5(reader.ReadFloat(), reader.ReadFloat(), reader.ReadFloat());
        //cout << "   Min: " << val4 << endl;
        //cout << "   Max: " << val5 << endl;
        
        // Now, we iterate over each mesh group in this mesh.
        for(int j = 0; j < numMeshGroups; j++)
        {
            // 4 bytes: mesh group block identifier "PRGM" (MGRP backwards).
            identifier = reader.ReadString(4); //reader.Read(identifier, 4);
            if(identifier != "PRGM")
            {
                cout << "Expected MGRP identifier." << endl;
                return;
            }
            
            // 32 bytes: the name of the mesh group.
            std::string meshGroupName = reader.ReadString(32);
            //cout << "       Mesh group name: " << meshGroupName << endl;
            
            // 4 bytes: unknown - often is (0x00FFFFFF), but not always.
            // Have also seen: 0x03773BB3, 0xFF000000, 0x50261200
            unsigned int val = reader.ReadUInt();
            if(val != 0x00FFFFFF)
            {
                cout << "Expected 0x00FFFFFF" << endl;
            }

            // 4 bytes: unknown - seems to always be 1.
            reader.ReadUInt();
            
            // Create mesh object and push onto array.
            Mesh* mesh = new Mesh();
            mMeshes.push_back(mesh);
            
            // Save offset to mesh.
            mesh->SetOffset(meshPos);
            mesh->SetRotation(rotQat);
            
            // Also, push group name onto texture name array.
            mTextureNames.push_back(std::string(meshGroupName));
            
            // 4 bytes: vertex count for this mesh group.
            int vertexCount = reader.ReadUInt();
            
            // Based on vertex count, we can allocate some arrays for data.
            float* vertexPositions = new float[vertexCount * 3];
            float* vertexNormals = new float[vertexCount * 3];
            float* vertexUVs = new float[vertexCount * 2];
            
            // 4 bytes: index count, for drawing indexed mesh.
            int indexCount = reader.ReadUInt();
            unsigned short* vertexIndexes = new unsigned short[indexCount * 3];

            // 4 bytes: number of LODK blocks in this mesh group. Not uncommon to be 0.
            unsigned int lodkCount = reader.ReadUInt();
            //cout << "Number of LODK blocks: " << numLodkBlocks << endl;
            
            // 4 bytes: unknown - always zero thus far.
            reader.ReadUInt();
            
            // The next n bytes represent a certain number of floating point values.
            // This is the (X, Y, Z) vertex data for the mesh (positions, normals, UVs).
            
            // First set of numbers represent positions for each vertex.
            //cout << "Positions: " << endl;
            for(int k = 0; k < vertexCount; k++)
            {
                Vector3 v(reader.ReadFloat(), reader.ReadFloat(), reader.ReadFloat());
                //cout << v;
                vertexPositions[k * 3] = v.GetX();
                vertexPositions[k * 3 + 1] = v.GetZ(); //v.GetY();
                vertexPositions[k * 3 + 2] = v.GetY(); //v.GetZ();
            }
            //cout << endl;
            mesh->SetPositions(&vertexPositions[0], vertexCount * 3);
            
            // Second set of numbers represent normals for each vertex.
            //cout << "Normals: " << endl;
            for(int k = 0; k < vertexCount; k++)
            {
                Vector3 v(reader.ReadFloat(), reader.ReadFloat(), reader.ReadFloat());
                //cout << v;
                vertexNormals[k * 3] = v.GetX();
                vertexNormals[k * 3 + 1] = v.GetY();
                vertexNormals[k * 3 + 2] = v.GetZ();
            }
            //cout << endl;
            
            // Possibly UV coordinates?
            //cout << "UV1: " << endl;
            for(int k = 0; k < vertexCount; k++)
            {
                Vector2 v(reader.ReadFloat(), reader.ReadFloat());
                //cout << v;
                vertexUVs[k * 2] = v.GetX();
                vertexUVs[k * 2 + 1] = v.GetY();
            }
            //cout << endl;
            mesh->SetUV1(vertexUVs, vertexCount * 2);
            
            // Next comes vertex indexes for drawing from an IBO.
            // Common sequence would be (2, 1, 0) or (5, 4, 3), referring to vertex indexes above.
            // Every 4th number seems out of place - not sure what they mean.
            // Seen: 0xF100 (241), 0x0000 (0), 0x0701 (263), 0x7F3F (16255), 0x56B1 (45398),
            // 0x9B3E (16027), 0x583F (16216), 0xCC0D (3532), 0xCD0D (3533)
            //cout << "Indexes: " << endl;
            for(int k = 0; k < indexCount; k++)
            {
                vertexIndexes[k * 3] = reader.ReadUShort();
                vertexIndexes[k * 3 + 1] = reader.ReadUShort();
                vertexIndexes[k * 3 + 2] = reader.ReadUShort();
                
                Vector3 v(vertexIndexes[k * 3], vertexIndexes[k * 3 + 1], vertexIndexes[k * 3 + 2]);
                //cout << v;
                reader.ReadUShort(); // WHAT IS THIS!?
            }
            //cout << endl;
            mesh->SetIndexes(vertexIndexes, indexCount * 3);
            
            // Next comes LODK blocks for this mesh group.
            // Not totally sure what these are for, but maybe LOD groups?
            for(int k = 0; k < lodkCount; k++)
            {
                // Identifier should be "KDOL" for this block.
                identifier = reader.ReadString(4);
                if(identifier != "KDOL")
                {
                    cout << "Expected LODK identifier. Instead found " << identifier << endl;
                    return;
                }
                
                // First three values in LODK block are counts for
                // how much data to read after.
                int unknownCount1 = reader.ReadUInt();
                int unknownCount2 = reader.ReadUInt();
                int unknownCount3 = reader.ReadUInt();
                
                // Read in all values. Currently don't know what they are though.
                for(int l = 0; l < unknownCount1; l++)
                {
                    reader.ReadUShort();
                    reader.ReadUShort();
                    reader.ReadUShort();
                    reader.ReadUShort();
                }
                for(int l = 0; l < unknownCount2; l++)
                {
                    reader.ReadUShort();
                    reader.ReadUShort();
                }
                for(int l = 0; l < unknownCount3; l++)
                {
                    reader.ReadUShort();
                }
            }
        }
        meshGroupCount += numMeshGroups;
    }
    
    // After all meshes and mesh groups, there is some additional data.
    // 4 bytes: identifier "XDOM" (MODX backwards).
    identifier = reader.ReadString(4); //reader.Read(identifier, 4);
    if(identifier != "XDOM")
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
    //cout << "Mesh count: " << mMeshes.size() << endl;
}
