#include "AssetsTool.h"

#include <imgui.h>

#include "Asset.h"
#include "AssetManager.h"
#include "InspectorUtil.h"

#include "Animation.h"
#include "Audio.h"
#include "BSP.h"
#include "BSPLightmap.h"
#include "Config.h"
#include "Cursor.h"
#include "Font.h"
#include "GAS.h"
#include "Model.h"
#include "NVC.h"
#include "SceneAsset.h"
#include "SceneInitFile.h"
#include "Sequence.h"
#include "Shader.h"
#include "SheepScript.h"
#include "Soundtrack.h"
#include "TextAsset.h"
#include "Texture.h"
#include "VertexAnimation.h"

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
            AddAssetList<Animation>("ANM");
            AddAssetList<Audio>();
            AddAssetList<BSP>();
            AddAssetList<BSPLightmap>();
            AddAssetList<Config>();
            AddAssetList<Cursor>();
            AddAssetList<Font>();
            AddAssetList<GAS>();
            AddAssetList<Model>();
            AddAssetList<Animation>("MOM");
            AddAssetList<NVC>();
            AddAssetList<SceneAsset>();
            AddAssetList<SceneInitFile>();
            AddAssetList<Sequence>();
            AddAssetList<Shader>();
            AddAssetList<SheepScript>();
            AddAssetList<Soundtrack>();
            AddAssetList<TextAsset>("");
            AddAssetList<Texture>();
            AddAssetList<VertexAnimation>();
            AddAssetList<Animation>("YAK");
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

template<typename T>
void AssetsTool::AddAssetList(const std::string& id)
{
    // Generate an asset ID based on the asset type and extension.
    const char* typeName = T::StaticTypeName();
    std::string assetId = std::string(typeName) + id;

    // Use asset type as an ID.
    ImGui::PushID(assetId.c_str());

     // Get list of loaded assets of this type, so we can display them in a giant tree view.
    const std::string_map_ci<T*>* loadedAssets = gAssetManager.GetLoadedAssets<T>(id);
    if(loadedAssets == nullptr)
    {
        return;
    }

    // For all nodes, only expand the tree if you click on the arrow.
    ImGuiTreeNodeFlags assetTypeFlags = ImGuiTreeNodeFlags_OpenOnArrow;

    // Draw the tree node.
    bool node_open;
    if(id.empty())
    {
        node_open = ImGui::TreeNodeEx(assetId.c_str(), assetTypeFlags, "%s (%zu)", typeName, loadedAssets->size());
    }
    else
    {
        node_open = ImGui::TreeNodeEx(assetId.c_str(), assetTypeFlags, "%s %s (%zu)", id.c_str(), typeName, loadedAssets->size());
    }

    // If open, draw all the loaded assets of this type.
    if(node_open)
    {
        for(auto& entry : *loadedAssets)
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