//
// Clark Kromenaker
//
// GK3 is very data-driven, but it is not (unfortunately) 100% data-driven.
// There are times when Sheepscript calls to the engine via CallSceneFunction. Per "SHEEP ENGINE.DOC":
//
// CallSceneFunction
// This function will send parameter to the current scene, the effects of which (if it does anything) depends on the custom scene code.
//
// This implies to me that CallSceneFunction executes custom written code for the scene.
// This seems to handle stuff too complex for Sheepscript (like rotating objects).
//
#pragma once
#include <functional>
#include <string>

#include "StringUtil.h"

class SceneFunctions
{
public:
    static void Execute(const std::string& functionName);

private:
    static std::string_map_ci<std::function<void()>> sSceneFunctions;
};