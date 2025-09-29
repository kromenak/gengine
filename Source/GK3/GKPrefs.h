//
// Clark Kromenaker
//
// A helper for easily accessing user preferences.
//
#pragma once
#include "SaveManager.h"

namespace Prefs
{
    inline bool UseOriginalUIScalingLogic()
    {
        return gSaveManager.GetPrefs()->GetBool(PREFS_UI, PREFS_USE_ORIGINAL_UI_SCALING_LOGIC, true);
    }

    inline bool ScaleUIAtHighResolutions()
    {
        return gSaveManager.GetPrefs()->GetBool(PREFS_UI, PREFS_SCALE_UI_AT_HIGH_RESOLUTIONS, true);
    }

    inline int GetMinimumScaleUIHeight()
    {
        return gSaveManager.GetPrefs()->GetInt(PREFS_UI, PREFS_SCALE_UI_MINIMUM_HEIGHT, 1280);
    }

    inline bool UsePixelPerfectUIScaling()
    {
        return gSaveManager.GetPrefs()->GetBool(PREFS_UI, PREFS_PIXEL_PERFECT_UI_SCALING, true);
    }

    inline float GetUIScalingBias()
    {
        return gSaveManager.GetPrefs()->GetFloat(PREFS_UI, PREFS_UI_SCALING_BIAS, 0.0f);
    }
}