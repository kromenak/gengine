//
// Clark Kromenaker
//
// The manager keeps track of the current game state, as well as a state history in a stack.
//
// Layers can be pushed/popped on/off a stack, with the top layer being the active one.
// Any layer that is not the top layer is paused/suspended and can't be interacted with.
//
// Examples of layers: the 3D scene, title screen, movie player, inventory screen, help overlay, etc.
//
#pragma once
#include <string>
#include <vector>

#include "Layer.h"

class LayerManager
{
public:
    LayerManager();

    // Push/pop from top of stack.
    void PushLayer(Layer* layer);
    void PopLayer(Layer* expectedLayer = nullptr);

    // Query layer state.
    bool IsTopLayer(const Layer* layer) const;
    bool IsTopLayer(const std::string& name) const;
    bool IsLayerInStack(const Layer* layer) const;
    bool IsLayerInStack(const std::string& name) const;

    // Query current state.
    const std::string& GetTopLayerName() const { return mLayerStack.back()->GetName(); }
    const std::string& GetBottomLayerName() const { return mLayerStack.front()->GetName(); }

    // Debug output.
    void DumpLayerStack();

private:
    // Current layer stack, higher indexes are "on top".
    // Active layer is the one on top of the stack.
    std::vector<Layer*> mLayerStack;

    // The "global" layer - always exists at the bottom of the layer stack.
    Layer mGlobalLayer;
};

extern LayerManager gLayerManager;
