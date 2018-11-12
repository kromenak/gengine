//
// RectTransform.cpp
//
// Clark Kromenaker
//
#include "RectTransform.h"

#include "Services.h"

TYPE_DEF_CHILD(Transform, RectTransform);

RectTransform::RectTransform(Actor* owner) : Transform(owner)
{
	
}

Rect RectTransform::GetScreenRect()
{
	// Calculate the rect min/max centered around local origin.
	Vector3 min = -(mSize / 2.0f);
	Vector3 max = (mSize / 2.0f);
	
	// Transform those points based on this transform's parents and scale/rotation/translation.
	min = LocalToWorldPoint(min);
	max = LocalToWorldPoint(max);
	
	// Construct the rect.
	return Rect(min.GetX(), min.GetY(), max.GetX() - min.GetX(), max.GetY() - min.GetY());
	
	/*
	Vector3 position = mLocalPosition;
	if(mParent != nullptr)
	{
		Matrix4 localToWorldMatrix = mParent->GetLocalToWorldMatrix();
		position = localToWorldMatrix * position;
	}
	
	Vector2 scaledSize = Vector2(mSize.GetX() * mLocalScale.GetX(), mSize.GetY() * mLocalScale.GetY());
	
	// Based on anchor, calculate our anchor position on-screen.
	//Vector2 windowSize = Services::GetRenderer()->GetWindowSize();
	//Vector3 anchorPos(windowSize.GetX() * mAnchor.GetX(), windowSize.GetY() * mAnchor.GetY());
	//Vector3 pos = anchorPos + mOwner->GetTransform()->GetPosition();
	return Rect(position.GetX() - (scaledSize.GetX() / 2), position.GetY() - (scaledSize.GetY() / 2), scaledSize.GetX(), scaledSize.GetY());
	*/
}

/*
void RectTransform::UpdateWorldTransform()
{
	// SCALE - scale by the actor's scale and the size of the rect.
	Matrix4 scaleMatrix = Matrix4::MakeScale(mScale);
	
	// ROTATE - needs to take pivot into account.
	//TODO: How does pivot affect this?
	Matrix4 rotateMatrix = Matrix4::MakeRotate(mRotation);
	
	// TRANSLATE - needs to take anchor and pivot into account.
	//TODO: How does pivot affect this?
	// Based on anchor, calculate our anchor position on-screen.
	//Vector2 windowSize = Services::GetRenderer()->GetWindowSize();
	//Vector3 anchorPos(windowSize.GetX() * mAnchorMin.GetX(), windowSize.GetY() * mAnchorMin.GetY());
	
	// Our position will be treated as an offset from the anchor position.
	//Vector3 pos = anchorPos + mPosition;
	//Matrix4 translateMatrix = Matrix4::MakeTranslate(pos);
	Matrix4 translateMatrix = Matrix4::MakeTranslate(mPosition);
	
	// Combine in order (Scale, Rotate, Translate) to generate world transform.
	mWorldTransform = translateMatrix * rotateMatrix * scaleMatrix;
	
	// If I'm a child, multiply parent transform into the mix.
	if(mParent != nullptr)
	{
		mWorldTransform = mParent->GetWorldTransform() * mWorldTransform;
	}
	
	// Tell my children to update their world transforms too.
	UpdateChildrenWorldTransforms();
}
*/
