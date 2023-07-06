//
// Clark Kromenaker
//
// Represents a virtual camera in the game world. At least one is required to render anything!
// The world will be rendered from the position/orientation of the Actor who owns the Camera component.
//
#pragma once
#include "Component.h"

#include "Frustum.h"
#include "GMath.h"
#include "Matrix4.h"
#include "Vector2.h"
#include "Vector3.h"

class Camera : public Component
{
    TYPE_DECL_CHILD();
public:
    Camera(Actor* owner);
    ~Camera();
    
    Matrix4 GetLookAtMatrix();
    Matrix4 GetLookAtMatrixNoTranslate();
    Matrix4 GetProjectionMatrix();

    Frustum GetWorldSpaceViewFrustum();
    
    Vector3 ScreenToWorldPoint(const Vector2& screenPoint, float distance);
    
	float GetCameraFovRadians() const { return mFovAngleRad; }
	float GetCameraFovDegrees() const { return Math::ToDegrees(mFovAngleRad); }
	
	void SetCameraFovRadians(float fovRad);
    void SetCameraFovDegrees(float fovDeg);

    float GetNearClipPlaneDistance() const { return mNearClipPlane; }
    void SetNearClipPlaneDistance(float distance) { mNearClipPlane = distance; }

    float GetFarClipPlaneDistance() const { return mFarClipPlane; }
    void SetFarClipPlaneDistance(float distance) { mFarClipPlane = distance; }
	
private:
    // Field of view angle, in radians, for perspective projection.
    float mFovAngleRad = 1.0472f;
    
    // Near and far clipping planes, for any projection type.
    // GK3 seems to use 12.0f/24000.0f, but 12.0f doesn't look great in GEngine, so going to use a smaller value.
    float mNearClipPlane = 1.0f;
    float mFarClipPlane = 24000.0f;
};
