//
// Clark Kromenaker
//
// A collection of UI components that are logically related and
// updated/drawn/managed together.
//
// A UI "canvas" could represent one distinct UI screen or dialog.
// E.g. a Main Menu, an Options Screen, a Popup, a Tooltip.
//
#pragma once
#include "Component.h"

#include <climits> // INT_MAX
#include <vector>

class RectTransform;
class UIWidget;

class UICanvas : public Component
{
    TYPEINFO_SUB(UICanvas, Component);
public:
    static const std::vector<UICanvas*>& GetCanvases() { return sCanvases; }
    static void UpdateMouseInput();
    static void RenderCanvases();
    static bool DidWidgetEatInput() { return sMouseOverWidget != nullptr; }
    static void NotifyWidgetDestruct(UIWidget* widget);

    UICanvas(Actor* owner);
    UICanvas(Actor* owner, int order);
    ~UICanvas();

    void Render();

    void AddWidget(UIWidget* widget);
    void RemoveWidget(UIWidget* widget);
    void RemoveAllWidgets() { mWidgets.clear(); }

    void SetMasked(bool masked) { mMasked = masked; }
    bool IsMasked() const { return mMasked; }

    void SetAutoScale(bool autoScale) { mAutoScale = autoScale; }
    void SetAutoScaleBias(int32_t bias) { mAutoScaleBias = bias; }
    float GetScaleFactor() const;

    RectTransform* GetRectTransform() const { return mRectTransform; }

protected:
    void OnEnable() override;
    void OnUpdate(float deltaTime) override;

private:
    // An array of all canvases that currently exist.
    static std::vector<UICanvas*> sCanvases;

    // At any time, the mouse can be over exactly one widget.
    // (at least, unless we add multi-pointer support...shudders)
    static UIWidget* sMouseOverWidget;

    // The canvas's rect transform.
    RectTransform* mRectTransform = nullptr;

    // Desired draw order for the canvas. Zero is drawn before one, one is drawn before two, etc.
    int mDrawOrder = INT_MAX;

    // All widgets on this canvas.
    std::vector<UIWidget*> mWidgets;

    // If true, the canvas only renders within its RectTransform borders, masking anything outside of it.
    bool mMasked = false;

    // If true, and this is a root canvas, the canvas will be automatically scaled up at higher resolutions.
    bool mAutoScale = true;

    // When canvas is autoscaled, the scale amount will be offset by this amount.
    // Useful if certain UIs look better at larger/smaller scale, even at a higher resolution.
    int32_t mAutoScaleBias = 0;

    // Track the last calculated scale factor and window height. Just helps to avoid dirtying transforms every frame.
    float mLastScaleFactor = 0.0f;
    uint32_t mLastWindowWidth = 0.0f;
    uint32_t mLastWindowHeight = 0.0f;

    void RefreshScale();
};
