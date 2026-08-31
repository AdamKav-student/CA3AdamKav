#include "RoboCatClientPCH.hpp"

void ReplicationManagerClient::Read(InputMemoryBitStream& inInputStream)
{
	while (inInputStream.GetRemainingBitCount() >= 32)
	{
		//read the network id...
		int networkId; inInputStream.Read(networkId);

		//only need 2 bits for action...
		uint8_t action; inInputStream.Read(action, 2);

		bool canKeepReading = false;

		switch (action)
		{
		case RA_Create:
			canKeepReading = ReadAndDoCreateAction(inInputStream, networkId);
			break;
		case RA_Update:
			canKeepReading = ReadAndDoUpdateAction(inInputStream, networkId);
			break;
		case RA_Destroy:
			canKeepReading = ReadAndDoDestroyAction(inInputStream, networkId);
			break;
		default:
			//we have no idea how many bits this record used, so the rest of the packet is garbage to us
			LOG("Replication packet contained unknown action %d- ignoring the rest of it", action);
			break;
		}

		if (!canKeepReading)
		{
			return;
		}
	}

}

bool ReplicationManagerClient::ReadAndDoCreateAction(InputMemoryBitStream& inInputStream, int inNetworkId)
{
	//need 4 cc
	uint32_t fourCCName;
	inInputStream.Read(fourCCName);

	//we might already have this object- could happen if our ack of the create got dropped so server resends create request 
	//( even though we might have created )
	GameObjectPtr gameObject = NetworkManagerClient::sInstance->GetGameObject(inNetworkId);
	if (!gameObject)
	{
		//create the object and map it...
		gameObject = GameObjectRegistry::sInstance->CreateGameObject(fourCCName);

		//an unknown class id means we're reading from the wrong place in the stream- bail out instead of
		//calling through a null creation function
		if (!gameObject)
		{
			LOG("Replication packet asked for unknown object type %d- ignoring the rest of it", fourCCName);
			return false;
		}

		gameObject->SetNetworkId(inNetworkId);
		NetworkManagerClient::sInstance->AddNetworkIdToGameObjectMap(gameObject);
	}
	else if (gameObject->GetClassId() != fourCCName)
	{
		//it had really better be the right type- if it isn't, we can't read the state that follows
		LOG("Replication packet re-created object %d as the wrong type- ignoring the rest of it", inNetworkId);
		return false;
	}

	//and read state
	gameObject->Read(inInputStream);

	return true;
}

bool ReplicationManagerClient::ReadAndDoUpdateAction(InputMemoryBitStream& inInputStream, int inNetworkId)
{
	//need object
	GameObjectPtr gameObject = NetworkManagerClient::sInstance->GetGameObject(inNetworkId);

	//the server only sends an update once the create was ack'd, so normally we have this object.
	//we can still miss it though- we destroy locally as soon as the destroy arrives, while an update
	//for the same object can already be on the wire. we don't know how big that update is, so all we
	//can safely do is stop reading this packet rather than dereference nothing.
	if (!gameObject)
	{
		LOG("Replication packet updated unknown object %d- ignoring the rest of it", inNetworkId);
		return false;
	}

	//and read state
	gameObject->Read(inInputStream);

	return true;
}

bool ReplicationManagerClient::ReadAndDoDestroyAction(InputMemoryBitStream& inInputStream, int inNetworkId)
{
	(void)inInputStream;

	//if something was destroyed before the create went through, we'll never get it
	//but we might get the destroy request, so be tolerant of being asked to destroy something that wasn't created
	GameObjectPtr gameObject = NetworkManagerClient::sInstance->GetGameObject(inNetworkId);
	if (gameObject)
	{
		gameObject->SetDoesWantToDie(true);
		NetworkManagerClient::sInstance->RemoveNetworkIdToGameObjectMap(gameObject);
	}

	//a destroy carries no state, so the stream is still aligned either way
	return true;
}
