#include "DemonFight.h"

#include "GameProgress.h"
#include "GKActor.h"
#include "Heading.h"
#include "SceneManager.h"
#include "Walker.h"

DemonFight::DemonFight() : Actor("DemonFightHelper")
{
    // In this scene, Gabe can't walk freely.
    // Instead, trying to walk sets a flag that tells TE6.SHP to play some move animations.
    gSceneManager.GetScene()->SetWalkOverride([](){
        if(!gGameProgress.GetFlag("Te6GabeWalk"))
        {
            gGameProgress.SetFlag("Te6ClickedOnFloor");
        }
    });

    // Also, the Demon can't walk skip, as that messes up the timing of this fight.
    GKActor* actor = gSceneManager.GetScene()->GetActorByNoun("Asmodeus");
    if(actor != nullptr)
    {
        actor->GetWalker()->SetAllowSkip(false);
    }
}

void DemonFight::OnLateUpdate(float deltaTime)
{
    // Despite my best efforts, Gabe's "walk backwards" animations during this fight are not cooperating.
    // No matter what I do, Gabe ends up rotating into a wall or the center of the room, and not circling the pentagram as expected.
    // (This likely means there are bugs in GKActor/Model anim rotation code...which is not surprising.)

    // To work around this, the below code forces Gabe to stay on a circular walk path as he moves around the room during the demon fight.
    // The result isn't perfect, but it looks a heck of a lot better than it does without this HACK!
    GKActor* actor = gSceneManager.GetScene()->GetActorByNoun("Gabriel");
    if(actor != nullptr)
    {
        // Only apply fixes while Gabe is actively walking.
        // The Sheepscript for this scene sets this flag when Gabe is walking backwards.
        if(gGameProgress.GetFlag("Te6GabeWalk"))
        {
            // Keep position on a radius around the center of the room.
            const float kWalkRadius = 210.0f;
            Vector3 originToActorDir = Vector3::Normalize(actor->GetPosition());
            actor->SetPosition(originToActorDir * kWalkRadius);

            // Using cross product, we can calculate expected facing direction (looking along movement circle edge).
            Vector3 cross = Vector3::Normalize(Vector3::Cross(Vector3::UnitY, originToActorDir));
            actor->SetForcedFacingDir(cross);
            actor->SetHeading(Heading::FromDirection(cross));
        }
        else
        {
            // When not walking, don't force a facing direction - use what's calculated internally.
            actor->SetForcedFacingDir(Vector3::Zero);
        }
    }
}