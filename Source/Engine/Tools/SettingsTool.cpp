#include "SettingsTool.h"

#include <imgui.h>

#include "SaveManager.h"

namespace
{
    void AddTooltip(const char* text)
    {
        ImGui::SameLine();
        ImGui::Text(" (i)");
        if(ImGui::IsItemHovered())
        {
            ImGui::BeginTooltip();
            ImGui::Text(text);
            ImGui::EndTooltip();
        }
    }

    int referenceHeight = 480;
}

void SettingsTool::Render(bool& toolActive)
{
    if(!toolActive) { return; }

    // Sets the default size of the window on first open.
    ImGui::SetNextWindowSize(ImVec2(430, 450), ImGuiCond_FirstUseEver);

    // Begin the window. Early out if collapsed.
    if(!ImGui::Begin("Settings", &toolActive))
    {
        ImGui::End();
        return;
    }

    bool useOriginalUIScalingLogic = gSaveManager.GetPrefs()->GetBool(PREFS_UI, PREFS_USE_ORIGINAL_UI_SCALING_LOGIC, true);
    ImGui::Checkbox("Use Original UI Scaling", &useOriginalUIScalingLogic);
    AddTooltip("Enables UI scaling present in the original game.\nFor example, scaling the title screen, driving screen, and a few others.");
    gSaveManager.GetPrefs()->Set(PREFS_UI, PREFS_USE_ORIGINAL_UI_SCALING_LOGIC, useOriginalUIScalingLogic);

    bool scaleUIsAtHighResolution = gSaveManager.GetPrefs()->GetBool(PREFS_UI, PREFS_SCALE_UI_AT_HIGH_RESOLUTIONS, true);
    ImGui::Checkbox("Scale UIs at High Resolutions", &scaleUIsAtHighResolution);
    AddTooltip("Increases UI scale when playing at higher resolutions.\nScale amount is (window_height / reference_height).");
    gSaveManager.GetPrefs()->Set(PREFS_UI, PREFS_SCALE_UI_AT_HIGH_RESOLUTIONS, scaleUIsAtHighResolution);

    int referenceHeight = gSaveManager.GetPrefs()->GetInt(PREFS_UI, PREFS_REFERENCE_HEIGHT, 480);
    ImGui::SetNextItemWidth(100.0f);
    ImGui::InputInt("Reference Height", &referenceHeight);
    AddTooltip("Can be used to tweak whether UI will scale at your current resolution.\nNot recommended to change significantly!");
    gSaveManager.GetPrefs()->Set(PREFS_UI, PREFS_REFERENCE_HEIGHT, referenceHeight);

    bool pixelPerfectUIScaling = gSaveManager.GetPrefs()->GetBool(PREFS_UI, PREFS_PIXEL_PERFECT_UI_SCALING, true);
    ImGui::Checkbox("Pixel Perfect UI Scaling", &pixelPerfectUIScaling);
    AddTooltip("Only allows scaling UI if pixel boundaries are maintained.\nDisabling allows very flexible scaling behavior, but expect graphical artifacts.");
    gSaveManager.GetPrefs()->Set(PREFS_UI, PREFS_PIXEL_PERFECT_UI_SCALING, pixelPerfectUIScaling);

    // End window.
    ImGui::End();
}