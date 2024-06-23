//
// Clark Kromenaker
//
// Utilities to help with creating UIs from code.
// Handles common boilerplate.
//
#pragma once

class Actor;
class Color32;

namespace UIUtil
{
    void AddCanvas(Actor* canvasActor, int canvasOrder);
    void AddColorCanvas(Actor* canvasActor, int canvasOrder, const Color32& color);

    template<typename T> T* NewUIActorWithWidget(Actor* parent)
    {
        Actor* uiActor = new Actor(TransformType::RectTransform);
        uiActor->GetTransform()->SetParent(parent->GetTransform());
        return uiActor->AddComponent<T>();
    }
}