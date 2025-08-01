#include "Tools.h"

#include <imgui.h>
#include <imgui_impl_sdl.h>

#include "GAPI.h"
#include "InputManager.h"
#include "SceneManager.h"
#include "Window.h"

#include "AssetsTool.h"
#include "HierarchyTool.h"
#include "MainMenuTool.h"

namespace
{
    // Have tools been initialized?
    bool toolsInitialized = false;

    // Are tools active globally?
    bool toolsActive = false;

    // Individual tools.
    MainMenuTool mainMenu;
    HierarchyTool hierarchy;
    AssetsTool assets;
}

void Tools::Init()
{
    if(!toolsInitialized)
    {
        // Create IMGUI context.
        // We're assuming that the IMGUI graphics context is created by the rendering system.
        IMGUI_CHECKVERSION();
        ImGui::CreateContext();
        ImGui::StyleColorsDark();
        toolsInitialized = true;
    }
}

void Tools::Shutdown()
{
    if(toolsInitialized)
    {
        // Destroy IMGUI context.
        // We're assuming that the IMGUI graphics context was destroyed by the rendering system before this happens.
        ImGui::DestroyContext();
        toolsInitialized = false;
    }
}

void Tools::Update()
{
    // Toggle tools with Tab key.
    if(gInputManager.IsKeyLeadingEdge(SDL_SCANCODE_TAB))
    {
        toolsActive = !toolsActive;
    }
}

void Tools::Render()
{
    // Render construction mode.
    //TODO: This is *kind of* not the right spot for this, since tools render *after* debug drawing stuff.
    //TODO: So any Debug::Draw* calls will be one frame late. But maybe that's no big deal?
    if(toolsActive)
    {
        Scene* scene = gSceneManager.GetScene();
        if(scene != nullptr)
        {
            scene->GetConstruction().Render();
        }
    }

    // Start a new frame.
    GAPI::Get()->ImGuiNewFrame();
    ImGui_ImplSDL2_NewFrame();
    ImGui::NewFrame();

    // Render any tools.
    if(toolsActive)
    {
        mainMenu.Render();
        hierarchy.Render(mainMenu.hierarchyToolActive);
        assets.Render(mainMenu.assetsToolActive);

        // Optionally show demo window.
        //ImGui::ShowDemoWindow();
    }

    // Render with OpenGL.
    ImGui::Render();
    GAPI::Get()->ImGuiRenderDrawData();
}

void Tools::SetActive(bool active)
{
    toolsActive = active;
}

bool Tools::Active()
{
    return toolsActive;
}

void Tools::ProcessEvent(const SDL_Event& event)
{
    ImGui_ImplSDL2_ProcessEvent(&event);
}

bool Tools::EatingMouseInputs()
{
    return toolsActive && ImGui::GetIO().WantCaptureMouse;
}

bool Tools::EatingKeyboardInputs()
{
    return toolsActive && ImGui::GetIO().WantCaptureKeyboard;
}

