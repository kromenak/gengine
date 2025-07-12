#include "Layer.h"

#include "SheepManager.h"

Layer::Layer(const std::string& name) :
    mName(name)
{

}

void Layer::OverrideAudioState(bool override)
{
    OverrideAudioState(override, override, override);
}

void Layer::OverrideAudioState(bool overrideSFX, bool overrideVO, bool overrideAmbient)
{
    mOverrideSfxAudioState = overrideSFX;
    mOverrideVoAudioState = overrideVO;
    mOverrideAmbientAudioState = overrideAmbient;
}

void Layer::Enter(Layer* fromLayer)
{
    //std::cout << "Enter " << mName << std::endl;
    OnEnter(fromLayer);
}

void Layer::Exit(Layer* toLayer)
{
    //std::cout << "Exit " << mName << std::endl;
    OnExit(toLayer);
}

void Layer::Pushed()
{
    // Save previous layer's audio state if we are overriding anything.
    gAudioManager.SaveAudioState(mOverrideSfxAudioState, mOverrideVoAudioState, mOverrideAmbientAudioState, mAudioSaveState);
}

void Layer::Popped()
{
    // Restore previous layer's audio state on pop.
    gAudioManager.RestoreAudioState(mAudioSaveState);

    // When a layer is popped, all associated executing SheepScripts should stop immediately.
    gSheepManager.StopExecution(mName);
}
