#include "Tools.h"

#include <imgui.h>
#include <imgui_impl_opengl3.h>
#include <imgui_impl_sdl.h>

#include "InputManager.h"
#include "Renderer.h"
#include "SceneManager.h"
#include "Window.h"

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
}

void Tools::Init()
{
    if(!toolsInitialized)
    {
        // Create IMGUI context.
        IMGUI_CHECKVERSION();
        ImGui::CreateContext();

        // Init for SDL & OpenGL.
        ImGui_ImplSDL2_InitForOpenGL(Window::Get(), gRenderer.GetGLContext());
        ImGui_ImplOpenGL3_Init("#version 150");

        // We'll use dark mode.
        ImGui::StyleColorsDark();
        
        toolsInitialized = true;
    }
}

void Tools::Shutdown()
{
    if(toolsInitialized)
    {
        // Shutdown/destroy in reverse order of initialization.
        ImGui_ImplOpenGL3_Shutdown();
        ImGui_ImplSDL2_Shutdown();
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
    if(!toolsActive) { return; }

    // Render construction mode.
    //TODO: This is *kind of* not the right spot for this, since tools render *after* debug drawing stuff.
    //TODO: So any Debug::Draw* calls will be on frame late. But maybe that's no big deal?
    Scene* scene = gSceneManager.GetScene();
    if(scene != nullptr)
    {
        scene->GetConstruction().Render();
    }

    // Start a new frame.
    ImGui_ImplOpenGL3_NewFrame();
    ImGui_ImplSDL2_NewFrame();
    ImGui::NewFrame();

    // Render any tools.
    mainMenu.Render();
    hierarchy.Render(mainMenu.hierarchyToolActive);

    // Optionally show demo window.
    //ImGui::ShowDemoWindow();

    // Render with OpenGL.
    ImGui::Render();
    ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());
}

void Tools::SetActive(bool active)
{
    toolsActive = active;
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

