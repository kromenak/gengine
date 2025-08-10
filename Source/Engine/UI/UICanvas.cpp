#include "UICanvas.h"

#include "Actor.h"
#include "GAPI.h"
#include "GKPrefs.h"
#include "InputManager.h"
#include "Rect.h"
#include "UIWidget.h"
#include "Window.h"

std::vector<UICanvas*> UICanvas::sCanvases;
UIWidget* UICanvas::sMouseOverWidget = nullptr;

/*static*/ void UICanvas::UpdateMouseInput()
{
    // Iterate canvases back-to-front. Since canvases at end of list are rendered last,
    // they should be the first to receive input events.
    for(int i = static_cast<int>(sCanvases.size() - 1); i >= 0; --i)
    {
        // Ignore canvases that aren't active & enabled.
        UICanvas* canvas = sCanvases[i];
        if(!canvas->IsActiveAndEnabled()) { continue; }

        // If this canvas is masked, the pointer must be within the canvas rect.
        // If the pointer is not within the rect, then anything its over in this canvas isn't visible, so we should ignore it.
        if(canvas->IsMasked() && !canvas->GetRectTransform()->GetWorldRect().Contains(gInputManager.GetMousePosition()))
        {
            continue;
        }

        // Same thing for widgets on each canvas - iterate back-to-front.
        // Same reason as above: last widgets render on top of everything, so they should receive input events first.
        for(int j = static_cast<int>(canvas->mWidgets.size() - 1); j >= 0; --j)
        {
            // Ignore widgets that aren't active and enabled.
            UIWidget* widget = canvas->mWidgets[j];
            if(!widget->IsActiveAndEnabled()) { continue; }

            // Widgets that are not recieving inputs should be ignored.
            if(!widget->ReceivesInput()) { continue; }

            // See whether the pointer is over this widget.
            RectTransform* widgetRT = widget->GetRectTransform();
            if(widgetRT->GetWorldRect().Contains(gInputManager.GetMousePosition()))
            {
                // Pointer is over this widget, but it is not the "over" widget yet!
                if(sMouseOverWidget != widget)
                {
                    // Any previous "over" widget no longer has the pointer over it.
                    if(sMouseOverWidget != nullptr)
                    {
                        sMouseOverWidget->OnPointerExit();
                    }

                    // All hail the new widget with the pointer over it.
                    sMouseOverWidget = widget;
                    sMouseOverWidget->OnPointerEnter();
                }

                // If pointer goes down or up this frame, notify whatever widget the pointer is currently over.
                if(sMouseOverWidget != nullptr)
                {
                    if(gInputManager.IsMouseButtonLeadingEdge(InputManager::MouseButton::Left))
                    {
                        sMouseOverWidget->OnPointerDown();
                    }
                    else if(gInputManager.IsMouseButtonTrailingEdge(InputManager::MouseButton::Left))
                    {
                        sMouseOverWidget->OnPointerUp();
                    }
                }

                // Inputs on further widgets or canvases are not possible, since this widget "ate" the input.
                //TODO: We could add some sort of "ignores input events" flag for certain widgets, if needed.
                return;
            }
        }
    }

    // If we get here, the pointer is not over ANY widget.
    // So, if we had a saved widget, the pointer is clearly no longer over it either...
    if(sMouseOverWidget != nullptr)
    {
        sMouseOverWidget->OnPointerExit();
        sMouseOverWidget = nullptr;
    }
}

/*static*/ void UICanvas::RenderCanvases()
{
    // For debugging purposes, it can be handy to sort the canvases each frame, in case we change the draw order via in-editor tools.
    #if defined(DEBUG)
    std::sort(sCanvases.begin(), sCanvases.end(), [](UICanvas* a, UICanvas* b){
        return a->mDrawOrder < b->mDrawOrder;
    });
    #endif

    for(UICanvas* canvas : sCanvases)
    {
        canvas->Render();
    }
}

/*static*/ void UICanvas::NotifyWidgetDestruct(UIWidget* widget)
{
    if(sMouseOverWidget == widget)
    {
        sMouseOverWidget = nullptr;
    }
}

TYPEINFO_INIT(UICanvas, Component, 14)
{
    TYPEINFO_VAR(UICanvas, VariableType::Int, mDrawOrder);
    TYPEINFO_VAR(UICanvas, VariableType::Bool, mAutoScale);
    TYPEINFO_VAR(UICanvas, VariableType::Int, mAutoScaleBias);
}

UICanvas::UICanvas(Actor* owner) : Component(owner)
{
    // Get RectTransform for this canvas.
    mRectTransform = GetOwner()->GetComponent<RectTransform>();
    assert(mRectTransform != nullptr);

    // Set scale correctly on creation.
    RefreshScale();

    // Add to list of canvases.
    sCanvases.push_back(this);
}

UICanvas::UICanvas(Actor* owner, int order) : Component(owner),
    mDrawOrder(order)
{
    // Get RectTransform for this canvas.
    mRectTransform = GetOwner()->GetComponent<RectTransform>();
    assert(mRectTransform != nullptr);

    // Set scale correctly on creation.
    //TODO: This assumes that the owner is parented correctly before UICanvas component is added.
    //TODO: We may want to be notified if transform parent changes?
    RefreshScale();

    // Attempt to find an ideal spot to slot in this canvas, based on desired draw order.
    for(int i = 0; i < sCanvases.size(); ++i)
    {
        if(mDrawOrder <= sCanvases[i]->mDrawOrder)
        {
            sCanvases.insert(sCanvases.begin() + i, this);
            return;
        }
    }

    // If we get here, the draw order is higher than everything in the canvases list.
    // So...just shove it on back!
    sCanvases.push_back(this);
}

UICanvas::~UICanvas()
{
    auto it = std::find(sCanvases.begin(), sCanvases.end(), this);
    if(it != sCanvases.end())
    {
        sCanvases.erase(it);
    }
}

void UICanvas::Render()
{
    if(IsActiveAndEnabled())
    {
        // If masked, set a scissor rect on our world rect.
        // This means that anything outside of our world rect doesn't render.
        if(mMasked)
        {
            GAPI::Get()->SetScissorRect(true, mRectTransform->GetWorldRect());
        }

        // Render all our widgets.
        for(auto& widget : mWidgets)
        {
            if(widget->IsActiveAndEnabled())
            {
                widget->Render();
            }
        }

        // Unset mask if we are using one.
        if(mMasked)
        {
            GAPI::Get()->SetScissorRect(false, Rect());
        }
    }
}

void UICanvas::AddWidget(UIWidget* widget)
{
    mWidgets.push_back(widget);
}

void UICanvas::RemoveWidget(UIWidget* widget)
{
    // Find the widget and erase it from our widget list.
    auto it = std::find(mWidgets.begin(), mWidgets.end(), widget);
    if(it != mWidgets.end())
    {
        mWidgets.erase(it);
    }

    // If the widget being removed is the mouse over widget, treat this as the pointer exiting the widget.
    if(sMouseOverWidget != nullptr && sMouseOverWidget == widget)
    {
        sMouseOverWidget->OnPointerExit();
        sMouseOverWidget = nullptr;

        // We also need to immediately UpdateInput so the sMouseOverWidget updates to whatever else might be under the mouse at this moment.
        // If we don't do this, there's a chance a scene item can be clicked when a UI widget was supposed to block it.
        UICanvas::UpdateMouseInput();
    }
}

float UICanvas::GetScaleFactor() const
{
    // Only autoscale if enabled for this canvas AND enabled globally.
    float scaleFactor = 1.0f;
    if(mAutoScale && Prefs::ScaleUIAtHighResolutions())
    {
        // Also only autoscale if above the minimum window height set globally.
        if(Window::GetHeight() >= Prefs::GetMinimumScaleUIHeight())
        {
            // Calculate how much to scale up the canvas based on the resolution.
            // GK3 UI was authored at 640x480 resolution - that's the lowest supported playable window size.
            // The amount to scale up is how much taller our resolution is than 480 pixels.
            const float kReferenceHeight = 480.0f;
            scaleFactor = Math::Max(Window::GetHeight() / kReferenceHeight, 1.0f);

            // To avoid artifacts from rendering UI images/glyphs across pixel boundaries, we only want integer scale factors.
            // This can be a bit limiting, but I haven't found another way to avoid artifacting yet.
            if(Prefs::UsePixelPerfectUIScaling())
            {
                scaleFactor = Math::Floor(scaleFactor);
            }

            // Add any additional offset that was manually specified for this canvas.
            scaleFactor = Math::Max(scaleFactor + mAutoScaleBias, 1.0f);
        }
    }
    return scaleFactor;
}

void UICanvas::OnEnable()
{
    RefreshScale();
}

void UICanvas::OnUpdate(float deltaTime)
{
    // When active/enabled, refresh scale every frame to ensure it stays at the correct scale even if resolution changes.
    //TODO: Perhaps a more efficient option would be to only do this if the window resolution changes (event-based approach).
    RefreshScale();
}

void UICanvas::RefreshScale()
{
    // Scaling behavior only applies for root-level canvases.
    // Canvases that are children of other transforms (usually for masking behavior) are assumed to be manually sized as desired for now.
    if(GetOwner()->GetParent() == nullptr)
    {
        uint32_t windowWidth = Window::GetWidth();
        uint32_t windowHeight = Window::GetHeight();
        if(mAutoScale)
        {
            // Anchor must be at bottom left in this mode, for simplicity.
            // This aligns with how the UI world coordinate system has origin in bottom left.
            mRectTransform->SetAnchor(AnchorPreset::BottomLeft);
        }

        // Calculate scale factor.
        float scaleFactor = GetScaleFactor();

        // Only update transform properties of scale factor or window height has changed. This avoids dirtying transforms every frame.
        if(!Math::AreEqual(mLastScaleFactor, scaleFactor) || mLastWindowHeight != windowHeight || mLastWindowWidth != windowWidth)
        {
            mLastScaleFactor = scaleFactor;
            mLastWindowWidth = windowWidth;
            mLastWindowHeight = windowHeight;

            // Now here's the magic: we scale UP the rect transform by some factor...
            mRectTransform->SetScale(Vector3(scaleFactor, scaleFactor, 1.0f));

            // ...but at the same time, scale DOWN the width/height of this transform.
            Vector2 sizeDelta = Window::GetSize() / scaleFactor;
            mRectTransform->SetSizeDelta(sizeDelta);
        }
    }
}
