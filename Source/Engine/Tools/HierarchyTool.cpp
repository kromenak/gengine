#include "HierarchyTool.h"

#include <imgui.h>
#include <imgui_stdlib.h>

#include "Actor.h"
#include "Debug.h"
#include "GameCamera.h"
#include "InspectorUtil.h"
#include "SceneManager.h"

void HierarchyTool::Render(bool& toolActive)
{
    if(!toolActive) { return; }

    // Sets the default size of the hierarchy window on first open.
    ImGui::SetNextWindowSize(ImVec2(430, 450), ImGuiCond_FirstUseEver);

    // Begin the hierarchy window. Early out if collapsed.
    if(!ImGui::Begin("Scene Hierarchy", &toolActive))
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
            // We need to iterate all Actors to render the scene hierarchy.
            bool encounteredSelectedActor = false;
            for(Actor* actor : gSceneManager.GetActors())
            {
                // Actors with no parent are at the root of the tree.
                if(actor->GetTransform()->GetParent() == nullptr)
                {
                    AddTreeNodeForActor(actor);
                }
            }

            // Here's a problem: this tool remembers the selected actor across frames. But what if that actor is deleted?
            // We need to detect that the selected actor no longer exists and null it out.
            for(Actor* actor : gSceneManager.GetActors())
            {
                if(actor == mSelectedActor)
                {
                    encounteredSelectedActor = true;
                    break;
                }
            }
            if(!encounteredSelectedActor)
            {
                mSelectedActor = nullptr;
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

                // Draw name of selected Actor as a header.
                ImGui::TextColored(ImVec4(1.0f, 1.0f, 1.0f, 1.0f), "%s", mSelectedActor->GetName().c_str());
                ImGui::Separator();

                // Display variables of the Actor itself.
                InspectorUtil::RenderVariables(&mSelectedActor->GetTypeInfo(), mSelectedActor);

                // Display variables of each component.
                for(Component* component : mSelectedActor->GetComponents())
                {
                    InspectorUtil::RenderVariables(&component->GetTypeInfo(), component);
                }
            }
        }
        ImGui::EndChild();
    }
    ImGui::EndGroup();

    // End extra frame padding.
    ImGui::PopStyleVar();

    // End window.
    ImGui::End();

    // This is a bit of a HACK, but if we have a selected actor, set them dirty.
    // This allows modifying transform data in the inspector and seeing the result in real-time.
    if(mSelectedActor != nullptr)
    {
        mSelectedActor->GetTransform()->SetDirty();
    }
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
    bool node_open = ImGui::TreeNodeEx("Object", flags, "%s [%s]", actor->GetName().c_str(), GetBestTypeLabelForActor(actor));

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

const char* HierarchyTool::GetBestTypeLabelForActor(Actor* actor)
{
    // Use the type name of the actor by default.
    const char* name = actor->GetTypeName();

    // If the type name is "Actor", that's not super descriptive!
    // See if there's a better name based on one of the components.
    if(strcmp(name, "Actor") == 0)
    {
        // Try to find a meaningful component name.
        // Transforms aren't too interesting, so skip those.
        for(Component* component : actor->GetComponents())
        {
            const char* componentName = component->GetTypeName();
            if(strcmp(componentName, "Transform") == 0) { continue; }
            if(strcmp(componentName, "RectTransform") == 0) { continue; }
            return componentName;
        }
    }

    // Just use the actor's type name, worst case.
    return name;
}