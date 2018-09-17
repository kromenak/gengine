//
// VertexAnimation.h
//
// Clark Kromenaker
//
// A vertex animation for a particular model/mesh.
//
// For an animation to work, its data must align with that of a particular model/mesh.
// This includes mesh count, vertex count within meshes, etc.
//
// For GK3, this is the in-memory representation of "ACT" file.
//
#pragma once
#include "Asset.h"

#include <vector>

#include "AtomicTypes.h"
#include "Matrix4.h"
#include "Vector3.h"

struct VertexAnimationVertexPose
{
    int32 mFrameNumber = 0;
    uint8 mMeshIndex = 0;
    
    std::vector<Vector3> mVertexPositions;
    VertexAnimationVertexPose* mNext = nullptr;
};

struct VertexAnimationTransformPose
{
    int32 mFrameNumber = 0;
    uint8 mMeshIndex = 0;
    
    Quaternion mLocalRotation;
    Vector3 mLocalPosition;
    VertexAnimationTransformPose* mNext = nullptr;
    
    Matrix4 GetLocalTransformMatrix()
    {
        return Matrix4::MakeTranslate(mLocalPosition) * Matrix4::MakeRotate(mLocalRotation);
    }
};

class VertexAnimation : public Asset
{
public:
    VertexAnimation(std::string name, char* data, int dataLength);
    
    VertexAnimationVertexPose SampleVertexPose(float time, int framesPerSecond, int meshIndex);
    VertexAnimationTransformPose SampleTransformPose(float time, int framesPerSecond, int meshIndex);
    
    float GetDuration(int framesPerSecond) { return (1.0f / framesPerSecond) * mFrameCount; }
    
private:
    // The number of frames in this animation.
    int mFrameCount = 0;
    
    // Arrays containing the FIRST vertex/transform poses for each mesh.
    // Subsequent poses are stored in the "next" of these poses.
    std::vector<VertexAnimationVertexPose*> mVertexPoses;
    std::vector<VertexAnimationTransformPose*> mTransformPoses;
    
    void ParseFromData(char* data, int dataLength);
    
    float DecompressFloatFromByte(unsigned char val);
    float DecompressFloatFromUShort(unsigned short val);
};
