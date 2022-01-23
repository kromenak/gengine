#include "Component.h"

#include "Actor.h"

TYPE_DEF_BASE(Component);

Component::Component(Actor* owner) : mOwner(owner)
{
    
}

bool Component::IsActiveAndEnabled() const
{
	return mEnabled && mOwner != nullptr && mOwner->IsActive();
}
