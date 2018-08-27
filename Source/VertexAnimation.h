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
#include "Vector3.h"
#include <vector>
#include "Matrix4.h"

struct VertexAnimationMeshPose
{
    int mMeshIndex = 0;
    
    std::vector<Vector3> mVertexPositions;
    
    Quaternion mLocalRotation;
    Vector3 mLocalPosition;
    
    Vector3 mMin;
    Vector3 mMax;
    
    Matrix4 GetLocalTransformMatrix()
    {
        // Use mesh position offset and rotation values to create a local transform matrix.
        Matrix4 transMatrix = Matrix4::MakeTranslate(mLocalPosition);
        Matrix4 rotMatrix = Matrix4::MakeRotate(mLocalRotation);
        return transMatrix * rotMatrix;
    }
};

struct VertexAnimationFrame
{
    std::vector<VertexAnimationMeshPose*> mMeshPoses;
};

class VertexAnimation : public Asset
{
public:
    VertexAnimation(std::string name, char* data, int dataLength);
    
    VertexAnimationMeshPose TakeSample(float time, int meshIndex);
    
    float GetDuration() { return (1.0f / mFramesPerSecond) * mFrames.size(); }
    
private:
    // The animation consists of one or more frames.
    // Each frame's length is dictated by the frame rate (often 30FPS or 60FPS).
    std::vector<VertexAnimationFrame*> mFrames;
    
    // The number of frames per second.
    // The duration of each frame is then 1/framesPerSecond seconds.
    int mFramesPerSecond = 24;
    
    void ParseFromData(char* data, int dataLength);
    
    float DecompressFloatFromByte(unsigned char byte);
    float DecompressFloatFromUShort(unsigned short ushort);
};
