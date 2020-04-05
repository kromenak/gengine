//
// Camera.cpp
//
// Clark Kromenaker
//
#include "Camera.h"

#include "Actor.h"
#include "Services.h"

TYPE_DEF_CHILD(Component, Camera);

Camera::Camera(Actor* owner) : Component(owner)
{
    Services::GetRenderer()->SetCamera(this);
}

Matrix4 Camera::GetLookAtMatrix()
{
    Vector3 eye = GetOwner()->GetPosition();
    Vector3 lookAt = eye + GetOwner()->GetForward();
    Vector3 up = Vector3::UnitY;
    return Matrix4::MakeLookAt(eye, lookAt, up);
}

Matrix4 Camera::GetLookAtMatrixNoTranslate()
{
    Vector3 lookAt = GetOwner()->GetForward();
    Vector3 up = Vector3::UnitY;
	return Matrix4::MakeLookAt(Vector3::Zero, lookAt, up);
}

Matrix4 Camera::GetProjectionMatrix()
{
    return Matrix4::MakePerspective(mFovAngleRad, 1.333f, mNearClipPlane, mFarClipPlane);
}

Vector3 Camera::ScreenToWorldPoint(const Vector2& screenPoint, float distance)
{
	// GOAL: convert screen point back to world space.
	
	// The screen point is in screen coordinates. These are (0, 0) in lower-left and (screenWidth, screenHeight) in top-right.
	// This is decided by how we map (x, y) values from the OS to the screen (in InputManager).
	//
	// We need the point to be in normalized device coordinates.
	// Normalized device coordinates vary from (-1, -1) in lower-left and (1, 1) in upper right.
	// This is decided by the ndcX and ndcY equations below.
	//
	//  				|-----------| (screenWidth, screenHeight) (1, 1)
	//  				|           |
	//  				|           |
	//  (0, 0) (-1, -1)	|-----------|
	//
	// Convert screen point to normalized device coordinate point.
	// Note that the mapping of NDCs to top-left/lower-left/etc is dictated by these equations.
	float screenWidth = Services::GetRenderer()->GetWindowWidth();
	float screenHeight = Services::GetRenderer()->GetWindowHeight();
    float ndcX = (2.0f * (screenPoint.GetX() / screenWidth)) - 1.0f; 		// 0 => -1, screenWidth => 1
	float ndcY = (2.0f * (screenPoint.GetY() / screenHeight)) - 1.0f; 		// 0 => -1, screenHeight => 1
    //float ndcY = -(2.0f * (screenPoint.GetY() / screenHeight)) + 1.0f; 	// 0 => 1, screenHeight => -1 (this would put (-1, -1) in top-left corner)
	
	// Our NDC point is X/Y values for starters.
	// Distance indicates what distance from the camera we want to get for the world point.
	// 1.0 is just standard for homogenous coordinates.
	Vector4 point(ndcX, ndcY, distance, 1.0f);
	
	// We now need to convert our normalized device coordinate to world space.
	// To do this, we must calculate the "world space to screen space" matrix, and then invert it.
	Matrix4 viewMatrix = GetLookAtMatrix();
	Matrix4 projectionMatrix = GetProjectionMatrix();
	Matrix4 projectionToWorld = (projectionMatrix * viewMatrix).Inverse();
	
	// Multiply the NDC point by the "screen space to world space" matrix to get to world space.
	// We must divide the whole point by W to deal with reversing perspective depth stuff.
    point = projectionToWorld * point;
    point /= point.GetW();
	
	// Finally, return the world point.
    return Vector3(point.GetX(), point.GetY(), point.GetZ());
}

void Camera::SetCameraFovRadians(float fovRad)
{
	mFovAngleRad = Math::Clamp(fovRad, 0.0f, Math::kPi);
}

void Camera::SetCameraFovDegrees(float fovDeg)
{
	mFovAngleRad = Math::ToRadians(Math::Clamp(fovDeg, 0.0f, 180.0f));
}
