#include "RoboCatServerPCH.hpp"


MouseServer::MouseServer()
{
}

void MouseServer::HandleDying()
{
	NetworkManagerServer::sInstance->UnregisterGameObject(this);
}


bool MouseServer::HandleCollisionWithCat(RoboCat* inCat)
{
	//a barrel is a pickup, not a kill- only shooting another player scores. we only consume it
	//if it actually healed someone, so driving over one at full health doesn't waste it
	if (static_cast<RoboCatServer*>(inCat)->Heal(1))
	{
		SetDoesWantToDie(true);
	}

	return false;
}






