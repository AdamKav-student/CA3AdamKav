class InputManager
{
public:
	static void StaticInit();
	static unique_ptr< InputManager >	sInstance;

	void HandleInput(EInputAction inInputAction, int inKeyCode);

	const InputState& GetState()	const { return mCurrentState; }

	//held, not toggled- the network panel shows for as long as Tab is down. this is local to
	//this client, so unlike the movement keys it never goes into a move
	bool IsShowingNetworkStats()	const { return mIsShowingNetworkStats; }

	MoveList& GetMoveList() { return mMoveList; }

	const Move* GetAndClearPendingMove() { auto toRet = mPendingMove; mPendingMove = nullptr; return toRet; }

	void				Update();

private:

	void CheckAndStopTankTracks();
	void PlayFireSoundIfOffCooldown();

	InputState							mCurrentState;

	InputManager();

	bool				IsTimeToSampleInput();
	const Move& SampleInputAsMove();

	MoveList		mMoveList;
	float			mNextTimeToSampleInput;
	float			mTimeOfNextFireSound;
	bool			mIsShowingNetworkStats;
	//key repeat sends KeyPressed over and over while M is held, and we only want one toggle
	bool			mIsMuteKeyDown;
	const Move* mPendingMove;
};

