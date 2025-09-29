//
// Clark Kromenaker
//
// Helpers for creating UI actors/components via code.
// These functions help cut down on boilerplate code typically required to set up a new UI actor.
//
#pragma once
#include <string>

#include "Actor.h"
#include "Color32.h"

class UICanvas;

namespace UI
{
    template<typename T, typename... Args>
    T* CreateWidgetActor(const std::string& name, Actor* parent, Args&&... args)
    {
        Actor* uiActor = new Actor(name, TransformType::RectTransform);
        uiActor->GetTransform()->SetParent(parent->GetTransform());
        return uiActor->AddComponent<T>(std::forward<Args>(args)...);
    }

    template<typename T, typename... Args>
    T* CreateWidgetActor(const std::string& name, Component* parent, Args&&... args)
    {
        return CreateWidgetActor<T>(name, parent->GetOwner(), std::forward<Args>(args)...);
    }

    UICanvas* AddCanvas(Actor* actor, int canvasOrder, const Color32& color = Color32::Clear);
    UICanvas* CreateCanvas(const std::string& name, Actor* parent, int canvasOrder, const Color32& color = Color32::Clear);
    UICanvas* CreateCanvas(const std::string& name, Component* parent, int canvasOrder, const Color32& color = Color32::Clear);

    float GetScaleFactor(float minimumUIScaleHeight, bool usePixelPerfectScaling, float bias = 0.0f);
}