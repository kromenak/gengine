//
// Mover.cpp
//
// Clark Kromenaker
//
#include "Mover.h"

#include "Actor.h"

TYPE_DEF_CHILD(Component, Mover);

Mover::Mover(Actor* owner) : Component(owner)
{
	
}

void Mover::UpdateInternal(float deltaTime)
{
	mTimer += deltaTime * mDirection;
	if(mTimer > 5.0f)
	{
		mDirection = -1.0f;
	}
	if(mTimer < 0.0f)
	{
		mDirection = 1.0f;
	}
	
	float t = mTimer / 5.0f;
	
	Vector3 fromPos(0.0f, 0.0f, 0.0f);
	Vector3 toPos(1024.0f, 768.0f, 0.0f);
	Vector3 pos = Vector3::Lerp(fromPos, toPos, t);
	
	Vector3 fromScale(0.5f, 0.5f, 0.5f);
	Vector3 toScale(1.5f, 1.5f, 1.5f);
	Vector3 scale = Vector3::Lerp(fromScale, toScale, t);
	
	Quaternion fromRotation(Vector3::UnitZ, 0.0f);
	Quaternion toRotation(Vector3::UnitZ, 90.0f);
	Quaternion rotation;
	Quaternion::Slerp(rotation, fromRotation, toRotation, t);
	
	mOwner->SetPosition(pos);
	mOwner->SetScale(scale);
	mOwner->SetRotation(rotation);
}
