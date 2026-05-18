#include "RoboCatPCH.hpp"

Yarn::Yarn() :
	mMuzzleSpeed(300.f),
	mVelocity(Vector3::Zero),
	mPlayerId(0)
{
	// increase visible size
	SetScale(GetScale() * 0.6f);
	SetCollisionRadius(10.f);
}



uint32_t Yarn::Write(OutputMemoryBitStream& inOutputStream, uint32_t inDirtyState) const
{
	uint32_t writtenState = 0;

	if (inDirtyState & EYRS_Pose)
	{
		inOutputStream.Write((bool)true);

		Vector3 location = GetLocation();
		inOutputStream.Write(location.mX);
		inOutputStream.Write(location.mY);

		Vector3 velocity = GetVelocity();
		inOutputStream.Write(velocity.mX);
		inOutputStream.Write(velocity.mY);

		inOutputStream.Write(GetRotation());

		writtenState |= EYRS_Pose;
	}
	else
	{
		inOutputStream.Write((bool)false);
	}

	if (inDirtyState & EYRS_Color)
	{
		inOutputStream.Write((bool)true);

		inOutputStream.Write(GetColor());

		writtenState |= EYRS_Color;
	}
	else
	{
		inOutputStream.Write((bool)false);
	}

	if (inDirtyState & EYRS_PlayerId)
	{
		inOutputStream.Write((bool)true);

		inOutputStream.Write(mPlayerId, 8);

		writtenState |= EYRS_PlayerId;
	}
	else
	{
		inOutputStream.Write((bool)false);
	}
	return writtenState;
}



bool Yarn::HandleCollisionWithCat(RoboCat* inCat)
{
	(void)inCat;

	//you hit a cat, so look like you hit a cat
	return false;
}


void Yarn::InitFromShooter(RoboCat* inShooter)
{
	SetColor(inShooter->GetColor());
	SetPlayerId(inShooter->GetPlayerId());

	Vector3 forward = inShooter->GetTurretForwardVector();
	// set spawn a bit in front of the turret based on shooter's collision radius
	float spawnOffset = inShooter->GetCollisionRadius() + 10.f;
	SetLocation(inShooter->GetLocation() + forward * spawnOffset);

	SetVelocity(inShooter->GetVelocity() + forward * mMuzzleSpeed);

	// compute rotation from forward
	float rad = atan2f(forward.mY, forward.mX);
	float deg = rad * 180.0f / 3.14159265f;
	SetRotation(deg);
}

void Yarn::Update()
{

	float deltaTime = Timing::sInstance.GetDeltaTime();

	SetLocation(GetLocation() + mVelocity * deltaTime);


	//we'll let the cats handle the collisions
}

