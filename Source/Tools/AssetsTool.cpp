#include "AssetsTool.h"

#include <imgui.h>

#include "Asset.h"
#include "AssetManager.h"
#include "InspectorUtil.h"
#include "Texture.h"

void AssetsTool::Render(bool& toolActive)
{
    if(!toolActive) { return; }

    // Sets the default size of the hierarchy window on first open.
    ImGui::SetNextWindowSize(ImVec2(430, 450), ImGuiCond_FirstUseEver);

    // Begin the hierarchy window. Early out if collapsed.
    if(!ImGui::Begin("Assets", &toolActive))
    {
        ImGui::End();
        return;
    }

    // Adds some extra padding around the edges.
    ImGui::PushStyleVar(ImGuiStyleVar_FramePadding, ImVec2(2, 2));

    // Figure out half size of window, since we will put assets on left and properties on right (for now).
    ImGuiStyle& style = ImGui::GetStyle();
    float panelWidth = (ImGui::GetContentRegionAvail().x - 4 * style.ItemSpacing.x) / 2;
    if(panelWidth < 1.0f)
    {
        panelWidth = 1.0f;
    }

    // Left panel: assets view.
    ImGui::BeginGroup();
    {
        const bool assetsAvailable = ImGui::BeginChild(ImGui::GetID("Hierarchy"), ImVec2(panelWidth, ImGui::GetContentRegionAvail().y), true, 0);
        if(assetsAvailable)
        {
            // Use asset type as an ID.
            ImGui::PushID("Textures");

            // For all nodes, only expand the tree if you click on the arrow.
            ImGuiTreeNodeFlags assetTypeFlags = ImGuiTreeNodeFlags_OpenOnArrow;

            // Draw the tree node.
            bool node_open = ImGui::TreeNodeEx("Textures", assetTypeFlags, "Textures");

            // If open, draw all the loaded assets of this type.
            if(node_open)
            {
                for(auto& entry : gAssetManager.GetLoadedTextures())
                {
                    ImGui::PushID(entry.second);

                    ImGuiTreeNodeFlags assetFlags = ImGuiTreeNodeFlags_Leaf | ImGuiTreeNodeFlags_Bullet;

                    // If selected, draw as selected.
                    if(mSelectedAsset == entry.second)
                    {
                        assetFlags |= ImGuiTreeNodeFlags_Selected;
                    }

                    // Draw the tree node.
                    bool assetNodeOpen = ImGui::TreeNodeEx("Object", assetFlags, "%s", entry.second->GetName().c_str());

                    // If this item is clicked (and not being toggled open with arrow), set it to selected.
                    if(ImGui::IsItemClicked() && !ImGui::IsItemToggledOpen())
                    {
                        mSelectedAsset = entry.second;
                    }

                    if(assetNodeOpen)
                    {
                        ImGui::TreePop();
                    }

                    // No longer drawing GUI for this asset.
                    ImGui::PopID();
                }
                ImGui::TreePop();
            }

            // No longer working on this asset type.
            ImGui::PopID();
        }
        ImGui::EndChild();
    }
    ImGui::EndGroup();

    // Move to right side of screen.
    ImGui::SameLine();

    // Right panel: selected asset's properties.
    ImGui::BeginGroup();
    {
        const bool propertiesVisible = ImGui::BeginChild(ImGui::GetID("Properties"), ImVec2(panelWidth, ImGui::GetContentRegionAvail().y), true, 0);
        if(propertiesVisible)
        {
            if(mSelectedAsset != nullptr)
            {
                // Draw name of selected Actor as a header.
                ImGui::TextColored(ImVec4(1.0f, 1.0f, 1.0f, 1.0f), "%s", mSelectedAsset->GetName().c_str());
                ImGui::Separator();

                // Display variables of the Actor itself.
                InspectorUtil::RenderVariables(&mSelectedAsset->GetTypeInfo(), mSelectedAsset);
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
