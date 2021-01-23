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

void Layer::Enter()
{
    std::cout << "Enter " << mName << std::endl; 
    OnEnter();
    
    // Restore audio state (if any) when entering layer.
    Services::GetAudio()->RestoreAudioState(mAudioSaveState);
}

void Layer::Exit(bool popped)
{
    std::cout << "Exit " << mName << std::endl;
    OnExit();
    
    // If possible to pop back to this layer, save audio state for later.
    // A "popped" state will not be returned to, so no need to save state in that case.
    if(!popped)
    {
        mAudioSaveState = Services::GetAudio()->SaveAudioState();
    }
}

TYPE_DEF_BASE(LayerManager);

LayerManager::LayerManager()
{
    
}

void LayerManager::PushLayer(Layer* layer)
{
    // Exit layer on "top" of stack.
    if(mLayerStack.size() > 0)
    {
        mLayerStack.back()->Exit(false);
    }
    
    // Add new layer to top, enter it.
    mLayerStack.push_back(layer);
    mLayerStack.back()->Enter();
}

void LayerManager::PopLayer(Layer* expectedLayer)
{
    if(mLayerStack.size() <= 0)
    {
        std::cout << "Attempting to pop layer, but stack is empty!" << std::endl;
        return;
    }
    
    // If an expected layer is provided, make sure we're popping that one!
    if(expectedLayer != nullptr && !IsTopLayer(expectedLayer))
    {
        std::cout << "Expected to pop " << expectedLayer->GetName() << ", but top layer is actually " << mLayerStack.back()->GetName() << std::endl;
        return;
    }
    
    // Exit layer at top of stack and pop off stack.
    mLayerStack.back()->Exit(true);
    mLayerStack.pop_back();
    
    // New top of stack is now entered.
    if(mLayerStack.size() > 0)
    {
        mLayerStack.back()->Enter();
    }
}

void LayerManager::RemoveLayer(Layer* layer)
{
    auto it = std::find(mLayerStack.begin(), mLayerStack.end(), layer);
    if(it != mLayerStack.end())
    {
        (*it)->Exit(true);
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
