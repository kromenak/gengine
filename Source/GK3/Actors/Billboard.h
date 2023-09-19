//
// Clark Kromenaker
//
// A component that rotates the owner to always face the camera (cylindrical billboarding).
//
// This *is not* a general purpose billboarding solution, however!
// GK3 has some nonstandard setups for Actor local spaces, so this component needs to take that weirdness into account.
//
// In a sane world, the local space origin for a billboard actor would be at the bottom-center of the 3D mesh.
// In GK3 however, this is not usually the case - most 3D model local space origins are at world space origin (0, 0, 0) with the 3D mesh offset from there.
// This complicates the logic for billboarding significantly!
//
#pragma once
#include "Component.h"

#include "Vector3.h"

class MeshRenderer;

class Billboard : public Component
{
public:
    Billboard(Actor* owner);

protected:
    void OnUpdate(float deltaTime) override;

private:
    // The billboard needs to freqently query the mesh renderer, so cache it here.
    MeshRenderer* mMeshRenderer = nullptr;

    // When first creating the billboard actor, this is the initial world position we calculate for the billboard visuals.
    // Because the billboard visuals are not centered on the local space origin, rotating the billboard also changes it's position.
    // So we need this so we can move the billboard visuals back to the initial position.
    Vector3 mInitialPos;

    Vector3 GetRotatePosition();
};