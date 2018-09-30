//
// UIWidget.cpp
//
// Clark Kromenaker
//
#include "UIWidget.h"

#include "Actor.h"
#include "Services.h"

TYPE_DEF_CHILD(Component, UIWidget);

UIWidget::UIWidget(Actor* owner) : Component(owner),
	mSize(1.0f, 1.0f),
	mPivot(0.5f, 0.5f),
	mAnchor(0.5f, 0.5f)
{
    Services::GetRenderer()->AddUIWidget(this);
}

UIWidget::~UIWidget()
{
    Services::GetRenderer()->RemoveUIWidget(this);
}

Matrix4 UIWidget::GetUIWorldTransformMatrix()
{
	// Scale by the actor's scale and the size of the UI widget.
	Vector3 scale = mOwner->GetScale();
	Matrix4 scaleMat = Matrix4::MakeScale(mSize.GetX() * scale.GetX(), mSize.GetY() * scale.GetY(), 1.0f);
	
	// Based on anchor, calculate our anchor position on-screen.
	Vector2 windowSize = Services::GetRenderer()->GetWindowSize();
	Vector3 anchorPos(windowSize.GetX() * mAnchor.GetX(), windowSize.GetY() * mAnchor.GetY());
	
	// Our position will be treated as an offset from the anchor position.
	Vector3 pos = anchorPos + mOwner->GetPosition();
	Matrix4 transMat = Matrix4::MakeTranslate(pos);
	return transMat * scaleMat;
}
