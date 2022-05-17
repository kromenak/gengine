#include "SheepAPI_Debug.h"

#include "LayerManager.h"
#include "Services.h"

using namespace std;

shpvoid DumpLayerStack()
{
    Services::Get<LayerManager>()->DumpLayerStack();
    return 0;
}
RegFunc0(DumpLayerStack, void, IMMEDIATE, DEV_FUNC);