#include "RoboCatClientPCH.hpp"

std::unique_ptr< HUD >	HUD::sInstance;


HUD::HUD() :
	mScoreBoardOrigin(20.f, 90.f, 0.0f),
	//the bandwidth and round trip readouts are debug information, so they get out of the way of
	//the kill counter and are drawn small
	mBandwidthOrigin(20.f, 12.f, 0.0f),
	mRoundTripTimeOrigin(20.f, 40.f, 0.0f),
	mScoreOffset(0.f, 50.f, 0.0f),
	mHealthOffset(1000.f, 12.f, 0.0f),
	mKillCountY(10.f),
	mHealth(0)
{
}


void HUD::StaticInit()
{
	sInstance.reset(new HUD());
}

void HUD::Render()
{
	RenderBandWidth();
	RenderRoundTripTime();
	RenderScoreBoard();
	RenderHealth();
	RenderKillCount();
}

void HUD::RenderKillCount()
{
	//a player's score is their kill count. we can only show it once the server has welcomed us
	//and told us which player we are
	int playerId = NetworkManagerClient::sInstance->GetPlayerId();

	ScoreBoardManager::Entry* entry = ScoreBoardManager::sInstance->GetEntry(playerId);
	if (!entry)
	{
		return;
	}

	string killCount = StringUtils::Sprintf("KILLS  %d / %d", entry->GetScore(), ScoreBoardManager::kKillsToWin);
	RenderTextCentered(killCount, mKillCountY, Colors::White, 44);
}

void HUD::RenderHealth()
{
	if (mHealth > 0)
	{
		string healthString = StringUtils::Sprintf("Health %d", mHealth);
		RenderText(healthString, mHealthOffset, Colors::Red);
	}
}

void HUD::RenderBandWidth()
{
	string bandwidth = StringUtils::Sprintf("In %d  Bps, Out %d Bps",
		static_cast<int>(NetworkManagerClient::sInstance->GetBytesReceivedPerSecond().GetValue()),
		static_cast<int>(NetworkManagerClient::sInstance->GetBytesSentPerSecond().GetValue()));
	RenderText(bandwidth, mBandwidthOrigin, Colors::White, 22);
}

void HUD::RenderRoundTripTime()
{
	float rttMS = NetworkManagerClient::sInstance->GetAvgRoundTripTime().GetValue() * 1000.f;

	string roundTripTime = StringUtils::Sprintf("RTT %d ms", (int)rttMS);
	RenderText(roundTripTime, mRoundTripTimeOrigin, Colors::White, 22);
}

void HUD::RenderScoreBoard()
{
	const vector< ScoreBoardManager::Entry >& entries = ScoreBoardManager::sInstance->GetEntries();
	Vector3 offset = mScoreBoardOrigin;

	for (const auto& entry : entries)
	{
		RenderText(entry.GetFormattedNameScore(), offset, entry.GetColor());
		offset.mX += mScoreOffset.mX;
		offset.mY += mScoreOffset.mY;
	}

}

void HUD::RenderText(const string& inStr, const Vector3& origin, const Vector3& inColor, int inCharacterSize)
{
	FontPtr font = FontManager::sInstance->GetFont("carlito");
	if (!font)
	{
		return;
	}

	sf::Text text;
	text.setFont(*font);
	text.setString(inStr);
	text.setFillColor(sf::Color(inColor.mX, inColor.mY, inColor.mZ, 255));
	text.setCharacterSize(inCharacterSize);
	text.setPosition(origin.mX, origin.mY);
	WindowManager::sInstance->draw(text);
}

void HUD::RenderTextCentered(const string& inStr, float inY, const Vector3& inColor, int inCharacterSize)
{
	FontPtr font = FontManager::sInstance->GetFont("carlito");
	if (!font)
	{
		return;
	}

	//measure the string so we can sit it in the middle of the view rather than guessing at an x
	sf::Text text;
	text.setFont(*font);
	text.setString(inStr);
	text.setCharacterSize(inCharacterSize);

	float viewWidth = WindowManager::sInstance->getView().getSize().x;
	float textWidth = text.getLocalBounds().width;

	text.setFillColor(sf::Color(inColor.mX, inColor.mY, inColor.mZ, 255));
	text.setPosition((viewWidth - textWidth) * 0.5f, inY);
	WindowManager::sInstance->draw(text);
}

