//
// Clark Kromenaker
//
// A "layer" in GK3 can be thought of as a game state.
// In conjunction with LayerManager, layers help track the active game state and remember the state history.
//
#pragma once
#include <string>

#include "AudioManager.h"

class Layer
{
public:
    Layer(const std::string& name);
    virtual ~Layer() = default;

    const std::string& GetName() const { return mName; }

    void OverrideAudioState(bool override);
    void OverrideAudioState(bool overrideSFX, bool overrideVO, bool overrideAmbient);

protected:
    virtual void OnEnter(Layer* fromLayer) { }
    virtual void OnExit(Layer* toLayer) { }

private:
    friend class LayerManager; // Let LayerManager access Enter/Exit functions.

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