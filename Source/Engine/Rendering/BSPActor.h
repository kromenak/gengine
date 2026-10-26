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

    void SetHitTest(bool hitTest);
    bool IsHitTest() const;
    void SetHitTestVisible(bool visible);

    bool Raycast(const Ray& ray, RaycastHit& hitInfo);

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

    // If this is a hit test, this forces the hit test to be visible at all times.
    bool mForceHitTestVisible = false;

    void SetSurfacesVisible(bool visible);
    void SetSurfacesInteractive(bool interactive);

    bool ShouldBeVisible() const;
    bool ShouldBeInteractive() const;
};
