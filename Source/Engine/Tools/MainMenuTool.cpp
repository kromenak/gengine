#include "MainMenuTool.h"

#include <imgui.h>

#include "Debug.h"
#include "SceneManager.h"
#include "SceneConstruction.h"
#include "Tools.h"

void MainMenuTool::Render()
{
    // Render the menu bar.
    if(ImGui::BeginMainMenuBar())
    {
        // FILE menu
        if(ImGui::BeginMenu("File"))
        {
            if(ImGui::MenuItem("Exit Construction Mode"))
            {
                Tools::SetActive(false);
            }
            ImGui::EndMenu();
        }

        // Get current preferences for showing scene visualizations, so we can show the right state in the menu bar.
        Scene* scene = gSceneManager.GetScene();
        bool showSceneCameraBounds = false;
        bool showSceneRegions = false;
        bool showSceneWalkerBoundaries = false;
        if(scene != nullptr)
        {
            showSceneCameraBounds = scene->GetConstruction().GetShowCameraBounds();
            showSceneRegions = scene->GetConstruction().GetShowRegions();
            showSceneWalkerBoundaries = scene->GetConstruction().GetShowWalkerBoundary();
        }

        // VIEW menu
        if(ImGui::BeginMenu("View"))
        {
            if(ImGui::MenuItem("Draw Debug Shapes Over Geometry", nullptr, Debug::GetFlag("DrawDebugShapesOverGeometry")))
            {
                Debug::ToggleFlag("DrawDebugShapesOverGeometry");
            }
            if(ImGui::MenuItem("Bounding Boxes", nullptr, Debug::GetFlag("ShowBoundingBoxes")))
            {
                Debug::ToggleFlag("ShowBoundingBoxes");
            }
            if(ImGui::MenuItem("Camera Bounds", nullptr, showSceneCameraBounds, scene != nullptr))
            {
                scene->GetConstruction().SetShowCameraBounds(!showSceneCameraBounds);
            }
            if(ImGui::MenuItem("Regions/Triggers", nullptr, showSceneRegions, scene != nullptr))
            {
                scene->GetConstruction().SetShowRegions(!showSceneRegions);
            }
            if(ImGui::MenuItem("Walker Bounds", nullptr, showSceneWalkerBoundaries, scene != nullptr))
            {
                scene->GetConstruction().SetShowWalkerBoundary(!showSceneWalkerBoundaries);
            }
            if(ImGui::MenuItem("Walker Paths", nullptr, Debug::GetFlag("ShowWalkerPaths"), scene != nullptr))
            {
                Debug::ToggleFlag("ShowWalkerPaths");
            }
            ImGui::EndMenu();
        }

        // WINDOW menu
        if(ImGui::BeginMenu("Window"))
        {
            if(ImGui::MenuItem("Scene Hierarchy", nullptr, hierarchyToolActive))
            {
                hierarchyToolActive = !hierarchyToolActive;
            }
            if(ImGui::MenuItem("Assets", nullptr, assetsToolActive))
            {
                assetsToolActive = !assetsToolActive;
            }
            if(ImGui::MenuItem("Raycasts", nullptr, raycastToolActive))
            {
                raycastToolActive = !raycastToolActive;
            }
            if(ImGui::MenuItem("Settings", nullptr, settingsToolActive))
            {
                settingsToolActive = !settingsToolActive;
            }
            ImGui::EndMenu();
        }

        /*
        if(ImGui::BeginMenu("File"))
        {
            //ShowExampleMenuFile();
            ImGui::EndMenu();
        }
        if(ImGui::BeginMenu("Edit"))
        {
            if(ImGui::MenuItem("Undo", "CTRL+Z")) { }
            if(ImGui::MenuItem("Redo", "CTRL+Y", false, false)) { }  // Disabled item
            ImGui::Separator();
            if(ImGui::MenuItem("Cut", "CTRL+X")) { }
            if(ImGui::MenuItem("Copy", "CTRL+C")) { }
            if(ImGui::MenuItem("Paste", "CTRL+V")) { }
            ImGui::EndMenu();
        }
        */
        ImGui::EndMainMenuBar();
    }
}