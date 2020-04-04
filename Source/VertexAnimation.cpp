//
// VertexAnimation.cpp
//
// Clark Kromenaker
//
#include "VertexAnimation.h"

#include <cassert>
#include <iostream>
#include <vector>
#include <unordered_map>

#include "BinaryReader.h"
#include "GMath.h"
#include "Matrix3.h"

//#define DEBUG_OUTPUT

VertexAnimation::VertexAnimation(std::string name, char* data, int dataLength) : Asset(name)
{
    ParseFromData(data, dataLength);
}

Vector3 VertexAnimation::SampleVertexPosition(float time, int framesPerSecond, int meshIndex, int submeshIndex, int vertexIndex)
{
	float duration = GetDuration(framesPerSecond);
	float localTime = time;
	if(localTime > duration)
	{
		localTime = Math::Mod(time, duration);
	}
	
	float currentPoseTime = 0.0f;
    float nextPoseTime = 0.0f;
	
	// Find the first vertex pose defined for this mesh/submesh.
	VertexAnimationVertexPose* firstVertexPose = nullptr;
	auto it = mVertexPoses.find(meshIndex);
	if(it != mVertexPoses.end())
	{
		auto it2 = it->second.find(submeshIndex);
		if(it2 != it->second.end())
		{
			firstVertexPose = it2->second;
		}
	}
	
	// If no vertex pose was found, we'll have to return an error state.
	if(firstVertexPose == nullptr)
	{
		return Vector3::Zero;
	}
	
	// Calculate how many seconds should be used for a single frame - used later.
    float secondsPerFrame = 1.0f / framesPerSecond;
	
	// Determine the poses right before the desired local time on the animation.
    VertexAnimationVertexPose* currentVertexPose = firstVertexPose;
    while(currentVertexPose->mNext != nullptr)
    {
        currentPoseTime = secondsPerFrame * currentVertexPose->mFrameNumber;
        nextPoseTime = secondsPerFrame * currentVertexPose->mNext->mFrameNumber;
        if(nextPoseTime > localTime) { break; }
		
        currentVertexPose = currentVertexPose->mNext;
    }
	
	// Get the next pose, after the desired local time.
	// If it doesn't exist, loop back to the first pose.
    VertexAnimationVertexPose* nextVertexPose = currentVertexPose->mNext;
    if(nextVertexPose == nullptr)
    {
		nextVertexPose = currentVertexPose;
		nextPoseTime = currentPoseTime;
    }
    
    // Determine our "t" value between the current and next pose.
    float t = 1.0f;
    if(!Math::IsZero(nextPoseTime - currentPoseTime))
    {
        t = (localTime - currentPoseTime) / (nextPoseTime - currentPoseTime);
    }
    assert(t >= 0.0f && t <= 1.0f);
	
    // Now calculate interpolated positions between current and next poses for this time t.
	return Vector3::Lerp(currentVertexPose->mVertexPositions[vertexIndex], nextVertexPose->mVertexPositions[vertexIndex], t);
}

VertexAnimationVertexPose VertexAnimation::SampleVertexPose(float time, int framesPerSecond, int meshIndex, int submeshIndex)
{
	float duration = GetDuration(framesPerSecond);
	float localTime = time;
	if(localTime > duration)
	{
		localTime = Math::Mod(time, duration);
	}
	
    float currentPoseTime = 0.0f;
    float nextPoseTime = 0.0f;
	
	// Find the first vertex pose defined for this mesh/submesh.
	VertexAnimationVertexPose* firstVertexPose = nullptr;
	auto it = mVertexPoses.find(meshIndex);
	if(it != mVertexPoses.end())
	{
		auto it2 = it->second.find(submeshIndex);
		if(it2 != it->second.end())
		{
			firstVertexPose = it2->second;
		}
	}
	
	// If no vertex pose was found, we'll have to return an error state.
	if(firstVertexPose == nullptr)
	{
		VertexAnimationVertexPose pose;
		pose.mFrameNumber = -1;
		return pose;
	}
	
	// Calculate how many seconds should be used for a single frame - used later.
    float secondsPerFrame = 1.0f / framesPerSecond;
	
	// Determine the poses right before the desired local time on the animation.
    VertexAnimationVertexPose* currentVertexPose = firstVertexPose;
    while(currentVertexPose->mNext != nullptr)
    {
        currentPoseTime = secondsPerFrame * currentVertexPose->mFrameNumber;
        nextPoseTime = secondsPerFrame * currentVertexPose->mNext->mFrameNumber;
        if(nextPoseTime > localTime) { break; }
		
        currentVertexPose = currentVertexPose->mNext;
    }
	
	// Get the next pose, after the desired local time.
	// If it doesn't exist, loop back to the first pose.
    VertexAnimationVertexPose* nextVertexPose = currentVertexPose->mNext;
    if(nextVertexPose == nullptr)
    {
		// Clamp approach.
		nextVertexPose = currentVertexPose;
		nextPoseTime = currentPoseTime;
		
		// Loop approach.
        //nextVertexPose = firstVertexPose;
        //nextPoseTime = GetDuration(framesPerSecond);
    }
    
    // Determine our "t" value between the current and next pose.
    float t = 1.0f;
    if(!Math::IsZero(nextPoseTime - currentPoseTime))
    {
        t = (localTime - currentPoseTime) / (nextPoseTime - currentPoseTime);
    }
    assert(t >= 0.0f && t <= 1.0f);
	
    // Now calculate interpolated positions between current and next poses for this time t.
	VertexAnimationVertexPose pose;
    for(int i = 0; i < currentVertexPose->mVertexPositions.size(); i++)
    {
        pose.mVertexPositions.push_back(Vector3::Lerp(currentVertexPose->mVertexPositions[i], nextVertexPose->mVertexPositions[i], t));
    }
    return pose;
}

VertexAnimationTransformPose VertexAnimation::SampleTransformPose(float time, int framesPerSecond, int meshIndex)
{
	// Calculate how many seconds should be used for a single frame.
	float secondsPerFrame = 1.0f / framesPerSecond;
	
	// Caller may pass in a global time that extends beyond the local time of this particular animation.
	// Desire here is for the animation to "loop", so we calculate how many seconds in we are.
	float duration = GetDuration(framesPerSecond);
	float localTime = time;
	if(localTime > duration)
	{
		localTime = Math::Mod(time, duration);
	}
	
	// Determine between which two transform poses the desired local time is located.
	// E.g. if local time is 50% between pose 5 and 6, we  want to interpolate 50% between those two poses.
    float currentPoseTime = 0.0f;
    float nextPoseTime = 0.0f;
    VertexAnimationTransformPose* firstTransformPose = mTransformPoses[meshIndex];
    VertexAnimationTransformPose* currentTransformPose = firstTransformPose;
    while(currentTransformPose->mNext != nullptr)
    {
        currentPoseTime = secondsPerFrame * currentTransformPose->mFrameNumber;
        nextPoseTime = secondsPerFrame * currentTransformPose->mNext->mFrameNumber;
        if(nextPoseTime > localTime) { break; }
        
        // Move on to next one.
        currentTransformPose = currentTransformPose->mNext;
    }
	
	// If there is no "next" pose, we can either loop to the first pose, or "clamp" on the last pose.
	// Testing suggests GK3 expects the "clamp" approach, but more generally, a parameter for this might make sense.
    VertexAnimationTransformPose* nextTransformPose = currentTransformPose->mNext;
    if(nextTransformPose == nullptr)
    {
		// Clamp approach.
		nextTransformPose = currentTransformPose;
		nextPoseTime = currentPoseTime;
		
		// Loop approach.
        //nextTransformPose = firstTransformPose;
        //nextPoseTime = GetDuration(framesPerSecond);
    }
	
    // Determine our "t" value between the prev and next pose.
    float t = 1.0f;
	if(!Math::IsZero(nextPoseTime - currentPoseTime))
    {
        t = (localTime - currentPoseTime) / (nextPoseTime - currentPoseTime);
    }
    assert(t >= 0.0f && t <= 1.0f);
	
	// Finally, create a pose with lerp/slerp that is interpolated between the two poses.
    VertexAnimationTransformPose pose;
    pose.mLocalPosition = Vector3::Lerp(currentTransformPose->mLocalPosition, nextTransformPose->mLocalPosition, t);
	pose.mLocalScale = Vector3::Lerp(currentTransformPose->mLocalScale, nextTransformPose->mLocalScale, t);
    Quaternion::Slerp(pose.mLocalRotation, currentTransformPose->mLocalRotation, nextTransformPose->mLocalRotation, t);
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
    mFrameCount = reader.ReadUInt();
    
    // 4 bytes: The number of mesh entries that exist for each keyframe in this animation.
    // This should 100% correlate to the mesh count for the model file itself.
	// If not, the animation probably won't play correctly.
    uint32_t meshCount = reader.ReadUInt();
    
    // File contents size after header info. Not important to us.
    reader.ReadUInt();
    
    // Name of .MOD file this animation data is for.
    // TODO: May want to hash and save this for verification before playback, to avoid mismatched model/anim playing.
    mModelName = reader.ReadString(32);
    #ifdef DEBUG_OUTPUT
    std::cout << "  Model Name: " << mModelName << std::endl;
    std::cout << "  Mesh Count: " << meshCount << std::endl;
    std::cout << "  Frame Count: " << mFrameCount << std::endl;
    #endif
    
    // Next is a byte offset within the data for each keyframe.
    // We will just read the data in order below, but this is useful to assert that we are aligned for each keyframe.
    std::vector<unsigned int> offsets;
    for(int i = 0; i < mFrameCount; i++)
    {
        offsets.push_back(reader.ReadUInt());
    }
    
    std::unordered_map<int, VertexAnimationVertexPose*> lastVertexPoseLookup;
    std::unordered_map<int, VertexAnimationTransformPose*> lastTransformPoseLookup;
	
	// Read in data for each keyframe.
    for(int i = 0; i < mFrameCount; i++)
    {
        #ifdef DEBUG_OUTPUT
        std::cout << "  Frame " << i << std::endl;
        #endif
        
        // Validation: current reader offset should be at the offset defined at top of the file.
        assert(reader.GetPosition() == offsets[i]);
        
        // Every keyframe will have an entry for each mesh within the model.
        // However, it IS possible for there to be no data (byteCount == 0).
        // No data means there's no change for the vertex positions on this keyframe (use the same vertex data as last frame).
        for(int j = 0; j < meshCount; j++)
        {
            #ifdef DEBUG_OUTPUT
            std::cout << "    Mesh " << j << std::endl;
            #endif
            
            // 2 bytes: Index of mesh in MOD file.
            // It should always be ordered (0, 1, 2, 3), so we check that here as well.
            unsigned short meshIndex = reader.ReadUShort();
            assert(meshIndex == j);
            
            // 4 bytes: Number of bytes of data for this mesh in this keyframe.
            // All bytes from here contain vertex or transform data for this mesh in this keyframe.
            unsigned int byteCount = reader.ReadUInt();
            while(byteCount > 0)
            {
                // 1 byte: A flag indicating what type of data is coming next.
                unsigned int dataId = reader.ReadUByte();
                byteCount -= 1;
                
                // Identifier 0 is uncompressed vertex data.
                if(dataId == 0)
                {
                    #ifdef DEBUG_OUTPUT
                    std::cout << "      Vertex Data Block" << std::endl;
                    #endif
                    
                    // 4 bytes: Number of bytes in this block.
                    unsigned int blockByteCount = reader.ReadUInt();
                    byteCount -= blockByteCount + 4;
                    
                    // 2 bytes: Submesh within mesh this block refers to.
                    unsigned short submeshIndex = reader.ReadUShort();
                    #ifdef DEBUG_OUTPUT
                    std::cout << "        Submesh Index: " << submeshIndex << std::endl;
                    #endif
					
					// This is a pretty sorry hash...but assuming we'll never
					// have more than 1000 submeshes (pretty likely for GK3), this'll do OK.
                    int hash = meshIndex * 1000 + submeshIndex;
					
					// Create a vertex pose for this frame and stick it in our dictionary and linked list.
                    VertexAnimationVertexPose* vertexPose = new VertexAnimationVertexPose();
                    vertexPose->mFrameNumber = i;
                    if(i == 0)
                    {
                        mVertexPoses[meshIndex][submeshIndex] = vertexPose;
                        lastVertexPoseLookup[hash] = vertexPose;
                    }
                    else
                    {
                        lastVertexPoseLookup[hash]->mNext = vertexPose;
                        lastVertexPoseLookup[hash] = vertexPose;
                    }
                    
                    // 2 bytes: Vertex count.
                    unsigned short vertexCount = reader.ReadUShort();
                    #ifdef DEBUG_OUTPUT
                    std::cout << "        Vertex Count: " << vertexCount << std::endl;
                    #endif
                    
                    // Next, three floats per vertex (X, Y, Z).
                    for(int k = 0; k < vertexCount; k++)
                    {
                        float x = reader.ReadFloat();
						float z = reader.ReadFloat();
                        float y = reader.ReadFloat();
                        vertexPose->mVertexPositions.push_back(Vector3(x, y, z));
                    }
                }
                // Identifier 1 also is vertex data, but in a compressed format.
                else if(dataId == 1)
                {
                    #ifdef DEBUG_OUTPUT
                    std::cout << "      Compressed Vertex Data Block" << std::endl;
                    #endif
                    
                    // 4 bytes: Number of bytes in this block.
                    uint32_t blockByteCount = reader.ReadUInt();
                    byteCount -= blockByteCount + 4;
                    
                    // 2 bytes: Submesh within mesh this block refers to.
                    unsigned short submeshIndex = reader.ReadUShort();
                    #ifdef DEBUG_OUTPUT
                    std::cout << "        Submesh Index: " << submeshIndex << std::endl;
                    #endif
                    
                    // Find position data from last recorded frame.
                    int hash = meshIndex * 1000 + submeshIndex;
                    std::vector<Vector3>& prevPositions = lastVertexPoseLookup[hash]->mVertexPositions;
					
					// Create a vertex pose to hold this new data and insert it into the vertex pose chain.
                    VertexAnimationVertexPose* vertexPose = new VertexAnimationVertexPose();
                    vertexPose->mFrameNumber = i;
                    if(i == 0)
                    {
						mVertexPoses[meshIndex][submeshIndex] = vertexPose;
                        lastVertexPoseLookup[hash] = vertexPose;
                    }
                    else
                    {
                        lastVertexPoseLookup[hash]->mNext = vertexPose;
                        lastVertexPoseLookup[hash] = vertexPose;
                    }
					
                    // 2 bytes: Vertex count.
                    unsigned short vertexCount = reader.ReadUShort();
                    #ifdef DEBUG_OUTPUT
                    std::cout << "        Vertex Count: " << vertexCount << std::endl;
                    #endif
                    
                    // Next ((VertexCount/4) + 1) bytes: Compression info for vertex data.
                    // Every 2 bits indicates how the vertex at that index is compressed.
                    unsigned short compressionInfoSize = (vertexCount / 4) + 1;
                    unsigned char* compressionInfo = new unsigned char[compressionInfoSize];
                    reader.Read(compressionInfo, compressionInfoSize);
                    
                    // Iterate through the compression info and, for each vertex, determine
                    // how it is stored (uncompressed, compressed, not present at all)
                    unsigned char* vertexDataFormat = new unsigned char[vertexCount];
                    for(int k = 0; k < vertexCount; k += 4)
                    {
                        unsigned char byte = compressionInfo[k / 4];
                        unsigned int val1 = (byte & 0x3);
                        unsigned int val2 = (byte & 0xC) >> 2;
                        unsigned int val3 = (byte & 0x30) >> 4;
                        unsigned int val4 = (byte & 0xC0) >> 6;

                        vertexDataFormat[k] = val1;        // Masking 0000 0011
                        if(k + 1 < vertexCount)
                        {
                            vertexDataFormat[k + 1] = val2;    // Masking 0000 1100
                        }
                        if(k + 2 < vertexCount)
                        {
                            vertexDataFormat[k + 2] = val3;    // Masking 0011 0000
                        }
                        if(k + 3 < vertexCount)
                        {
                            vertexDataFormat[k + 3] = val4;    // Masking 1100 0000
                        }
                    }
                    
                    // Now that we have deciphered how each vertex is compressed, we can read in each vertex.
                    for(int k = 0; k < vertexCount; k++)
                    {
						// 0 means no vertex data, so just use whatever we had for the previous frame.
						// If the vertex data hasn't changed since last frame, it isn't stored, to save space.
                        if(vertexDataFormat[k] == 0)
                        {
                            vertexPose->mVertexPositions.push_back(prevPositions[k]);
                        }
                        // 1 means (X, Y, Z) are compressed in next 3 bytes.
						// This tends to be used for storing vertex position delta for internal vertices in a mesh.
                        else if(vertexDataFormat[k] == 1)
                        {
                            float x = DecompressFloatFromByte(reader.ReadByte());
							float z = DecompressFloatFromByte(reader.ReadByte());
                            float y = DecompressFloatFromByte(reader.ReadByte());
                            vertexPose->mVertexPositions.push_back(prevPositions[k] + Vector3(x, y, z));
                        }
                        // 2 means (X, Y, Z) are compressed in next 3 ushorts.
						// This tends to be used for storing vertex position deltas where meshes meet (like a knee or elbow).
                        else if(vertexDataFormat[k] == 2)
                        {
                            float x = DecompressFloatFromUShort(reader.ReadUShort());
							float z = DecompressFloatFromUShort(reader.ReadUShort());
							float y = DecompressFloatFromUShort(reader.ReadUShort());
							vertexPose->mVertexPositions.push_back(prevPositions[k] + Vector3(x, y, z));
                        }
                        // 3 means (X, Y, Z) are not compressed - just floats.
                        else if(vertexDataFormat[k] == 3)
                        {
                            float x = reader.ReadFloat();
							float z = reader.ReadFloat();
                            float y = reader.ReadFloat();
                            vertexPose->mVertexPositions.push_back(prevPositions[k] + Vector3(x, y, z));
                        }
                    }
                    
                    // Don't need these anymore!
                    delete[] compressionInfo;
                    delete[] vertexDataFormat;
                }
                // Identifier 2 is transform matrix data.
                else if(dataId == 2)
                {
                    #ifdef DEBUG_OUTPUT
                    std::cout << "      Matrix Data Block" << std::endl;
                    #endif
                    
                    // 4 bytes: Number of bytes in this block.
                    // Should always be 48 bytes, for 12 floats, which is a 4x3 matrix of data.
                    unsigned int blockByteCount = reader.ReadUInt();
                    assert(blockByteCount == 48);
                    byteCount -= blockByteCount + 4;
                    
                    // These are in i,k,j order, rather than i,j,k, likely due to 3DS Max conventions.
                    // 12 bytes: mesh's x-axis basis vector (i)
                    // 12 bytes: mesh's z-axis basis vector (k)
                    // 12 bytes: mesh's y-axis basis vector (j)
                    Vector3 iBasis = reader.ReadVector3();
                    Vector3 kBasis = reader.ReadVector3();
                    Vector3 jBasis = reader.ReadVector3();
					
					// We can derive an import scale factor from the i/j/k basis by taking the length.
					Vector3 scale(iBasis.GetLength(), jBasis.GetLength(), kBasis.GetLength());
				
					// If the imported model IS scaled, we need to normalize our bases before generating a rotation.
					// Otherwise, the rotation will be off/incorrect.
					iBasis.Normalize();
					jBasis.Normalize();
					kBasis.Normalize();
                    
                    // From the basis vectors, calculate a quaternion representing
                    // a rotation from the standard basis to that basis. We also need to negate some elements
                    // to represent "reflection" from a right-handed rotation to a left-handed rotation.
                    Quaternion rotQuat = Quaternion(Matrix3::MakeBasis(iBasis, jBasis, kBasis));
                    #ifdef DEBUG_OUTPUT
                    std::cout << "        Mesh Rotation: " << rotQuat << std::endl;
                    #endif
                    
                    // 12 bytes: an (X, Y, Z) offset or position for placing this mesh.
                    // Each mesh within the model has it's local offset from the model origin.
                    // This if vital, for example, if a mesh contains a human's head, legs, arms...
                    // want to position them all correctly relative to one another!
                    float x = reader.ReadFloat();
                    float y = reader.ReadFloat();
					float z = reader.ReadFloat();
                    Vector3 meshPos(x, y, z);
                    #ifdef DEBUG_OUTPUT
                    std::cout << "        Mesh Position: " << meshPos << std::endl;
                    #endif
                    
                    VertexAnimationTransformPose* transformPose = new VertexAnimationTransformPose();
                    transformPose->mFrameNumber = i;
                    transformPose->mLocalPosition = meshPos;
                    transformPose->mLocalRotation = rotQuat;
					transformPose->mLocalScale = scale;
                    if(i == 0)
                    {
                        mTransformPoses.push_back(transformPose);
                        lastTransformPoseLookup[meshIndex] = transformPose;
                    }
                    else
                    {
                        lastTransformPoseLookup[meshIndex]->mNext = transformPose;
                        lastTransformPoseLookup[meshIndex] = transformPose;
                    }
                }
                // Identifier 3 is min/max data.
                else if(dataId == 3)
                {
                    #ifdef DEBUG_OUTPUT
                    std::cout << "      Min/Max Data Block" << std::endl;
                    #endif
                    
                    // 4 bytes: Number of bytes in this block.
                    // Should always be 24 bytes, for 6 floats, which is two Vector3.
                    unsigned int blockByteCount = reader.ReadUInt();
                    assert(blockByteCount == 24);
                    byteCount -= blockByteCount + 4;
                    
                    // Assign min/max data.
					#ifdef DEBUG_OUTPUT
					Vector3 min = reader.ReadVector3();
                    Vector3 max = reader.ReadVector3();
					std::cout << "        Min: " << min << std::endl;
					std::cout << "        Max: " << max << std::endl;
					#else
					reader.ReadVector3();
					reader.ReadVector3();
					#endif
                }
                else
                {
                    std::cout << "Unexpected identifier " << (int)dataId << std::endl;
                }
            } // while(byteCount > 0)
        } // iterate mesh groups
    } // iterate keyframes
}

float VertexAnimation::DecompressFloatFromByte(unsigned char val)
{
	// Sign flag is 1 bit - masked by 1000 0000.
    unsigned int signFlag = (val & 0x80);
    float sign = (signFlag == 0) ? 1.0f : -1.0f;
    
	// Whole portion is 2 bits - masked by 0111 1111, then shift 5.
	float whole = (val & 0x7F) >> 5;
	
	// Fractional portion is 5 bits - masked by 0001 1111.
    float frac = (float)(val & 0x1F) / 32.0f;
    return sign * (whole + frac);
}

float VertexAnimation::DecompressFloatFromUShort(unsigned short val)
{
	// Sign flag is 1 bit - masked by 1000 0000 0000 0000.
    unsigned int signFlag = (val & 0x8000);
    float sign = (signFlag == 0) ? 1.0f : -1.0f;
    
	// Whole portion is 7 bits - masked by 0111 1111 1111 1111, then shift 8.
	float whole = (val & 0x7FFF) >> 8;
	
	// Fractional portion is 8 bits - masked by 0000 0000 1111 1111.
    float frac = (float)(val & 0x00FF) / 256.0f;
    return sign * (whole + frac);
}
