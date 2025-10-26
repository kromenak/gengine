#include "BSPActor.h"

#include "Debug.h"

TYPEINFO_INIT(BSPActor, GKObject, GENERATE_TYPE_ID)
{

}

BSPActor::BSPActor(BSP* bsp, const std::string& name, std::vector<BSPSurface*>& surfaces, std::vector<BSPPolygon*>& polygons) : GKObject(),
    mBSP(bsp),
    mSurfaces(std::move(surfaces)),
    mPolygons(std::move(polygons))
{
    SetName(name);

    // When a BSP actor is created, it's not always necessarily visible and interactive to start.
    // Update surface flags to match the current state of the underlying BSP surfaces.
    RefreshSurfaceFlags();
}

void BSPActor::SetVisible(bool visible)
{
    // Update visibility flag and then actually set surfaces visible or not.
    mVisible = visible;
    SetSurfacesVisible(visible);
}

void BSPActor::SetInteractive(bool interactive)
{
    // Update interactivity flag and then actually set surfaces visible or not.
    mInteractive = interactive;
    SetSurfacesInteractive(interactive);
}

void BSPActor::SetHitTest(bool hitTest)
{
    // Just set the surfaces as hit tests directly.
    // We don't need to track a local flag for this, since setting active/inactive doesn't change this.
    for(auto& surface : mSurfaces)
    {
        surface->hitTest = hitTest;
    }
}

bool BSPActor::IsHitTest() const
{
    return !mSurfaces.empty() && mSurfaces[0]->hitTest;
}

bool BSPActor::Raycast(const Ray& ray, RaycastHit& hitInfo)
{
    // Do a check against the AABB first.
    if(Intersect::TestRayAABB(ray, mAABB, hitInfo.t))
    {
        // If AABB check succeeds, do a per-polygon test.
        for(auto& polygon : mPolygons)
        {
            if(mBSP->RaycastPolygon(ray, polygon, hitInfo))
            {
                hitInfo.name = GetName();
                hitInfo.actor = this;
                return true;
            }
        }
    }
    return false;
}

void BSPActor::OnPersist(PersistState& ps)
{
    // BSP geometry is save/loaded before BSPActors.
    // So, when loading, the BSP geometry may have already loaded to a different state that no longer matches our state flags.
    // As a result, we need to refresh those before loading so they're accurate - they have an effect when Persist code activates or deactivates.
    if(ps.IsLoading())
    {
        RefreshSurfaceFlags();
    }

    // And then persist normally.
    GKObject::OnPersist(ps);
}

void BSPActor::OnActive()
{
    // For a BSPActor to be visible/interactive, it must be active AND the corresponding flag must be true.
    SetSurfacesVisible(mVisible);
    SetSurfacesInteractive(mInteractive);
}

void BSPActor::OnInactive()
{
    // Since we're about to force this BSPActor invisible and noninteractive, make sure our local state flags are up to date.
    RefreshSurfaceFlags();

    // If BSPActor is inactive, it is always invisible and noninteractive, overriding individual state flags.
    SetSurfacesVisible(false);
    SetSurfacesInteractive(false);
}

void BSPActor::OnUpdate(float deltaTime)
{
    // Draw debug AABB to visualize interactable BSP objects in the scene.
    // Noun check stops visualizing non-interactive BSP objects.
    if(Debug::RenderAABBs() && !GetNoun().empty())
    {
        Debug::DrawAABB(mAABB, Color32::Orange);
    }
}

void BSPActor::RefreshSurfaceFlags()
{
    // Derive initial visibility/interactivity from the BSP surface values.
    // We're assuming that all surfaces of an object are set visible/interactive as a whole, so checking the first one is sufficient.
    mVisible = !mSurfaces.empty() && mSurfaces[0]->visible;
    mInteractive = !mSurfaces.empty() && mSurfaces[0]->interactive;
}

void BSPActor::SetSurfacesVisible(bool visible)
{
    for(auto& surface : mSurfaces)
    {
        surface->visible = visible;
    }
}

void BSPActor::SetSurfacesInteractive(bool interactive)
{
    for(auto& surface : mSurfaces)
    {
        surface->interactive = interactive;
    }
}
