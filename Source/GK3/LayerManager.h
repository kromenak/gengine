//
// LayerManager.h
//
// Clark Kromenaker
//
// A "layer" in GK3 can be thought of as a game state.
// The manager keeps track of the current game state, as well as a state history in a stack.
//
// Layers can be pushed/popped on/off a stack, with the top layer being the active one.
// Any layer that is not the top layer is paused/suspended and can't be interacted with.
//
// Examples of layers: the 3D scene, title screen, movie player, inventory screen, help overlay, etc.
//
#pragma once
#include <iostream>
#include <string>
#include <vector>

#include "AudioManager.h"
#include "Type.h"

// Basic layer class.
// If custom enter/exit functionality is needed, this should be subclassed.
class Layer
{
public:
    Layer(const std::string& name);
    
    void Enter();
    void Exit(bool popped);

    const std::string& GetName() const { return mName; }
    
protected:
    virtual void OnEnter() { }
    virtual void OnExit() { }
    
private:
    // Name of the layer.
    std::string mName;
    
    // Saved audio state.
    AudioSaveState mAudioSaveState;
};

class LayerManager
{
    TYPE_DECL_BASE();
public:
    LayerManager();
    
    // Push/pop from top of stack.
    void PushLayer(Layer* layer);
    void PopLayer(Layer* expectedLayer = nullptr);
    
    // Removes layer, even if not on top of stack.
    void RemoveLayer(Layer* layer);
    
    // Query layer state.
    bool IsTopLayer(const Layer* layer) const;
    bool IsTopLayer(const std::string& name) const;
    
    // Debug output.
    void DumpLayerStack();
    
private:
    // Current layer stack, higher indexes are "on top".
    // Active layer is the one on top of the stack.
    std::vector<Layer*> mLayerStack;
};
