#include "SettingsTool.h"

#include <imgui.h>

#include "GKPrefs.h"
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

    bool useOriginalUIScalingLogic = Prefs::UseOriginalUIScalingLogic();
    ImGui::Checkbox(PREFS_USE_ORIGINAL_UI_SCALING_LOGIC, &useOriginalUIScalingLogic);
    AddTooltip("Enables UI scaling logic present in the original game.\nFor example, scaling the title screen, driving screen, and a few others.");
    gSaveManager.GetPrefs()->Set(PREFS_UI, PREFS_USE_ORIGINAL_UI_SCALING_LOGIC, useOriginalUIScalingLogic);

    bool scaleUIsAtHighResolution = Prefs::ScaleUIAtHighResolutions();
    ImGui::Checkbox(PREFS_SCALE_UI_AT_HIGH_RESOLUTIONS, &scaleUIsAtHighResolution);
    AddTooltip("Increases UI scale when playing the game at high resolutions.");
    gSaveManager.GetPrefs()->Set(PREFS_UI, PREFS_SCALE_UI_AT_HIGH_RESOLUTIONS, scaleUIsAtHighResolution);

    int minScaleHeight = Prefs::GetMinimumScaleUIHeight();
    ImGui::SetNextItemWidth(100.0f);
    ImGui::InputInt(PREFS_SCALE_UI_MINIMUM_HEIGHT, &minScaleHeight);
    AddTooltip("If the resolution height is below this value, UI scaling will not occur.\nModify this if you think the UI is too big or too small at your target resolution.");
    gSaveManager.GetPrefs()->Set(PREFS_UI, PREFS_SCALE_UI_MINIMUM_HEIGHT, minScaleHeight);

    bool pixelPerfectUIScaling = Prefs::UsePixelPerfectUIScaling();
    ImGui::Checkbox(PREFS_PIXEL_PERFECT_UI_SCALING, &pixelPerfectUIScaling);
    AddTooltip("Only allows scaling UI if pixel boundaries are maintained.\nDisabling allows very flexible scaling behavior, but expect graphical artifacts.");
    gSaveManager.GetPrefs()->Set(PREFS_UI, PREFS_PIXEL_PERFECT_UI_SCALING, pixelPerfectUIScaling);

    // End window.
    ImGui::End();
}