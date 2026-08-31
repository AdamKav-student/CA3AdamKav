#include "RoboCatServerPCH.hpp"

void ReplicationManagerServer::ReplicateCreate(int inNetworkId, uint32_t inInitialDirtyState)
{
	mNetworkIdToReplicationCommand[inNetworkId] = ReplicationCommand(inInitialDirtyState);
}

void ReplicationManagerServer::ReplicateDestroy(int inNetworkId)
{
	//if we never told this client about the object there's nothing for it to destroy, and using
	//operator[] here would resurrect an entry we already finished with
	auto it = mNetworkIdToReplicationCommand.find(inNetworkId);
	if (it != mNetworkIdToReplicationCommand.end())
	{
		it->second.SetDestroy();
	}
}

void ReplicationManagerServer::RemoveFromReplication(int inNetworkId)
{
	mNetworkIdToReplicationCommand.erase(inNetworkId);
}

void ReplicationManagerServer::SetStateDirty(int inNetworkId, uint32_t inDirtyState)
{
	//only objects we're already replicating can go dirty- anything else would create a command
	//for an object this client has never heard of
	auto it = mNetworkIdToReplicationCommand.find(inNetworkId);
	if (it != mNetworkIdToReplicationCommand.end())
	{
		it->second.AddDirtyState(inDirtyState);
	}
}

void ReplicationManagerServer::HandleCreateAckd(int inNetworkId)
{
	//the create might have been ack'd after we already finished replicating the object away
	auto it = mNetworkIdToReplicationCommand.find(inNetworkId);
	if (it != mNetworkIdToReplicationCommand.end())
	{
		it->second.HandleCreateAckd();
	}
}

void ReplicationManagerServer::Write(OutputMemoryBitStream& inOutputStream, ReplicationManagerTransmissionData* ioTransmissionData)
{
	//run through each replication command and do something...
	for (auto& pair : mNetworkIdToReplicationCommand)
	{
		ReplicationCommand& replicationCommand = pair.second;
		if (replicationCommand.HasDirtyState())
		{
			int networkId = pair.first;

			ReplicationAction action = replicationCommand.GetAction();

			//a create or an update has to ask the object to write its state, so the object has to still
			//be registered. it can die between two writes ( a yarn that hits a cat does exactly that ),
			//and then there's nothing to say about it until the destroy goes out
			if (action != RA_Destroy && !NetworkManagerServer::sInstance->GetGameObject(networkId))
			{
				continue;
			}

			//well, first write the network id...
			inOutputStream.Write(networkId);

			//only need 2 bits for action...
			inOutputStream.Write(action, 2);

			uint32_t writtenState = 0;
			uint32_t dirtyState = replicationCommand.GetDirtyState();

			//now do what?
			switch (action)
			{
			case RA_Create:
				writtenState = WriteCreateAction(inOutputStream, networkId, dirtyState);
				//once the create action is transmitted, future replication
				//of this object should be updates instead of creates
				break;
			case RA_Update:
				writtenState = WriteUpdateAction(inOutputStream, networkId, dirtyState);
				break;
			case RA_Destroy:
				//don't need anything other than state!
				writtenState = WriteDestroyAction(inOutputStream, networkId, dirtyState);
				//add this to the list of replication commands to remove
				break;
			}

			ioTransmissionData->AddTransmission(networkId, action, writtenState);

			//let's pretend everything was written- don't make this too hard
			replicationCommand.ClearDirtyState(writtenState);

		}
	}
}


uint32_t ReplicationManagerServer::WriteCreateAction(OutputMemoryBitStream& inOutputStream, int inNetworkId, uint32_t inDirtyState)
{
	//need object- Write() only gets us here while the object is still registered
	GameObjectPtr gameObject = NetworkManagerServer::sInstance->GetGameObject(inNetworkId);
	//need 4 cc
	inOutputStream.Write(gameObject->GetClassId());
	return gameObject->Write(inOutputStream, inDirtyState);
}

uint32_t ReplicationManagerServer::WriteUpdateAction(OutputMemoryBitStream& inOutputStream, int inNetworkId, uint32_t inDirtyState)
{
	//need object- Write() only gets us here while the object is still registered
	GameObjectPtr gameObject = NetworkManagerServer::sInstance->GetGameObject(inNetworkId);

	//if we can't find the gameObject on the other side, we won't be able to read the written data ( since we won't know which class wrote it )
	//so we need to know how many bytes to skip.


	//this means we need byte sand each new object needs to be byte aligned

	uint32_t writtenState = gameObject->Write(inOutputStream, inDirtyState);

	return writtenState;
}

uint32_t ReplicationManagerServer::WriteDestroyAction(OutputMemoryBitStream& inOutputStream, int inNetworkId, uint32_t inDirtyState)
{
	(void)inOutputStream;
	(void)inNetworkId;
	(void)inDirtyState;
	//don't have to do anything- action already written

	return inDirtyState;
}
