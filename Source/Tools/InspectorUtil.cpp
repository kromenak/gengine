#include "InspectorUtil.h"

#include <imgui.h>
#include <imgui_stdlib.h>

#include "Asset.h"
#include "Quaternion.h"
#include "TypeInfo.h"
#include "Vector3.h"

void InspectorUtil::RenderVariables(GTypeInfo* typeInfo, void* instance)
{
    // Display a collapsing section for the component, open by default.
    if(ImGui::CollapsingHeader(typeInfo->GetTypeName(), ImGuiTreeNodeFlags_DefaultOpen))
    {
        // Since multiple components can have the same variable name (mEnabled), we need to uniquely ID *this* component's variables.
        // This is done by pushing/popping unique IDs in IMGUI.
        ImGui::PushID(instance);

        // Indent the variables within the collapsing section.
        ImGui::Indent();

        // Iterate from subclass to base class, outputting edit fields for each registered variable.
        while(typeInfo != nullptr)
        {
            for(VariableInfo& variableInfo : typeInfo->GetVariables())
            {
                switch(variableInfo.GetType())
                {
                case VariableType::Int:
                    ImGui::InputInt(variableInfo.GetName(), &variableInfo.GetRef<int>(instance));
                    break;

                case VariableType::Bool:
                    ImGui::Checkbox(variableInfo.GetName(), &variableInfo.GetRef<bool>(instance));
                    break;

                case VariableType::Float:
                    ImGui::InputFloat(variableInfo.GetName(), variableInfo.GetPtr<float>(instance));
                    break;

                case VariableType::String:
                    ImGui::InputText(variableInfo.GetName(), &variableInfo.GetRef<std::string>(instance));
                    break;

                case VariableType::Vector2:
                    ImGui::DragFloat2(variableInfo.GetName(), variableInfo.GetPtr<float>(instance));
                    break;

                case VariableType::Vector3:
                    ImGui::DragFloat3(variableInfo.GetName(), variableInfo.GetPtr<float>(instance));
                    break;

                case VariableType::Quaternion:
                {
                    Quaternion& quat = variableInfo.GetRef<Quaternion>(instance);
                    Vector3 eulerAngles = quat.GetEulerAngles();
                    ImGui::DragFloat3(variableInfo.GetName(), &eulerAngles.x);
                    quat.Set(eulerAngles.x, eulerAngles.y, eulerAngles.z);
                    break;
                }

                case VariableType::Asset:
                {
                    Asset** asset = variableInfo.GetPtr<Asset*>(instance);
                    if((*asset) == nullptr)
                    {
                        ImGui::LabelText(variableInfo.GetName(), "null");
                    }
                    else
                    {
                        ImGui::LabelText(variableInfo.GetName(), "%s [%s]", (*asset)->GetName().c_str(), (*asset)->GetTypeName());
                    }
                    break;
                }

                //TODO: Pointer-to-type variables
                //TODO: Callbacks
                //TODO: Assets
                //TODO: Structs

                default:
                    // Just display name, but uneditable.
                    ImGui::TextColored(ImVec4(1.0f, 1.0f, 1.0f, 1.0f), "%s", variableInfo.GetName());
                    break;
                }
            }
            typeInfo = typeInfo->GetBaseType();
        }

        ImGui::Unindent();
        ImGui::PopID();
    }
}