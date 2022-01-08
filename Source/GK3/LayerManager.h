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
    friend class LayerManager; // Let LayerManager access Enter/Exit functions.

public:
    Layer(const std::string& name);
    
    const std::string& GetName() const { return mName; }

    void OverrideAudioState(bool override);
    void OverrideAudioState(bool overrideSFX, bool overrideVO, bool overrideAmbient);
    
protected:
    virtual void OnEnter(Layer* fromLayer) { }
    virtual void OnExit(Layer* toLayer) { }
    
private:
    // Name of the layer.
    std::string mName;

    // If any are true, this layer will pause the previous layer's audio when it is pushed onto the layer stack.
    // The previous layer's audio will resume when this layer is popped off the layer stack.
    bool mOverrideSfxAudioState = false;
    bool mOverrideVoAudioState = false;
    bool mOverrideAmbientAudioState = false;

    // Saved audio state.
    AudioSaveState mAudioSaveState;

    void Enter(Layer* fromLayer);
    void Exit(Layer* toLayer);

    void Pushed();
    void Popped();
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
