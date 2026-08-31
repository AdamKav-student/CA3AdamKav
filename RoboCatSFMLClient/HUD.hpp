//I take care of rendering things!

class HUD
{
public:

	static void StaticInit();
	static std::unique_ptr< HUD >	sInstance;

	void Render();

	void SetPlayerHealth(int inHealth) { mHealth = inHealth; }

private:

	HUD();

	void	RenderBandWidth();
	void	RenderRoundTripTime();
	void	RenderScoreBoard();
	void	RenderHealth();
	void	RenderKillCount();
	void	RenderText(const string& inStr, const Vector3& origin, const Vector3& inColor, int inCharacterSize = 50);
	void	RenderTextCentered(const string& inStr, float inY, const Vector3& inColor, int inCharacterSize);

	Vector3										mBandwidthOrigin;
	Vector3										mRoundTripTimeOrigin;
	Vector3										mScoreBoardOrigin;
	Vector3										mScoreOffset;
	Vector3										mHealthOffset;
	float										mKillCountY;
	int											mHealth;
};



