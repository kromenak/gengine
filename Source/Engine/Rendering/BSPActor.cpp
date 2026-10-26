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
}

void BSPActor::SetHitTest(bool hitTest)
{
    // Just set the surfaces as hit tests directly.
    // We don't need to track a local flag for this, since setting active/inactive doesn't change this.
    for(auto& surface : mSurfaces)
    {
        surface->hitTest = hitTest;
    }

    // Enabing or disabling hit test may affect visibility.
    SetSurfacesVisible(ShouldBeVisible());
    SetSurfacesInteractive(ShouldBeInteractive());
}

bool BSPActor::IsHitTest() const
{
    return !mSurfaces.empty() && mSurfaces[0]->hitTest;
}

void BSPActor::SetHitTestVisible(bool visible)
{
    mForceHitTestVisible = visible;
    SetSurfacesVisible(ShouldBeVisible());
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

void BSPActor::OnActive()
{
    SetSurfacesVisible(ShouldBeVisible());
    SetSurfacesInteractive(ShouldBeInteractive());
}

void BSPActor::OnInactive()
{
    SetSurfacesVisible(ShouldBeVisible());
    SetSurfacesInteractive(ShouldBeInteractive());
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

bool BSPActor::ShouldBeVisible() const
{
    // "Normal" BSP Actors are always visible when active.
    // Hit tests are only visible when the debug flag is on.
    return IsActive() && (!IsHitTest() || Debug::GetFlag("ShowHitTests") || mForceHitTestVisible);
}

bool BSPActor::ShouldBeInteractive() const
{
    // Both "Normal" and hit test BSP actors are always interactive if active.
    return IsActive();
}
