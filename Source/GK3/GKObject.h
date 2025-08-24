//
// Clark Kromenaker
//
// Base class for an object of interest in a GK3 scene.
//
// Could be a piece of interactable BSP geometry, a prop model in the scene,
// or even a full-fledged humanoid actor that pathfinds, emotes, etc.
//
#pragma once
#include "Actor.h"

#include "AABB.h"
#include "Heading.h"

class Color32;
class MeshRenderer;
class PersistState;
class VertexAnimation;
struct VertexAnimParams;

class GKObject : public Actor
{
    TYPEINFO_SUB(GKObject, Actor);
public:
    GKObject() = default;

    // GK3 rotations are often defined in terms of a heading (360 degrees about Y-axis).
    virtual void SetHeading(const Heading& heading);
    Heading GetHeading() const;

    void DumpPosition();

    void SetNoun(const std::string& noun) { mNoun = noun; }
    const std::string& GetNoun() const { return mNoun; }

    void SetVerb(const std::string& verb) { mVerb = verb; }
    const std::string& GetVerb() const { return mVerb; }

    bool CanInteract() const { return IsActive() && !mNoun.empty(); }

    //TODO (but maybe OK): feels like these shouldn't be here, BUT some subclasses need these...
    virtual void StartAnimation(VertexAnimParams& animParams) { }
    virtual void SampleAnimation(VertexAnimParams& animParams, int frame) { }
    virtual void StopAnimation(VertexAnimation* anim = nullptr) { }
    virtual MeshRenderer* GetMeshRenderer() const { return nullptr;  }
    void SetFixedLightingColor(const Color32& color);

    virtual AABB GetAABB() const { return AABB::FromCenterAndSize(GetPosition(), Vector3::One); }

    Vector3 GetAudioPosition() const;

    virtual void OnPersist(PersistState& ps);

private:
    // A noun is used to refer to the objects in NVC logic.
    // Only objects with nouns can be interacted with!
    std::string mNoun;

    // When defined, the verb indicates that interacting with this object ONLY
    // executes this verb in NVC logic. This often also changes the cursor.
    std::string mVerb;
};
