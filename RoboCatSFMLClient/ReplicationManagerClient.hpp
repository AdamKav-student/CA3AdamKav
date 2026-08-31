class ReplicationManagerClient
{
public:
	void Read(InputMemoryBitStream& inInputStream);

private:

	//these return false when the stream can no longer be trusted- when that happens we have to stop
	//reading the packet, because everything after the bad record is misaligned garbage
	bool ReadAndDoCreateAction(InputMemoryBitStream& inInputStream, int inNetworkId);
	bool ReadAndDoUpdateAction(InputMemoryBitStream& inInputStream, int inNetworkId);
	bool ReadAndDoDestroyAction(InputMemoryBitStream& inInputStream, int inNetworkId);

};
