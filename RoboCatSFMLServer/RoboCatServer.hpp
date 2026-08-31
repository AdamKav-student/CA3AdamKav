enum ECatControlType
{
	ESCT_Human,
	ESCT_AI
};

class RoboCatServer : public RoboCat
{
public:
	static GameObjectPtr	StaticCreate() { return NetworkManagerServer::sInstance->RegisterAndReturn(new RoboCatServer()); }
	virtual void HandleDying() override;

	virtual void Update() override;

	void SetCatControlType(ECatControlType inCatControlType) { mCatControlType = inCatControlType; }

	void TakeDamage(int inDamagingPlayerId);

	//returns whether any health was actually restored- false when already at the cap
	bool Heal(int inAmount);

protected:
	RoboCatServer();

private:

	void HandleShooting();

	ECatControlType	mCatControlType;


	float		mTimeOfNextShot;
	float		mTimeBetweenShots;

};

