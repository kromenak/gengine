#include "MainMenuTool.h"

#include <imgui.h>

#include "Debug.h"
#include "GEngine.h"
#include "Scene.h"
#include "SceneConstruction.h"
#include "Tools.h"

void MainMenuTool::Render()
{
    // This menu won't be super valuable if the scene is null.
    Scene* scene = GEngine::Instance()->GetScene();
    if(scene == nullptr) { return; }

    // Get construction object.
    SceneConstruction& construction = scene->GetConstruction();

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

        // VIEW menu
        if(ImGui::BeginMenu("View"))
        {
            if(ImGui::MenuItem("Bounding Boxes", nullptr, Debug::GetFlag("ShowBoundingBoxes")))
            {
                Debug::ToggleFlag("ShowBoundingBoxes");
            }
            if(ImGui::MenuItem("Camera Bounds", nullptr, construction.GetShowCameraBounds()))
            {
                construction.SetShowCameraBounds(!construction.GetShowCameraBounds());
            }
            if(ImGui::MenuItem("Walker Bounds", nullptr, construction.GetShowWalkerBoundary()))
            {
                construction.SetShowWalkerBoundary(!construction.GetShowWalkerBoundary());
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