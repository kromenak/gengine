#include "Component.h"

#include "Actor.h"

TYPE_DEF_BASE(Component);

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
