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
class UIImage;
class UILabel;
struct UINineSliceParams;

struct FilePreviewWindow
{
    Actor* root = nullptr;
    UILabel* header = nullptr;
    UIImage* image = nullptr;
};

class SidneyUtil
{
public:
    static Color32 TransBgColor;
    static Color32 VeryTransBgColor;

    static Actor* CreateBackground(Actor* parent);
    static void CreateMainMenuButton(Actor* parent, const std::function<void()>& pressCallback);
    static UIButton* CreateCloseWindowButton(Actor* parent, const std::function<void()>& pressCallback);

    static SidneyButton* CreateBigButton(Actor* parent);
    static SidneyButton* CreateSmallButton(Actor* parent);

    static FilePreviewWindow CreateFilePreviewWindow(Actor* parent);

    static const UINineSliceParams& GetGrayBoxParams(const Color32& centerColor);
    static const UINineSliceParams& GetGoldBoxParams(const Color32& centerColor);

    static const Localizer& GetMainScreenLocalizer();
    static const Localizer& GetSearchLocalizer();
    static const Localizer& GetEmailLocalizer();
    static const Localizer& GetAnalyzeLocalizer();
    static const Localizer& GetTranslateLocalizer();
    static const Localizer& GetAddDataLocalizer();
    static const Localizer& GetMakeIdLocalizer();
    static const Localizer& GetSuspectsLocalizer();

    static int GetCurrentLSRStep();
};