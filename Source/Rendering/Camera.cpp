#include "Camera.h"

#include "Actor.h"
#include "RenderTransforms.h"
#include "Services.h"
#include "Window.h"

TYPE_DEF_CHILD(Component, Camera);

Camera::Camera(Actor* owner) : Component(owner)
{
    Services::GetRenderer()->SetCamera(this);
}

Camera::~Camera()
{
    if(Services::GetRenderer()->GetCamera() == this)
    {
        Services::GetRenderer()->SetCamera(nullptr);
    }
}

Matrix4 Camera::GetLookAtMatrix()
{
    Vector3 eye = GetOwner()->GetPosition();
    
    // World space axes directions are all a matter of perspective.
    // Here, we say that view forward equals Actor forward (mapped to Z axis).
    // And view up equals Actor up (mapped to Y axis).
    Vector3 lookAt = eye + GetOwner()->GetForward();
    Vector3 up = GetOwner()->GetUp();
    return RenderTransforms::MakeLookAt(eye, lookAt, up);
}

Matrix4 Camera::GetLookAtMatrixNoTranslate()
{
    Vector3 lookAt = GetOwner()->GetForward();
    Vector3 up = GetOwner()->GetUp();
	return RenderTransforms::MakeLookAt(Vector3::Zero, lookAt, up);
}

Matrix4 Camera::GetProjectionMatrix()
{
    Vector2 windowSize = Window::GetSize();
    return RenderTransforms::MakePerspective(mFovAngleRad, windowSize.x / windowSize.y, mNearClipPlane, mFarClipPlane);
}

Frustum Camera::GetWorldSpaceViewFrustum()
{
    return Frustum(GetProjectionMatrix() * GetLookAtMatrix());
}

Vector3 Camera::ScreenToWorldPoint(const Vector2& screenPoint, float distance)
{
    // First, convert point to NDC space.
	float screenWidth = Window::GetWidth();
	float screenHeight = Window::GetHeight();
    Vector4 point = RenderTransforms::ScreenPointToNDCPoint(screenPoint, distance, screenWidth, screenHeight);
    
    // Usually, a point is converted from world to NDC using a "worldToProjection" matrix.
    // To do the reverse, we create that matrix and then invert it.
    Matrix4 viewMatrix = GetLookAtMatrix();
    Matrix4 projectionMatrix = GetProjectionMatrix();
    Matrix4 projectionToWorld = Matrix4::Inverse(projectionMatrix * viewMatrix);
	
    // Convert point to world space.
    point = projectionToWorld * point;
    
    // W value is likely affected by the transformation.
    // So, must divide to get back to a valid point (w == 1).
    point /= point.w;
    
	// Finally, return the world point.
    return Vector3(point.x, point.y, point.z);
}

void Camera::SetCameraFovRadians(float fovRad)
{
	mFovAngleRad = Math::Clamp(fovRad, 0.0f, Math::kPi);
}

void Camera::SetCameraFovDegrees(float fovDeg)
{
	mFovAngleRad = Math::ToRadians(Math::Clamp(fovDeg, 0.0f, 180.0f));
}

