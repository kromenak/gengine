//
// CameraComponent.cpp
//
// Clark Kromenaker
//
#include "CameraComponent.h"
#include "Services.h"
#include "SDLRenderer.h"

CameraComponent::CameraComponent(Actor* owner) : Component(owner)
{
    Services::GetRenderer()->SetCamera(this);
}

Matrix4 CameraComponent::GetLookAtMatrix()
{
    Vector3 eye = mOwner->GetPosition();
    Vector3 lookAt = mOwner->GetPosition() + mOwner->GetForward() * 5.0f;
    Vector3 up = Vector3::UnitY;
    Matrix4 lookAtMat = MakeLookAt(eye, lookAt, up);
    return lookAtMat;
}

Matrix4 CameraComponent::GetProjectionMatrix()
{
    Matrix4 projMat = Matrix4::MakePerspective(fovAngleRad, 1.333f,
                                               nearClippingPlane,
                                               farClippingPlane);
    return projMat;
}

Vector3 CameraComponent::ScreenToWorldPoint(const Vector2& screenPoint, float distance)
{
    float screenWidth = Services::GetRenderer()->GetWidth();
    float screenHeight = Services::GetRenderer()->GetHeight();
    
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

Matrix4 CameraComponent::MakeLookAt(const Vector3& eye, const Vector3& lookAt, const Vector3& up)
{
    // Generate view space axes. First, the view direction.
    Vector3 viewDir = lookAt - eye;
    viewDir.Normalize();
    
    // We can use "Gram-Schmidt Orthogonalization" to ensure
    // that the up axis passed is orthogonal with our view direction.
    Vector3 viewUp = up - Vector3::Dot(up, viewDir) * viewDir;
    viewUp.Normalize();
    
    // Once we know the view direction and up, we can use cross product
    // to generate side axis. Order also has an effect on our view-space coordinate system.
    Vector3 viewSide = Vector3::Cross(viewDir, viewUp);
    
    // Generate rotation matrix. This is where we make some concrete
    // choices about our view-space coordinate system.
    // +X is right, +Y is up, +Z is forward (left-handed).
    Matrix3 rotate;
    rotate.SetColumns(-viewSide, viewUp, -viewDir);
    
    // This operation calculates wheter this is a left or right-handed system.
    //float result = Vector3::Dot(-viewSide, Vector3::Cross(viewUp, -viewDir));
    //std::cout << result << std::endl;
    
    // It's convenient for us to also calculate our view -> world matrix at the same time.
    // So let's do that while we're at it.
    mViewToWorldMatrix = Matrix4::MakeRotate(rotate);
    mViewToWorldMatrix(0, 3) = eye.GetX();
    mViewToWorldMatrix(1, 3) = eye.GetY();
    mViewToWorldMatrix(2, 3) = eye.GetZ();
    
    // Make a 4x4 matrix based on the 3x3 rotation matrix.
    rotate.Transpose();
    Matrix4 m = Matrix4::MakeRotate(rotate);
    
    // Calculate inverse of eye vector and assign it to 4x4 matrix.
    Vector3 eyeInv = -(rotate * eye);
    m(0, 3) = eyeInv.GetX();
    m(1, 3) = eyeInv.GetY();
    m(2, 3) = eyeInv.GetZ();
    return m;
}
