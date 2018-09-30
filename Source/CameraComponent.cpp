//
// CameraComponent.cpp
//
// Clark Kromenaker
//
#include "CameraComponent.h"

#include "Actor.h"
#include "Services.h"

TYPE_DEF_CHILD(Component, CameraComponent);

CameraComponent::CameraComponent(Actor* owner) : Component(owner)
{
    Services::GetRenderer()->SetCamera(this);
}

Matrix4 CameraComponent::GetLookAtMatrix()
{
    Vector3 eye = mOwner->GetPosition();
    Vector3 lookAt = mOwner->GetPosition() + mOwner->GetForward() * 5.0f;
    Vector3 up = Vector3::UnitY;
    return Matrix4::MakeLookAt(eye, lookAt, up);
}

Matrix4 CameraComponent::GetLookAtMatrixNoTranslate()
{
    Vector3 lookAt = mOwner->GetForward() * 5.0f;
    Vector3 up = Vector3::UnitY;
	return Matrix4::MakeLookAt(Vector3::Zero, lookAt, up);
}

Matrix4 CameraComponent::GetProjectionMatrix()
{
    return Matrix4::MakePerspective(mFovAngleRad, 1.333f, mNearClipPlane, mFarClipPlane);
}

Vector3 CameraComponent::ScreenToWorldPoint(const Vector2& screenPoint, float distance)
{
    float screenWidth = Services::GetRenderer()->GetWindowWidth();
    float screenHeight = Services::GetRenderer()->GetWindowHeight();
    
    Matrix4 viewMatrix = GetLookAtMatrix();
    Matrix4 projectionMatrix = GetProjectionMatrix();
    
    Matrix4 worldToProjection = projectionMatrix * viewMatrix;
    Matrix4 projectionToWorld = worldToProjection.Inverse();
    
    float ndcX = (2.0f * (screenPoint.GetX() / screenWidth)) - 1.0f;
    float ndcY = -(2.0f * (screenPoint.GetY() / screenHeight)) + 1.0f;
    
    Vector4 point(ndcX, ndcY, distance, 1.0f);
    point = projectionToWorld * point;
    point /= point.GetW();
    
    return Vector3(point.GetX(), point.GetY(), point.GetZ());
}
