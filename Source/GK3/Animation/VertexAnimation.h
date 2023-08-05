//
// Clark Kromenaker
//
// A vertex animation for a particular model/mesh. For GK3, this is the in-memory representation of "ACT" file.
//
// For an animation to work, its vertex data must align with that of a particular model/mesh.
// This includes mesh count, submesh count, vertex count within submeshes, etc.
//
#pragma once
#include "Asset.h"

#include <vector>
#include <unordered_map>

#include "Matrix4.h"
#include "Vector3.h"

struct VertexAnimationPose
{
    int frameNumber = 0;
    VertexAnimationPose* next = nullptr;

    virtual ~VertexAnimationPose() { } 

    VertexAnimationPose* GetForFrame(int frame);
    void GetForTime(float time, int framesPerSecond, VertexAnimationPose*& outCurrent, VertexAnimationPose*& outNext, float& outT);
};

struct VertexAnimationVertexPose : public VertexAnimationPose
{   
    std::vector<Vector3> vertexPositions;
};

struct VertexAnimationTransformPose : public VertexAnimationPose
{
    Matrix4 meshToLocalMatrix;
};

class VertexAnimation : public Asset
{
public:
    VertexAnimation(const std::string& name, AssetScope scope) : Asset(name, scope) { }
    ~VertexAnimation();

    void Load(uint8_t* data, uint32_t dataLength);

    // Queries transform (position, rotation, scale) for a mesh at a frame/time.
    VertexAnimationTransformPose SampleTransformPose(int frame, int meshIndex);
    VertexAnimationTransformPose SampleTransformPose(float time, int framesPerSecond, int meshIndex);
    
	// Queries ALL vertices for a submesh at a frame/time.
    VertexAnimationVertexPose SampleVertexPose(int frame, int meshIndex, int submeshIndex);
    VertexAnimationVertexPose SampleVertexPose(float time, int framesPerSecond, int meshIndex, int submeshIndex);
	
    // Queries single vertex for a submesh at a frame/time.
    Vector3 SampleVertexPosition(int frame, int meshIndex, int submeshIndex, int vertexIndex);
    Vector3 SampleVertexPosition(float time, int framesPerSecond, int meshIndex, int submeshIndex, int vertexIndex);
    
	// Length and duration.
	int GetFrameCount() const { return mFrameCount; }
    float GetDuration(int framesPerSecond) const { return (float)mFrameCount / (float)framesPerSecond; }
	
	const std::string& GetModelName() const { return mModelName; }
	
private:
    // The number of frames in this animation.
    int mFrameCount = 0;
	
	// The name of the model that is meant to play this animation.
	// If we ever play the animation on a mismatched model, the graphics will probably glitch out.
	std::string mModelName;
    
	// Each array element is the FIRST vertex pose for each mesh/submesh index.
	// Subsequent poses for the mesh are stored in the "next" of the first pose.
	std::unordered_map<int, std::unordered_map<int, VertexAnimationVertexPose*>> mVertexPoses;
	
	// Each element of array is the FIRST transform poses for each mesh index.
	// Subsequent poses for the mesh are stored in the "next" of the first pose.
    std::vector<VertexAnimationTransformPose*> mTransformPoses;
    
    void ParseFromData(uint8_t* data, uint32_t dataLength);
    
    float DecompressFloatFromByte(unsigned char val);
    float DecompressFloatFromUShort(unsigned short val);
};
