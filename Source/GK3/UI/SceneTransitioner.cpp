#include "SceneTransitioner.h"

#include "UIUtil.h"

SceneTransitioner::SceneTransitioner() : Actor(TransformType::RectTransform)
{
    // Draw scene transitioner above the normal scene.
    UI::AddCanvas(this, 12, Color32(0, 0, 0, 128));
}

void SceneTransitioner::Show()
{
    SetActive(true);
}

void SceneTransitioner::Hide()
{
    SetActive(false);
}