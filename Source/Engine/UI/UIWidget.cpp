#include "UIWidget.h"

#include "Actor.h"
#include "UICanvas.h"

TYPEINFO_INIT(UIWidget, Component, 15)
{

}


UIWidget::UIWidget(Actor* owner) : Component(owner)
{
    // Get RectTransform attached to this owner.
    mRectTransform = GetOwner()->GetComponent<RectTransform>();
    assert(mRectTransform != nullptr);

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
    return mRectTransform->GetLocalToWorldMatrix() * mRectTransform->GetRectToLocalMatrix();
}
