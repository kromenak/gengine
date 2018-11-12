//
// UIWidget.cpp
//
// Clark Kromenaker
//
#include "UIWidget.h"

#include "Actor.h"
#include "Services.h"

TYPE_DEF_CHILD(Component, UIWidget);

UIWidget::UIWidget(Actor* owner) : Component(owner)
{
    Services::GetRenderer()->AddUIWidget(this);
	
	// Get RectTransform attached to this owner.
	mRectTransform = mOwner->GetComponent<RectTransform>();
	if(mRectTransform == nullptr)
	{
		std::cout << "Attempting to attach UI widget to an Actor that doesn't have a RectTransform. This will cause problems!" << std::endl;
	}
}

UIWidget::~UIWidget()
{
    Services::GetRenderer()->RemoveUIWidget(this);
}

Matrix4 UIWidget::GetWorldTransformWithSizeForRendering()
{
	return mRectTransform->GetLocalToWorldMatrix() * Matrix4::MakeScale(mRectTransform->GetSize());
}
