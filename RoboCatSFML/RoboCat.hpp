class RoboCat : public GameObject
{
public:
	CLASS_IDENTIFICATION('RCAT', GameObject)

	enum ECatReplicationState
	{
		ECRS_Pose = 1 << 0,
		ECRS_Color = 1 << 1,
		ECRS_PlayerId = 1 << 2,
		ECRS_Health = 1 << 3,

		ECRS_AllState = ECRS_Pose | ECRS_Color | ECRS_PlayerId | ECRS_Health
	};


	//how long a tank has to wait between shots, in seconds. the server is what actually enforces
	//it, but the client needs it as well so it doesn't play a firing sound for a shot that the
	//server is still going to refuse
	static const float kTimeBetweenShots;

	//the most health a tank can have, and what it spawns with. health is replicated in four
	//bits, so this has to stay under 16
	static const int kMaxHealth = 4;

	static	GameObject* StaticCreate() { return new RoboCat(); }

	virtual uint32_t GetAllStateMask()	const override { return ECRS_AllState; }

	virtual	RoboCat* GetAsCat() override { return this; }

	virtual void Update() override;

	void ProcessInput(float inDeltaTime, const InputState& inInputState);
	void SimulateMovement(float inDeltaTime);

	void ProcessCollisions();
	void ProcessCollisionsWithScreenWalls();

	void		SetPlayerId(uint32_t inPlayerId) { mPlayerId = inPlayerId; }
	uint32_t	GetPlayerId()						const { return mPlayerId; }

	void			SetVelocity(const Vector3& inVelocity) { mVelocity = inVelocity; }
	const Vector3& GetVelocity()						const { return mVelocity; }

	virtual uint32_t	Write(OutputMemoryBitStream& inOutputStream, uint32_t inDirtyState) const override;

	float GetTurretRotation() const { return mTurretRotation; }
	Vector3 GetTurretForwardVector() const;

	void SetTurretRotation(float inRot) { mTurretRotation = inRot; } // add setter

protected:
	RoboCat();

private:


	void	AdjustVelocityByThrust(float inDeltaTime);

	Vector3				mVelocity;


	float				mMaxLinearSpeed;
	float				mMaxRotationSpeed;

	//bounce fraction when hitting various things
	float				mWallRestitution;
	float				mCatRestitution;


	uint32_t			mPlayerId;

protected:

	///move down here for padding reasons...

	float				mLastMoveTimestamp;

	float				mThrustDir;
	int					mHealth;

	bool				mIsShooting;

	float			mTurretRotation;
	float			mMaxTurretRotationSpeed;
};

typedef shared_ptr< RoboCat >	RoboCatPtr;

