#include "Tools.h"

#include <imgui.h>
#include <imgui_impl_opengl3.h>
#include <imgui_impl_sdl.h>

#include "GEngine.h"
#include "Scene.h"
#include "Services.h"
#include "Window.h"

#include "MainMenuTool.h"

namespace
{
    // Are tools active globally?
    bool toolsActive = false;

    // Individual tools.
    MainMenuTool mainMenu;
}

void Tools::Init()
{
    // Create IMGUI context.
    IMGUI_CHECKVERSION();
    ImGui::CreateContext();

    // Init for SDL & OpenGL.
    ImGui_ImplSDL2_InitForOpenGL(Window::Get(), Services::GetRenderer()->GetGLContext());
    ImGui_ImplOpenGL3_Init("#version 150");

    // We'll use dark mode.
    ImGui::StyleColorsDark();
}

void Tools::Shutdown()
{
    ImGui_ImplOpenGL3_Shutdown();
    ImGui_ImplSDL2_Shutdown();
    ImGui::DestroyContext();
}

void Tools::Update()
{
    // Toggle tools with Tab key.
    if(Services::GetInput()->IsKeyLeadingEdge(SDL_SCANCODE_TAB))
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
    Scene* scene = GEngine::Instance()->GetScene();
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

