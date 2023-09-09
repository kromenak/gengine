#include "HierarchyTool.h"

#include <imgui.h>

#include "Actor.h"
#include "Debug.h"
#include "GameCamera.h"
#include "SceneManager.h"

void HierarchyTool::Render(bool& toolActive)
{
    if(!toolActive) { return; }

    // Sets the default size of the hierarchy window on first open.
    ImGui::SetNextWindowSize(ImVec2(430, 450), ImGuiCond_FirstUseEver);

    // Begin the hierarchy window. Early out if collapsed.
    if(!ImGui::Begin("Hierarchy", &toolActive))
    {
        ImGui::End();
        return;
    }

    // Adds some extra padding around the edges.
    ImGui::PushStyleVar(ImGuiStyleVar_FramePadding, ImVec2(2, 2));

    // Figure out half size of window, since we will put hierarchy on left and properties on right (for now).
    ImGuiStyle& style = ImGui::GetStyle();
    float panelWidth = (ImGui::GetContentRegionAvail().x - 4 * style.ItemSpacing.x) / 2;
    if(panelWidth < 1.0f)
    {
        panelWidth = 1.0f;
    }

    // Left panel: hierarchy view.
    ImGui::BeginGroup();
    {
        const bool hierarchyVisible = ImGui::BeginChild(ImGui::GetID("Hierarchy"), ImVec2(panelWidth, ImGui::GetContentRegionAvail().y), true, 0);
        if(hierarchyVisible)
        {
            for(Actor* actor : gSceneManager.GetActors())
            {
                if(actor->GetTransform()->GetParent() == nullptr)
                {
                    AddTreeNodeForActor(actor);
                }
            }
        }
        ImGui::EndChild();
    }
    ImGui::EndGroup();

    // Move to right side of screen.
    ImGui::SameLine();

    // Right panel: selected actor properties.
    ImGui::BeginGroup();
    {
        const bool propertiesVisible = ImGui::BeginChild(ImGui::GetID("Properties"), ImVec2(panelWidth, ImGui::GetContentRegionAvail().y), true, 0);
        if(propertiesVisible)
        {
            if(mSelectedActor != nullptr)
            {
                Debug::DrawAxes(mSelectedActor->GetTransform()->GetLocalToWorldMatrix());

                ImGui::TextColored(ImVec4(1.0f, 1.0f, 1.0f, 1.0f), "%s", mSelectedActor->GetName().c_str());

                Vector3 position = mSelectedActor->GetPosition();
                ImGui::DragFloat3("Position", reinterpret_cast<float*>(&position));
                mSelectedActor->SetPosition(position);
            }
        }
        ImGui::EndChild();
    }
    ImGui::EndGroup();

    // End extra frame padding.
    ImGui::PopStyleVar();

    // End window.
    ImGui::End();
}

void HierarchyTool::AddTreeNodeForActor(Actor* actor)
{
    // Use object uid as identifier. Most commonly you could also use the object pointer as a base ID.
    ImGui::PushID(actor);

    // For all nodes, only expand the tree if you click on the arrow.
    ImGuiTreeNodeFlags flags = ImGuiTreeNodeFlags_OpenOnArrow;

    // If the actor has no children, draw as a leaf node.
    if(actor->GetTransform()->GetChildren().empty())
    {
        flags |= ImGuiTreeNodeFlags_Leaf | ImGuiTreeNodeFlags_Bullet;
    }

    // If selected, draw as selected.
    if(mSelectedActor == actor)
    {
        flags |= ImGuiTreeNodeFlags_Selected;
    }
    
    // Draw the tree node.
    bool node_open = ImGui::TreeNodeEx("Object", flags, "%s", actor->GetName().c_str());

    // If this item is clicked (and not being toggled open with arrow), set it to selected.
    if(ImGui::IsItemClicked() && !ImGui::IsItemToggledOpen())
    {
        mSelectedActor = actor;

        // If this is a double click, also focus on the selected actor.
        if(ImGui::IsMouseDoubleClicked(0))
        {
            GameCamera* camera = gSceneManager.GetScene()->GetCamera();
            camera->SetPosition(Vector3::Lerp(camera->GetWorldPosition(), mSelectedActor->GetWorldPosition(), 0.9f));
        }
    }
    
    // If node is open, draw it's child contents.
    if(node_open)
    {
        for(Transform* child : actor->GetTransform()->GetChildren())
        {
            AddTreeNodeForActor(child->GetOwner());
        }
        ImGui::TreePop();
    }

    // No longer working on this actor ID.
    ImGui::PopID();
}