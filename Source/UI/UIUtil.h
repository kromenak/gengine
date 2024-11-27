//
// Clark Kromenaker
//
// Utilities to help with creating UIs from code.
// Handles common boilerplate.
//
#pragma once
#include "Actor.h"

class Color32;
class UICanvas;

namespace UIUtil
{
    void AddCanvas(Actor* canvasActor, int canvasOrder);
    void AddColorCanvas(Actor* canvasActor, int canvasOrder, const Color32& color);
    UICanvas* NewUIActorWithCanvas(Actor* parent, int canvasOrder);

    template<typename T> T* NewUIActorWithWidget(Actor* parent)
    {
        Actor* uiActor = new Actor(TransformType::RectTransform);
        uiActor->GetTransform()->SetParent(parent->GetTransform());
        return uiActor->AddComponent<T>();
    }
}