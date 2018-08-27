//
// VertexAnimation.cpp
//
// Clark Kromenaker
//
#include "VertexAnimation.h"
#include "BinaryReader.h"
#include <iostream>
#include <vector>
#include <unordered_map>
#include "Math.h"
#include "Matrix3.h"

//#define DEBUG_OUTPUT

VertexAnimation::VertexAnimation(std::string name, char* data, int dataLength) : Asset(name)
{
    ParseFromData(data, dataLength);
}

VertexAnimationMeshPose VertexAnimation::TakeSample(float time, int meshIndex)
{
    float secondsPerFrame = 1.0f / mFramesPerSecond;
    float localTime = Math::Mod(time, GetDuration());
    
    float prevPoseTime = 0.0f;
    float nextPoseTime = 0.0f;
    VertexAnimationMeshPose* prevPose = nullptr;
    VertexAnimationMeshPose* nextPose = nullptr;
    for(int i = 0; i < mFrames.size(); i++)
    {
        // See if this frame has a pose for the desired mesh index.
        VertexAnimationMeshPose* meshPose = nullptr;
        for(int j = 0; j < mFrames[i]->mMeshPoses.size(); j++)
        {
            if(mFrames[i]->mMeshPoses[j]->mMeshIndex == meshIndex)
            {
                meshPose = mFrames[i]->mMeshPoses[j];
                break;
            }
        }
        
        // If this frame doesn't have a pose for the desired mesh index, skip it.
        if(meshPose == nullptr) { continue; }
        
        // Determine the time of this frame in the animation.
        float frameTime = secondsPerFrame * i;
        
        // Based on whether the sample time is after this frame, exactly at this frame,
        // or before this frame, save our prev/next poses. Once equal or after, we can also break.
        if(frameTime < localTime)
        {
            prevPose = meshPose;
            prevPoseTime = frameTime;
        }
        else if(frameTime == localTime)
        {
            prevPose = nextPose = meshPose;
            prevPoseTime = nextPoseTime = frameTime;
            break;
        }
        else if(frameTime > localTime)
        {
            nextPose = meshPose;
            nextPoseTime = frameTime;
            break;
        }
    }
    
    // If prev pose is null, we are screwed!
    if(prevPose == nullptr)
    {
        return VertexAnimationMeshPose();
    }
    
    // This shouldn't happen, but if it does, just use the same as prev pose.
    if(nextPose == nullptr)
    {
        nextPose = prevPose;
        nextPoseTime = prevPoseTime;
    }
    
    // Determine our "t" value between the prev and next pose.
    float t = 1.0f;
    if(nextPoseTime != prevPoseTime)
    {
        t = (localTime - prevPoseTime) / (nextPoseTime - prevPoseTime);
    }
    
    // Create our pose struct to return.
    VertexAnimationMeshPose pose;
    
    // Now calculate interpolated positions between those poses for this time t.
    for(int i = 0; i < prevPose->mVertexPositions.size(); i++)
    {
        pose.mVertexPositions.push_back(Vector3::Lerp(prevPose->mVertexPositions[i], nextPose->mVertexPositions[i], t));
    }
    
    // Also interpolate translation/rotation.
    pose.mLocalPosition = Vector3::Lerp(prevPose->mLocalPosition, nextPose->mLocalPosition, t);
    Quaternion::Slerp(pose.mLocalRotation, prevPose->mLocalRotation, nextPose->mLocalRotation, t);
    return pose;
}

void VertexAnimation::ParseFromData(char *data, int dataLength)
{
    #ifdef DEBUG_OUTPUT
    std::cout << "Vertex Animation " << mName << std::endl;
    #endif
    BinaryReader reader(data, dataLength);
    
    // First 4 bytes: file identifier "HTCA" (ACT backwards, but what's the H for?)
    std::string identifier = reader.ReadString(4);
    if(identifier != "HTCA")
    {
        std::cout << "ACT file does not have ACTH identifier!" << std::endl;
        return;
    }
    
    // 4 bytes: Unknown - probably a version number for the file format.
    reader.ReadUInt();
    
    // 4 bytes: Number of keyframes in the animation.
    uint keyframeCount = reader.ReadUInt();
    
    // 4 bytes: The number of mesh entries that exist for each keyframe in this animation.
    // This should 100% correlate to the mesh count for the model file itself. If not, the animation probably won't play correctly.
    uint meshCount = reader.ReadUInt();
    
    // File contents size after header info. Not important to us.
    reader.ReadUInt();
    
    // Name of .MOD file this animation data is for.
    // TODO: May want to hash and save this for verification before playback, to avoid mismatched model/anim playing.
    std::string modelName = reader.ReadString(32);
    #ifdef DEBUG_OUTPUT
    std::cout << "  Model Name: " << modelName << std::endl;
    std::cout << "  Mesh Count: " << meshCount << std::endl;
    std::cout << "  Keyframe Count: " << keyframeCount << std::endl;
    #endif
    
    // Next is a byte offset within the data for each keyframe.
    // We will just read the data in order below, but this is useful to assert that we are aligned for each keyframe.
    std::vector<uint> offsets;
    for(int i = 0; i < keyframeCount; i++)
    {
        offsets.push_back(reader.ReadUInt());
    }
    
    // Read in data for each keyframe.
    std::unordered_map<int, std::unordered_map<int, VertexAnimationMeshPose*>> lastPoseLookup;
    for(int i = 0; i < keyframeCount; i++)
    {
        #ifdef DEBUG_OUTPUT
        std::cout << "  Keyframe " << i << std::endl;
        #endif
        
        // Validation: current reader offset should be at the offset defined at top of the file.
        assert(reader.GetPosition() == offsets[i]);
        
        // Create keyframe and put into keyframe array.
        VertexAnimationFrame* keyframe = new VertexAnimationFrame();
        mFrames.push_back(keyframe);
        
        // Every keyframe will have an entry for each mesh within the model.
        // However, it IS possible for there to be no data (byteCount == 0).
        // No data means there's no change for the vertex positions on this keyframe (use the same vertex data as last frame).
        for(int j = 0; j < meshCount; j++)
        {
            #ifdef DEBUG_OUTPUT
            std::cout << "    Mesh " << j << std::endl;
            #endif
            
            // The original GK3 data has the idea of meshes that can contain one or more "sub meshes" (or mesh groups).
            // For our code, we'd like to ignore this notion and just deal in "meshes".
            // To do this, we need to "unroll" some of the data that assumes the original data structure.
            
            // In this case, there may be more than one vertex position data for a single mesh.
            // But if so, any min/max or transform matrix data is expected to be used for all of them.
            // So, we keep this list of created mesh datas so we can assign to all of them.
            std::vector<VertexAnimationMeshPose*> createdPoses;
            Vector3 minPos;
            Vector3 maxPos;
            Quaternion localRotation;
            Vector3 localPosition;
            
            // 2 bytes: Index of mesh in MOD file.
            // It should always be ordered (0, 1, 2, 3), so we check that here as well.
            ushort index = reader.ReadUShort();
            assert(index == j);
            
            // 4 bytes: Number of bytes of data for this mesh in this keyframe.
            // All bytes from here contain vertex or transform data for this mesh in this keyframe.
            uint byteCount = reader.ReadUInt();
            while(byteCount > 0)
            {
                // 1 byte: A flag indicating what type of data is coming next.
                uint8_t identifier = reader.ReadUByte();
                byteCount -= 1;
                
                // Identifier 0 is uncompressed vertex data.
                if(identifier == 0)
                {
                    #ifdef DEBUG_OUTPUT
                    std::cout << "      Vertex Data Block" << std::endl;
                    #endif
                    
                    // Create mesh pose for this vertex data.
                    VertexAnimationMeshPose* meshPose = new VertexAnimationMeshPose();
                    keyframe->mMeshPoses.push_back(meshPose);
                    createdPoses.push_back(meshPose);
                    
                    // 4 bytes: Number of bytes in this block.
                    uint blockByteCount = reader.ReadUInt();
                    byteCount -= blockByteCount + 4;
                    
                    // 2 bytes: Mesh group within mesh this block refers to.
                    ushort submeshIndex = reader.ReadUShort();
                    if(i == 0)
                    {
                        meshPose->mMeshIndex = (int)keyframe->mMeshPoses.size() - 1;
                    }
                    else
                    {
                        meshPose->mMeshIndex = lastPoseLookup[index][submeshIndex]->mMeshIndex;
                    }
                    #ifdef DEBUG_OUTPUT
                    std::cout << "        Submesh Index: " << submeshIndex << std::endl;
                    #endif
                    
                    // 2 bytes: Vertex count.
                    ushort vertexCount = reader.ReadUShort();
                    #ifdef DEBUG_OUTPUT
                    std::cout << "        Vertex Count: " << vertexCount << std::endl;
                    #endif
                    
                    // Next, three floats per vertex (X, Y, Z).
                    for(int i = 0; i < vertexCount; i++)
                    {
                        float x = reader.ReadFloat();
                        float z = -reader.ReadFloat();
                        float y = reader.ReadFloat();
                        meshPose->mVertexPositions.push_back(Vector3(x, y, z));
                    }
                    
                    #ifdef DEBUG_OUTPUT
                    std::cout << "        Vertex Positions: " << std::endl;
                    for(auto& pos : meshPose->mVertexPositions)
                    {
                        std::cout << pos << std::endl;
                    }
                    std::cout << std::endl;
                    #endif
                    
                    // Save last pose lookup.
                    if(i == 0)
                    {
                        lastPoseLookup[index][submeshIndex] = meshPose;
                    }
                }
                // Identifier 1 also appears to be vertex data, but in a compressed format.
                else if(identifier == 1)
                {
                    #ifdef DEBUG_OUTPUT
                    std::cout << "      Compressed Vertex Data Block" << std::endl;
                    #endif
                    
                    // Create mesh pose for this vertex data.
                    VertexAnimationMeshPose* meshPose = new VertexAnimationMeshPose();
                    keyframe->mMeshPoses.push_back(meshPose);
                    createdPoses.push_back(meshPose);
                    
                    // 4 bytes: Number of bytes in this block.
                    uint blockByteCount = reader.ReadUInt();
                    byteCount -= blockByteCount + 4;
                    
                    // 2 bytes: Mesh group within mesh this block refers to.
                    ushort submeshIndex = reader.ReadUShort();
                    if(i == 0)
                    {
                        meshPose->mMeshIndex = (int)keyframe->mMeshPoses.size() - 1;
                    }
                    else
                    {
                        meshPose->mMeshIndex = lastPoseLookup[index][submeshIndex]->mMeshIndex;
                    }
                    #ifdef DEBUG_OUTPUT
                    std::cout << "        Submesh Index: " << submeshIndex << std::endl;
                    #endif
                    
                    // 2 bytes: Vertex count.
                    ushort vertexCount = reader.ReadUShort();
                    #ifdef DEBUG_OUTPUT
                    std::cout << "        Vertex Count: " << vertexCount << std::endl;
                    #endif
                    
                    // Next ((VertexCount/4) + 1) bytes: Compression info for vertex data.
                    // Every 2 bits indicates how the vertex at that index is compressed.
                    ushort compressionInfoSize = (vertexCount / 4) + 1;
                    unsigned char* compressionInfo = new unsigned char[compressionInfoSize];
                    reader.Read(compressionInfo, compressionInfoSize);
                    
                    // Iterate through the compression info and, for each vertex, determine
                    // how it is stored (uncompressed, compressed, not present at all)
                    unsigned char* vertexDataFormat = new unsigned char[vertexCount];
                    for(int k = 0; k < vertexCount; k += 4)
                    {
                        unsigned char byte = compressionInfo[k / 4];
                        uint val1 = (byte & 0x3);
                        uint val2 = (byte & 0xC) >> 2;
                        uint val3 = (byte & 0x30) >> 4;
                        uint val4 = (byte & 0xC0) >> 6;
                        
                        vertexDataFormat[k] = val1;        // Masking 0000 0011
                        vertexDataFormat[k + 1] = val2;    // Masking 0000 1100
                        vertexDataFormat[k + 2] = val3;    // Masking 0011 0000
                        vertexDataFormat[k + 3] = val4;    // Masking 1100 0000
                    }
                    
                    // Find position data from last recorded frame.
                    std::vector<Vector3>& prevPositions = lastPoseLookup[index][submeshIndex]->mVertexPositions;
                   
                    // Now that we have deciphered how each vertex is compressed, we can read in each vertex.
                    for(int k = 0; k < vertexCount; k++)
                    {
                        // 0 means no vertex data, so don't do anything!
                        if(vertexDataFormat[k] == 0)
                        {
                            meshPose->mVertexPositions.push_back(prevPositions[k]);
                        }
                        // 1 means (X, Y, Z) are compressed in next 3 bytes.
                        else if(vertexDataFormat[k] == 1)
                        {
                            float x = DecompressFloatFromByte(reader.ReadByte());
                            float z = -DecompressFloatFromByte(reader.ReadByte());
                            float y = DecompressFloatFromByte(reader.ReadByte());
                            meshPose->mVertexPositions.push_back(prevPositions[k] + Vector3(x, y, z));
                        }
                        // 2 means (X, Y, Z) are compressed in next 3 ushorts.
                        else if(vertexDataFormat[k] == 2)
                        {
                            float x = DecompressFloatFromByte(reader.ReadUShort());
                            float z = -DecompressFloatFromByte(reader.ReadUShort());
                            float y = DecompressFloatFromByte(reader.ReadUShort());
                            meshPose->mVertexPositions.push_back(prevPositions[k] + Vector3(x, y, z));
                        }
                        // 3 means (X, Y, Z) are not compressed - just floats.
                        else if(vertexDataFormat[k] == 3)
                        {
                            float x = reader.ReadFloat();
                            float z = -reader.ReadFloat();
                            float y = reader.ReadFloat();
                            meshPose->mVertexPositions.push_back(prevPositions[k] + Vector3(x, y, z));
                        }
                    }
                    
                    // Don't need these anymore!
                    delete[] compressionInfo;
                    delete[] vertexDataFormat;
                    
                    #ifdef DEBUG_OUTPUT
                    std::cout << "        Vertex Positions: " << std::endl;
                    for(auto& pos : meshPose->mVertexPositions)
                    {
                        std::cout << pos << std::endl;
                    }
                    std::cout << std::endl;
                    #endif
                    
                    // Save last pose lookup.
                    if(i == 0)
                    {
                        lastPoseLookup[index][submeshIndex] = meshPose;
                    }
                }
                // Identifier 2 is transform matrix data.
                else if(identifier == 2)
                {
                    //std::cout << "      Matrix Data Block" << std::endl;
                    
                    // 4 bytes: Number of bytes in this block.
                    // Should always be 48 bytes, for 12 floats, which is a 4x3 matrix of data.
                    uint blockByteCount = reader.ReadUInt();
                    assert(blockByteCount == 48);
                    byteCount -= blockByteCount + 4;
                    //std::cout << "        Byte Count: " << blockByteCount << std::endl;
                    
                    // These are in i,k,j order, rather than i,j,k, likely due to 3DS Max conventions.
                    // 12 bytes: mesh's x-axis basis vector (i)
                    // 12 bytes: mesh's z-axis basis vector (k)
                    // 12 bytes: mesh's y-axis basis vector (j)
                    Vector3 iBasis(reader.ReadFloat(), reader.ReadFloat(), reader.ReadFloat());
                    Vector3 kBasis(reader.ReadFloat(), reader.ReadFloat(), reader.ReadFloat());
                    Vector3 jBasis(reader.ReadFloat(), reader.ReadFloat(), reader.ReadFloat());
                    
                    // From the basis vectors, calculate a quaternion representing
                    // a rotation from the standard basis to that basis. We also need to negate some elements
                    // to represent "reflection" from a right-handed rotation to a left-handed rotation.
                    Quaternion rotQuat = Quaternion(Matrix3::MakeBasis(iBasis, jBasis, kBasis));
                    rotQuat.SetZ(-rotQuat.GetZ());
                    rotQuat.SetW(-rotQuat.GetW());
                    localRotation = rotQuat;
                    
                    // 12 bytes: an (X, Y, Z) offset or position for placing this mesh.
                    // Each mesh within the model has it's local offset from the model origin.
                    // This if vital, for example, if a mesh contains a human's head, legs, arms...
                    // want to position them all correctly relative to one another!
                    Vector3 meshPos(reader.ReadFloat(), reader.ReadFloat(), -reader.ReadFloat());
                    localPosition = meshPos;
                    //std::cout << "    Mesh Position: " << meshPos << std::endl;
                }
                // Identifier 3 is min/max data.
                else if(identifier == 3)
                {
                    //std::cout << "      Min/Max Data Block" << std::endl;
                    
                    // 4 bytes: Number of bytes in this block.
                    // Should always be 24 bytes, for 6 floats, which is two Vector3.
                    uint blockByteCount = reader.ReadUInt();
                    assert(blockByteCount == 24);
                    byteCount -= blockByteCount + 4;
                    //std::cout << "        Byte Count: " << blockByteCount << std::endl;
                    
                    // Assign min/max data.
                    minPos = Vector3(reader.ReadFloat(), reader.ReadFloat(), reader.ReadFloat());
                    maxPos = Vector3(reader.ReadFloat(), reader.ReadFloat(), reader.ReadFloat());
                }
                else
                {
                    std::cout << "Unexpected identifier " << (int)identifier << std::endl;
                }
            } // while(byteCount > 0)
            
            // Make sure min/max values and transform matrices are assigned to all poses.
            for(auto& pose : createdPoses)
            {
                pose->mMin = minPos;
                pose->mMax = maxPos;
                pose->mLocalPosition = localPosition;
                pose->mLocalRotation = localRotation;
            }
            
            // Check that we have data in the MeshData structure.
            // If not, we want to just reuse data from a previous keyframe.
            /*
            if(meshData.positions.size() == 0)
            {
                meshData.positions = mFrames[mFrames.size() - 1].meshDatas[j].positions;
            }
            if(mFrames.size() > 0 && meshData.positions.size() < mFrames[mFrames.size() - 1].meshDatas[j].positions.size())
            {
                for(int k = (int)meshData.positions.size(); k < (int)mFrames[mFrames.size() - 1].meshDatas[j].positions.size(); k++)
                {
                    meshData.positions.push_back(mFrames[mFrames.size() - 1].meshDatas[j].positions[k]);
                }
            }
            */
        } // iterate mesh groups
    } // iterate keyframes
}

float VertexAnimation::DecompressFloatFromByte(unsigned char byte)
{
    uint signFlag = (byte & 0x80) >> 7;
    float sign = (signFlag == 0) ? 1.0f : -1.0f;
    
    float frac = (float)(byte & 0x1F) / 32.0f;
    float whole = (byte & 0x7F) >> 5;
    
    return sign * (whole + frac);
}

float VertexAnimation::DecompressFloatFromUShort(unsigned short ushort)
{
    uint signFlag = (ushort & 0x8000) >> 15;
    float sign = (signFlag == 0) ? 1.0f : -1.0f;
    
    float frac = (float)(ushort & 0x1FF) / 256.0f;
    float whole = (ushort & 0x7FFF) >> 8;
    
    return sign * (whole + frac);
}
