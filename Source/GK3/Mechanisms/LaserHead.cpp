#include "LaserHead.h"

#include <string>

#include "Debug.h"
#include "GameProgress.h"
#include "GKObject.h"
#include "Ray.h"
#include "SceneManager.h"
#include "Vector3.h"

TYPE_DEF_CHILD(Component, LaserHead);

namespace
{
    // The center point from which the heads are positioned (I think this is where the lasers meet).
    // Thankfully, this is known to be correct b/c the original game outputs it into the log.
    Vector3 cs2HeadsCenter(4.02f, 57.0f, 18.90f);

    // The angles for the heads. Index 2 (the middle one) is the default.
    // Also known to be correct b/c the original game outputs them to the log.
    float cs2HeadAngles[5][5] = {
        {  160.57f, -163.08f, -145.32f, -126.91f, -90.95f },
        { -126.72f, -90.69f,  -72.84f,  -54.56f,  -18.47f },
        { -54.56f,  -18.47f,  -0.70f,    16.62f,   53.28f },
        {  17.77f,   53.09f,   71.40f,   89.31f,   125.44f },
        {  89.05f,   125.27f,  143.58f,  161.53f, -162.23f }
    };

    // All known lasers - needed to properly ignore them during raycasts.
    GKObject* lasers[5] = { 0 };
}

LaserHead::LaserHead(Actor* owner, int index) : Component(owner),
    mIndex(index)
{
    // We're assuming the owner is a GKObject here.
    GKObject* gkOwner = static_cast<GKObject*>(owner);

    // Set each head to face the center angle.
    Heading defaultHeading = Heading::FromDegrees(cs2HeadAngles[index][mTurnIndex]);
    gkOwner->SetHeading(defaultHeading);

    // Luckily, we can derive the position of the head from it's initial heading and the heads' center point.
    // The radius is, admittedly, derived from trial and error.
    const float kCircleRadius = 129.5f;
    Vector3 position = cs2HeadsCenter + (defaultHeading.ToDirection() * -kCircleRadius);
    position.y = 52.0f; // y-height of the heads is also derived from trial and error.
    gkOwner->SetPosition(position);

    // Also gotta handle the lasers!
    mLaser = gSceneManager.GetScene()->GetSceneObjectByModelName("cs2laser_0" + std::to_string(index + 1));
    if(mLaser != nullptr)
    {
        // Store laser for later use when raycasting.
        lasers[index] = mLaser;

        // Make the laser a child of the head, so that rotating the head will rotate the laser too.
        // This also makes the laser's match the heads' headings (heh).
        mLaser->GetTransform()->SetParent(gkOwner->GetTransform());

        // The lasers are positioned just a tiny bit closer (smaller radius).
        const float kLaserCircleRadius = 128.0f;
        position = cs2HeadsCenter + (defaultHeading.ToDirection() * -kLaserCircleRadius);
        position.y = cs2HeadsCenter.y; // use exact height this time
        mLaser->SetWorldPosition(position);

        // Scale the laser so it looks correct.
        const float kLaserWidthScale = 0.2f;
        const float kLaserLengthScale = 1.0f;
        mLaser->SetScale(Vector3(kLaserWidthScale, kLaserWidthScale, kLaserLengthScale));
    }

    // Set the game variable so Sheep logic knows what rotation this head is at.
    gGameProgress.SetGameVariable("Cs2Head" + std::to_string(mIndex + 1), mTurnIndex);
}

void LaserHead::SetLaserEnabled(bool enabled)
{
    mLaser->SetActive(enabled);
}

bool LaserHead::IsLaserEnabled() const
{
    return mLaser->IsActive();
}

void LaserHead::TurnLeft()
{
    if(mTurnIndex < 4)
    {
        ++mTurnIndex;
        gGameProgress.SetGameVariable("Cs2Head" + std::to_string(mIndex + 1), mTurnIndex);

        mTurnTimer = 0.0f;
    }
}

void LaserHead::TurnRight()
{
    if(mTurnIndex > 0)
    {
        --mTurnIndex;
        gGameProgress.SetGameVariable("Cs2Head" + std::to_string(mIndex + 1), mTurnIndex);

        mTurnTimer = 0.0f;
    }
}

void LaserHead::OnUpdate(float deltaTime)
{
    // Turn the head if the timer has been set.
    if(mTurnTimer < kTurnDuration)
    {
        mTurnTimer += deltaTime;

        GKObject* gkOwner = static_cast<GKObject*>(GetOwner());
        Heading currentHeading = gkOwner->GetHeading();
        Heading desiredHeading = Heading::FromDegrees(cs2HeadAngles[mIndex][mTurnIndex]);

        // Using lerp to ensure exact to/from behavior.
        Vector3 currentFacing = currentHeading.ToDirection();
        Vector3 desiredFacing = desiredHeading.ToDirection();
        Vector3 newFacing = Vector3::Lerp(currentFacing, desiredFacing, Math::Clamp(mTurnTimer / kTurnDuration, 0.0f, 1.0f));

        gkOwner->SetRotation(Heading::FromDirection(newFacing).ToQuaternion());
    }

    // The laser needs to extend until it hits something (a wall, a prop, a character).
    {
        // Create a ray in the direction the laser is facing.
        // I'm starting the ray 10 units forward to avoid it intersecting with the head models.
        Ray ray(mLaser->GetWorldPosition() + GetOwner()->GetForward() * 10.0f, GetOwner()->GetForward());

        // Do a raycast, see what we hit.
        SceneCastResult result = gSceneManager.GetScene()->Raycast(ray, false, lasers, 5);

        // The lasers are 100 units long by default.
        // Determine whether we need to increase or decrease scale to get the correct effect.
        float distance = (mLaser->GetWorldPosition() - ray.GetPoint(result.hitInfo.t)).GetLength();
        float scaleFactor = distance / 100.0f;

        // Change the laser's z-scale.
        Vector3 scale = mLaser->GetScale();
        scale.z = scaleFactor;
        mLaser->SetScale(scale);
    }
}