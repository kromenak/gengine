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
	
	// Determine what position is relative to - this will be based on the anchor.
	//Vector2 origin;
	//origin.SetX()
	
	Vector3 pos = mOwner->GetPosition();
	Matrix4 transMat = Matrix4::MakeTranslate(pos);
	
	return transMat * scaleMat;
}
