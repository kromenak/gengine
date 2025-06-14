//
// Clark Kromenaker
//
// A component is a reusable bit of functionality that can be attached to an Actor.
//
#pragma once
#include "TypeInfo.h"

class Actor;

class Component
{
    TYPEINFO_BASE(Component);
public:
    Component(Actor* owner);
    virtual ~Component() = default;

    void Update(float deltaTime);
    void LateUpdate(float deltaTime);

    Actor* GetOwner() const { return mOwner; }

    void SetEnabled(bool enabled);
    bool IsEnabled() const { return mEnabled; }

    bool IsActiveAndEnabled() const;

protected:
    virtual void OnEnable() { }
    virtual void OnDisable() { }
    virtual void OnUpdate(float deltaTime) { }
    virtual void OnLateUpdate(float deltaTime) { }

private:
    // The component's owner.
    friend class Actor;
    Actor* mOwner = nullptr;

    // Is the component enabled? If not, OnUpdate won't be called.
    // Components can otherwise use this as needed - for example, a disabled UIWidget may not render.
    bool mEnabled = true;
};

inline void Component::Update(float deltaTime)
{
    if(mEnabled)
    {
        OnUpdate(deltaTime);
    }
}

inline void Component::LateUpdate(float deltaTime)
{
    if(mEnabled)
    {
        OnLateUpdate(deltaTime);
    }
}
