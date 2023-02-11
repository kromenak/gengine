#include "SheepAPI_Debug.h"

#include "Debug.h"
#include "LayerManager.h"
#include "Services.h"

using namespace std;

int GetDebugFlag(const std::string& flagName)
{
    return Debug::GetFlag(flagName);
}
RegFunc1(GetDebugFlag, int, string, IMMEDIATE, DEV_FUNC);

shpvoid SetDebugFlag(const std::string& flagName)
{
    Debug::SetFlag(flagName);
    return 0;
}
RegFunc1(SetDebugFlag, void, string, IMMEDIATE, DEV_FUNC);

shpvoid ClearDebugFlag(const std::string& flagName)
{
    Debug::ClearFlag(flagName);
    return 0;
}
RegFunc1(ClearDebugFlag, void, string, IMMEDIATE, DEV_FUNC);

shpvoid ToggleDebugFlag(const std::string& flagName)
{
    Debug::ToggleFlag(flagName);
    return 0;
}
RegFunc1(ToggleDebugFlag, void, string, IMMEDIATE, DEV_FUNC);

shpvoid DumpDebugFlags()
{
    Debug::DumpFlags();
    return 0;
}
RegFunc0(DumpDebugFlags, void, IMMEDIATE, DEV_FUNC);

shpvoid DumpLayerStack()
{
    Services::Get<LayerManager>()->DumpLayerStack();
    return 0;
}
RegFunc0(DumpLayerStack, void, IMMEDIATE, DEV_FUNC);