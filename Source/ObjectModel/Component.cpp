#include "Component.h"

#include "Actor.h"

TYPEINFO_INIT(Component, NoBaseClass, 1)
{

}

Component::Component(Actor* owner) : mOwner(owner)
{
    
}

void Component::SetEnabled(bool enabled)
{
    if(mEnabled != enabled)
    {
        mEnabled = enabled;
        if(mEnabled)
        {
            OnEnable();
        }
        else
        {
            OnDisable();
        }
    }
}

bool Component::IsActiveAndEnabled() const
{
	return mEnabled && mOwner != nullptr && mOwner->IsActive();
}
