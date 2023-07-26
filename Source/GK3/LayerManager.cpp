#include "LayerManager.h"

#include <sstream>

#include "ReportManager.h"
#include "SheepManager.h"
#include "StringUtil.h"

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

LayerManager gLayerManager;

LayerManager::LayerManager() :
    mGlobalLayer("GlobalLayer")
{
    PushLayer(&mGlobalLayer);
}

void LayerManager::PushLayer(Layer* layer)
{
    // Exit layer on "top" of stack.
    Layer* fromLayer = nullptr;
    if(mLayerStack.size() > 0)
    {
        fromLayer = mLayerStack.back();
        fromLayer->Exit(layer);
    }
    
    // Add new layer to top, enter it.
    mLayerStack.push_back(layer);
    mLayerStack.back()->Pushed();
    mLayerStack.back()->Enter(fromLayer);
}

void LayerManager::PopLayer(Layer* expectedLayer)
{
    // Can't pop an empty stack. We also don't allow popping the bottom/global layer.
    if(mLayerStack.size() <= 1)
    {
        std::cout << "Attempting to pop layer, but stack is empty!" << std::endl;
        return;
    }
    
    // Caller can provide an "expected layer", kind of as a way to assert that they are popping what they think they're popping.
    if(expectedLayer != nullptr && !IsTopLayer(expectedLayer))
    {
        std::cout << "Expected to pop " << expectedLayer->GetName() << ", but top layer is actually " << mLayerStack.back()->GetName() << std::endl;
        return;
    }
    
    // Figure out layer being popped and layer entering.
    Layer* fromLayer = mLayerStack.back();
    Layer* toLayer = mLayerStack.size() > 1 ? mLayerStack[mLayerStack.size() - 2] : nullptr;
    
    // Exit from layer and pop off stack.
    fromLayer->Exit(toLayer);
    mLayerStack.pop_back();
    fromLayer->Popped();
    
    // New top of stack is now entered.
    if(toLayer != nullptr)
    {
        toLayer->Enter(fromLayer);
    }
}

bool LayerManager::IsTopLayer(const Layer* layer) const
{
    return !mLayerStack.empty() && mLayerStack.back() == layer;
}

bool LayerManager::IsTopLayer(const std::string& name) const
{
    return !mLayerStack.empty() && StringUtil::EqualsIgnoreCase(name, mLayerStack.back()->GetName());
}

bool LayerManager::IsLayerInStack(const Layer* layer) const
{
    return std::find(mLayerStack.begin(), mLayerStack.end(), layer) != mLayerStack.end();
}

bool LayerManager::IsLayerInStack(const std::string& name) const
{
    for(auto& layer : mLayerStack)
    {
        if(StringUtil::EqualsIgnoreCase(layer->GetName(), name))
        {
            return true;
        }
    }
    return false;
}

void LayerManager::DumpLayerStack()
{
    std::stringstream ss;
    ss << "Dumping layer info...";
    
    // Output layer stack from top to bottom.
    // Since top is end of vector, we have to iterate backwards.
    for(int i = mLayerStack.size() - 1; i >= 0; --i)
    {
        ss << std::endl;
        ss << StringUtil::Format("Layer %0i (%s): ambient='%s' rect=%s", mLayerStack.size() - 1 - i, mLayerStack[i]->GetName().c_str(), "do not save", "{0, 0, 1024, 768}");
    }
    
    // Log to dump stream.
    gReportManager.Log("Dump", ss.str());
}
