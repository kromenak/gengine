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
    BSPActor(BSP* bsp, const std::string& name);

    void AddSurface(BSPSurface* surface) { mSurfaces.push_back(surface); }
    void AddPolygon(BSPPolygon* polygon) { mPolygons.push_back(polygon); }

    void SetAABB(const AABB& aabb) { mAABB = aabb; }
    AABB GetAABB() const override { return mAABB; }

    void SetVisible(bool visible);
    void SetInteractive(bool interactive);

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
};
