//
// Clark Kromenaker
//
// The Sidney computer system is a fairly complex UI, so this class just has a few helper functions/factories
// for creating UI elements more quickly/easily.
//
#pragma once
#include <string>

#include "Localizer.h"
#include "Vector2.h"

class Actor;
class UIButton;

class SidneyUtil
{
public:
    static UIButton* CreateTextButton(Actor* parent, const std::string& text, const std::string& font,
                                      const Vector2& pivotAndAnchor, const Vector2& position, const Vector2& size);
    static void CreateMenuBar(Actor* parent, const std::string& screenName);

    static const Localizer& GetMainScreenLocalizer();
    static const Localizer& GetAddDataLocalizer();
};