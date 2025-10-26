//
// Clark Kromenaker
//
// An actor formed by a portion of BSP geometry.
//
#pragma once
#include "GKObject.h"

#include <string>
#include <vector>

#include "AABB.h"
#include "BSP.h"

class BSPActor : public GKObject
{
    TYPEINFO_SUB(BSPActor, GKObject);
public:
    BSPActor(BSP* bsp, const std::string& name, std::vector<BSPSurface*>& surfaces, std::vector<BSPPolygon*>& polygons);

    void SetAABB(const AABB& aabb) { mAABB = aabb; }
    AABB GetAABB() const override { return mAABB; }

    void SetVisible(bool visible);
    void SetInteractive(bool interactive);

    void SetHitTest(bool hitTest);
    bool IsHitTest() const;

    bool Raycast(const Ray& ray, RaycastHit& hitInfo);

    void OnPersist(PersistState& ps) override;

protected:
    void OnActive() override;
    void OnInactive() override;
    void OnUpdate(float deltaTime) override;

private:
    // The BSP this actor was created from.
    BSP* mBSP = nullptr;

    // An AABB around the BSP geometry that makes up the object.
    AABB mAABB;

    // BSP surfaces and polygons belonging to this actor.
    std::vector<BSPSurface*> mSurfaces;
    std::vector<BSPPolygon*> mPolygons;

    // If true, this BSP actor is visible (we are rendering its surfaces).
    bool mVisible = true;

    // If true, this BSP actor is interactive (it can be hit by raycasts).
    bool mInteractive = true;

    void RefreshSurfaceFlags();
    void SetSurfacesVisible(bool visible);
    void SetSurfacesInteractive(bool interactive);
};
