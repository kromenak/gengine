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
	// Get RectTransform attached to this owner.
	mRectTransform = mOwner->GetComponent<RectTransform>();
	if(mRectTransform == nullptr)
	{
		std::cout << "Attempting to attach UI widget to an Actor that doesn't have a RectTransform. This will cause problems!" << std::endl;
	}
}

UIWidget::~UIWidget()
{
    
}

Matrix4 UIWidget::GetWorldTransformWithSizeForRendering()
{
	// In addition to normal local to world matrix, first offset by local rect offset to get rect in right spot.
	// And afterwards, apply size to deal with rect that isn't perfect square.
	Matrix4 localRectOffsetMatrix = mRectTransform->GetLocalRectOffset();
	Matrix4 result = localRectOffsetMatrix * mRectTransform->GetLocalToWorldMatrix();
	return result * Matrix4::MakeScale(mRectTransform->GetSize());
}
