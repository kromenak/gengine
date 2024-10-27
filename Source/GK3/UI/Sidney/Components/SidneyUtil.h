//
// Clark Kromenaker
//
// The Sidney computer system is a fairly complex UI, so this class just has a few helper functions/factories
// for creating UI elements more quickly/easily.
//
#pragma once
#include <functional>
#include <string>

#include "Color32.h"
#include "Localizer.h"
#include "Vector2.h"

class Actor;
class SidneyButton;
class UIButton;
struct UINineSliceParams;

class SidneyUtil
{
public:
    static Color32 TransBgColor;

    static UIButton* CreateTextButton(Actor* parent, const std::string& text, const std::string& font,
                                      const Vector2& pivotAndAnchor, const Vector2& position, const Vector2& size);

    static Actor* CreateBackground(Actor* parent);
    static void CreateMainMenuButton(Actor* parent, std::function<void()> pressCallback);
    static Actor* CreateMenuBar(Actor* parent, const std::string& screenName, float labelWidth);

    static SidneyButton* CreateBigButton(Actor* parent);

    static const UINineSliceParams& GetGrayBoxParams(const Color32& centerColor);
    static const UINineSliceParams& GetGoldBoxParams(const Color32& centerColor);

    static const Localizer& GetMainScreenLocalizer();
    static const Localizer& GetAnalyzeLocalizer();
    static const Localizer& GetAddDataLocalizer();
    static const Localizer& GetMakeIdLocalizer();
};