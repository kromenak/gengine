//
// LayerManager.cpp
//
// Clark Kromenaker
//
#include "LayerManager.h"

#include <sstream>

#include "Services.h"
#include "StringUtil.h"

Layer::Layer(const std::string& name) :
    mName(name)
{
    
}

void Layer::Pushed()
{
    // Save audio state of layer below us in the stack.
    mAudioSaveState = Services::GetAudio()->SaveAudioState(!mPersistAmbientState);
}

void Layer::Popped()
{
    // Restore audio state of layer below us in the stack.
    Services::GetAudio()->RestoreAudioState(mAudioSaveState);
}

void Layer::Enter(Layer* fromLayer)
{
    std::cout << "Enter " << mName << std::endl; 
    OnEnter(fromLayer);
}

void Layer::Exit(Layer* toLayer)
{
    std::cout << "Exit " << mName << std::endl;
    OnExit(toLayer);
}

TYPE_DEF_BASE(LayerManager);

LayerManager::LayerManager()
{
    
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
    // Can't pop an empty stack.
    if(mLayerStack.size() <= 0)
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

void LayerManager::RemoveLayer(Layer* layer)
{
    auto it = std::find(mLayerStack.begin(), mLayerStack.end(), layer);
    if(it != mLayerStack.end())
    {
        //TODO: This is sort of a "non-standard" operation that leaves the Layer stack in a weird state.
        //TODO: Maybe we shouldn't even allow this behavior.
        //TODO: Unclear whether removing in middle counts as exiting...and what about saved ambient state!?
        //TODO: Fix this...get rid of it...something!
        //(*it)->Exit(nullptr, true);
        mLayerStack.erase(it);
    }
}

bool LayerManager::IsTopLayer(const Layer* layer) const
{
    return mLayerStack.size() > 0 && mLayerStack.back() == layer;
}

bool LayerManager::IsTopLayer(const std::string& name) const
{
    return mLayerStack.size() > 0 && StringUtil::EqualsIgnoreCase(name, mLayerStack.back()->GetName());
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
    Services::GetReports()->Log("Dump", ss.str());
}
