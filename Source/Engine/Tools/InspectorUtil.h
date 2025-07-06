//
// Clark Kromenaker
//
// Multiple tools need to render arbitrary lists of object properties using general "inspector" behavior.
// For example, the Hierarchy Tool shows a list of Actor properties, the Assets Tool shows a list of Asset properties.
//
// This util has shared code for rendering a list of properties based on a Type and an instance of that type.
//
#pragma once
class GTypeInfo;

namespace InspectorUtil
{
    void RenderVariables(GTypeInfo* typeInfo, void* instance);
}