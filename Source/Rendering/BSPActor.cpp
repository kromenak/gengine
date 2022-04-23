#include "BSPActor.h"

BSPActor::BSPActor(BSP* bsp, const std::string& name) : GKObject(),
	mBSP(bsp),
	mName(name)
{
	
}

void BSPActor::SetVisible(bool visible)
{
	for(auto& surface : mSurfaces)
	{
		surface->visible = visible;
	}
}

void BSPActor::SetInteractive(bool interactive)
{
	for(auto& surface : mSurfaces)
	{
		surface->interactive = interactive;
	}
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
				hitInfo.name = mName;
				hitInfo.actor = this;
				return true;
			}
		}
	}
	return false;
}

void BSPActor::OnActive()
{
	SetVisible(true);
	SetInteractive(true);
}

void BSPActor::OnInactive()
{
	SetVisible(false);
	SetInteractive(false);
}

void BSPActor::OnUpdate(float deltaTime)
{
	// Draw debug AABB to visualize interactable BSP objects in the scene.
	// Noun check stops visualizing non-interactive BSP objects.
	if(!GetNoun().empty())
	{
		//mAABB.DebugDraw(Color32::Green);
	}
}


