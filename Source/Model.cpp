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

Model::Model(std::string name, char* data, int dataLength) :
    Asset(name)
{
    ParseFromData(data, dataLength);
}

void Model::ParseFromData(char *data, int dataLength)
{
    //std::cout << "MOD " << mName << std::endl;
    BinaryReader reader(data, dataLength);
    
    // First 4 bytes: file identifier "LDOM" (MODL backwards).
    std::string identifier = reader.ReadString(4);
    if(identifier != "LDOM")
    {
        std::cout << "MOD file does not have MODL identifier!" << std::endl;
        return;
    }
    
    // 4 bytes: first 2 are a major/minor version number. Next 2 unknown.
    reader.ReadUInt();
    
    // 4 bytes: number of meshes in this model.
    unsigned int numMeshes = reader.ReadUInt();
    //std::cout << "  Number of meshes: " << numMeshes << std::endl;
    
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
        //std::cout << "  Mesh " << i << std::endl;
        
        // 4 bytes: mesh block identifier "HSEM" (MESH backwards).
        identifier = reader.ReadString(4);
        if(identifier != "HSEM")
        {
            std::cout << "Expected MESH identifier. Instead got " << identifier << std::endl;
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
        //cout << "   x: " << Vector3::Dot(iBasis, Vector3::Cross(jBasis, kBasis)) << endl;
        
        // From the basis vectors, calculate a quaternion representing
        // a rotation from the standard basis to that basis. We also need to negate some elements
        // to represent "reflection" from a right-handed rotation to a left-handed rotation.
        Quaternion rotQuat = Quaternion(Matrix3::MakeBasis(iBasis, jBasis, kBasis));
        rotQuat.SetZ(-rotQuat.GetZ());
        rotQuat.SetW(-rotQuat.GetW());
        
        // 4 bytes: an (X, Y, Z) offset or position for placing this mesh.
        // Each mesh within the model has it's local offset from the model origin.
        // This if vital, for example, if a mesh contains a human's head, legs, arms...
        // want to position them all correctly relative to one another!
        Vector3 meshPos(reader.ReadFloat(), reader.ReadFloat(), -reader.ReadFloat());
        //std::cout << "    Mesh Position: " << meshPos << std::endl;
        
        // Use mesh position offset and rotation values to create a local transform matrix.
        Matrix4 transMatrix = Matrix4::MakeTranslate(meshPos);
        Matrix4 rotMatrix = Matrix4::MakeRotate(rotQuat);
        Matrix4 localTransformMatrix = transMatrix * rotMatrix;
        
        // 4 bytes: number of mesh group blocks in this mesh.
        unsigned int numMeshGroups = reader.ReadUInt();
        //std::cout << "    Number of mesh groups in mesh: " << numMeshGroups << std::endl;
        
        // 24 bytes: two more sets of floating point values.
        // Based on plot test, seems very likely these are min/max values for the mesh.
        Vector3 minVal(reader.ReadFloat(), reader.ReadFloat(), reader.ReadFloat());
        Vector3 maxVal(reader.ReadFloat(), reader.ReadFloat(), reader.ReadFloat());
        
        // Now, we iterate over each mesh group in this mesh.
        for(int j = 0; j < numMeshGroups; j++)
        {
            //std::cout << "    Mesh Group " << j << std::endl;
            
            // 4 bytes: mesh group block identifier "PRGM" (MGRP backwards).
            identifier = reader.ReadString(4);
            if(identifier != "PRGM")
            {
                std::cout << "Expected MGRP identifier." << std::endl;
                return;
            }
            
            // 32 bytes: the name of the texture for this mesh group
            std::string textureName = reader.ReadString(32);
            //std::cout << "      Texture name: " << textureName << std::endl;
            
            // 4 bytes: unknown - often is (0x00FFFFFF), but not always.
            // Have also seen: 0x03773BB3, 0xFF000000, 0x50261200
            // Maybe a color value?
            reader.ReadUInt();

            // 4 bytes: unknown - seems to always be 1.
            reader.ReadUInt();
            
            // 4 bytes: vertex count for this mesh group.
            int vertexCount = reader.ReadUInt();
            //std::cout << "      Vertex count: " << vertexCount << std::endl;
            
            // Create mesh object and push onto array.
            Mesh* mesh = new Mesh(vertexCount, 8 * sizeof(float), MeshUsage::Static);
            mMeshes.push_back(mesh);
            
            // Save local offset and rotation of mesh.
            mesh->SetLocalTransformMatrix(localTransformMatrix);
            
            // Save texture name.
            mesh->SetTextureName(textureName);
            
            // Based on vertex count, we can allocate some arrays for data.
            float* vertexPositions = new float[vertexCount * 3];
            float* vertexNormals = new float[vertexCount * 3];
            float* vertexUVs = new float[vertexCount * 2];
            
            // 4 bytes: index count, for drawing indexed mesh.
            int indexCount = reader.ReadUInt();
            unsigned short* vertexIndexes = new unsigned short[indexCount * 3];
            //std::cout << "      Index count: " << indexCount << std::endl;
            
            // 4 bytes: number of LODK blocks in this mesh group. Not uncommon to be 0.
            unsigned int lodkCount = reader.ReadUInt();
            //std::cout << "      LODK count: " << lodkCount << std::endl;
            
            // 4 bytes: unknown - always zero thus far.
            reader.ReadUInt();
            
            // The next n bytes represent a number of floating point values.
            // This is the vertex data for the mesh (positions, normals, UVs).
            
            // Vertex positions.
            for(int k = 0; k < vertexCount; k++)
            {
                Vector3 pos(reader.ReadFloat(), reader.ReadFloat(), reader.ReadFloat());
                vertexPositions[k * 3] = pos.GetX();
                vertexPositions[k * 3 + 1] = pos.GetZ();
                vertexPositions[k * 3 + 2] = -pos.GetY();
            }
            mesh->SetPositions(vertexPositions);
            
            // Vertex normals.
            for(int k = 0; k < vertexCount; k++)
            {
                Vector3 normal(reader.ReadFloat(), reader.ReadFloat(), reader.ReadFloat());
                vertexNormals[k * 3] = normal.GetX();
                vertexNormals[k * 3 + 1] = normal.GetZ();
                vertexNormals[k * 3 + 2] = normal.GetY();
            }
            mesh->SetNormals(vertexNormals);
            
            // Vertex UV coordinates.
            for(int k = 0; k < vertexCount; k++)
            {
                Vector2 uv(reader.ReadFloat(), reader.ReadFloat());
                vertexUVs[k * 2] = uv.GetX();
                vertexUVs[k * 2 + 1] = uv.GetY();
            }
            mesh->SetUV1(vertexUVs);
            
            // Next comes vertex indexes for drawing from an IBO.
            // Common sequence would be (2, 1, 0) or (5, 4, 3), referring to vertex indexes above.
            for(int k = 0; k < indexCount; k++)
            {
                vertexIndexes[k * 3] = reader.ReadUShort();
                vertexIndexes[k * 3 + 1] = reader.ReadUShort();
                vertexIndexes[k * 3 + 2] = reader.ReadUShort();
                
                // Every 4th number seems out of place - not sure what they mean.
                // Seen: 0xF100 (241), 0x0000 (0), 0x0701 (263), 0x7F3F (16255), 0x56B1 (45398),
                // 0x9B3E (16027), 0x583F (16216), 0xCC0D (3532), 0xCD0D (3533)
                reader.ReadUShort(); // WHAT IS IT!?
            }
            mesh->SetIndexes(vertexIndexes, indexCount * 3);
            
            // Next comes LODK blocks for this mesh group.
            // Not totally sure what these are for, but maybe LOD groups?
            for(int k = 0; k < lodkCount; k++)
            {
                // Identifier should be "KDOL" for this block.
                identifier = reader.ReadString(4);
                if(identifier != "KDOL")
                {
                    std::cout << "Expected LODK identifier. Instead found " << identifier << std::endl;
                    return;
                }
                
                // First three values in LODK block are counts for
                // how much data to read after.
                int unknownCount1 = reader.ReadUInt();
                int unknownCount2 = reader.ReadUInt();
                int unknownCount3 = reader.ReadUInt();
                //std::cout << k << ": " << unknownCount1 << ", " << unknownCount2 << ", " << unknownCount3 << std::endl;
                
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
    identifier = reader.ReadString(4);
    if(identifier != "XDOM")
    {
        std::cout << "Expected MODX identifier." << std::endl;
        return;
    }
    
    /*
    // There seems to always be exactly one GRPX block for each MGRP block earlier.
    // Each GRPX block's size correlates in some way to the size of the earlier MGRP block...
    for(int i = 0; i < meshGroupCount; i++)
    {
        // 4 bytes: identifier "XPRG" (GRPX backwards).
        identifier = reader.ReadString(4);
        if(identifier != "XPRG")
        {
            cout << "Expected GRPX identifier." << endl;
            return;
        }
        
        // 4 bytes: vertex count (why, what good is that?)
        uint vertexCount = reader.ReadUInt();
        
        // The next n bytes represent a certain number of ushort values.
        // Not yet clear how to derive the size of this block, however.
    }
    */
}
