#include "Tools.h"

#include <imgui.h>
#include <imgui_impl_sdl.h>

#include "GAPI.h"
#include "InputManager.h"
#include "SceneManager.h"

#include "AssetsTool.h"
#include "HierarchyTool.h"
#include "MainMenuTool.h"
#include "RaycastTool.h"
#include "SettingsTool.h"

namespace
{
    // Have tools been initialized?
    bool toolsInitialized = false;

    // Are tools active globally?
    bool toolsActive = false;

    // Do we want to render the main menu?
    // Usually true, but not if we're showing a tool in isolation.
    bool renderMainMenu = true;

    // Individual tools.
    MainMenuTool mainMenu;
    HierarchyTool hierarchy;
    AssetsTool assets;
    RaycastTool raycasts;
    SettingsTool settings;
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
        SetActive(!Active());
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
        // Render the main menu, which grants access to all other tools.
        // We usually DO want to render this, but not if we're rendering a specific tool in isolation.
        if(renderMainMenu)
        {
            mainMenu.Render();
        }

        // Render specific tools based on whether they are active.
        hierarchy.Render(mainMenu.hierarchyToolActive);
        assets.Render(mainMenu.assetsToolActive);
        raycasts.Render(mainMenu.raycastToolActive);
        settings.Render(mainMenu.settingsToolActive);

        // Optionally show demo window.
        //ImGui::ShowDemoWindow();
    }

    // Render the frame.
    ImGui::Render();
    GAPI::Get()->ImGuiRenderDrawData();

    // Handle disabling tools if an isolated tool window (such as settings) is closed.
    // A more tedious way to do this would be to manually track whether tools are active AND anything was rendered (check all the bools).
    // Luckily, IMGUI tracks active window count. Note that this is still 1 when nothing is showing (I guess maybe that's the entire frame?).
    if(ImGui::GetIO().MetricsActiveWindows == 1)
    {
        toolsActive = false;
    }
}

void Tools::SetActive(bool active)
{
    toolsActive = active;

    // If tools are activated via normal means, make sure main menu always renders.
    renderMainMenu = true;
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

void Tools::ShowSettings()
{
    // Show settings in isolation. This means:
    // 1) Tools are active.
    // 2) But don't show the main menu - we only want settings.
    // 3) Show the settings tool.
    toolsActive = true;
    renderMainMenu = false;
    mainMenu.settingsToolActive = true;
}

