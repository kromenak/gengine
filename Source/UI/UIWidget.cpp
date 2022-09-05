#include "UIWidget.h"

#include "Actor.h"
#include "Services.h"
#include "UICanvas.h"

TYPE_DEF_CHILD(Component, UIWidget);

UIWidget::UIWidget(Actor* owner) : Component(owner)
{
	// Get RectTransform attached to this owner.
	mRectTransform = GetOwner()->GetComponent<RectTransform>();
	if(mRectTransform == nullptr)
	{
		std::cout << "Attempting to attach UI widget to an Actor that doesn't have a RectTransform. This will cause problems!" << std::endl;
	}

    // Notify nearest canvas (if any) in transform hierarchy that widget was just added.
    UICanvas* canvas = GetOwner()->GetComponentInParents<UICanvas>();
    if(canvas != nullptr)
    {
        canvas->AddWidget(this);
    }
}

UIWidget::~UIWidget()
{
    // This is definitely a HACK - probably can be improved by refactoring how Widgets/Canvases interact.
    UICanvas::NotifyWidgetDestruct(this);
}

Matrix4 UIWidget::GetWorldTransformWithSizeForRendering()
{
	// In addition to normal local to world matrix, first offset by local rect offset to get rect in right spot.
	// And afterwards, apply size to deal with rect that isn't perfect square.
	Matrix4 localRectOffsetMatrix = mRectTransform->GetLocalRectOffset();
	Matrix4 result = localRectOffsetMatrix * mRectTransform->GetLocalToWorldMatrix();
	return result * Matrix4::MakeScale(mRectTransform->GetSize());
}
