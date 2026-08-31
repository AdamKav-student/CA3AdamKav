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
	RenderNetworkStats();
}

void HUD::RenderNetworkStats()
{
	//held on Tab, like a scoreboard key in most shooters
	if (!InputManager::sInstance->IsShowingNetworkStats())
	{
		return;
	}

	const DeliveryNotificationManager& dnm = NetworkManagerClient::sInstance->GetDeliveryNotificationManager();

	uint32_t dispatched = dnm.GetDispatchedPacketCount();
	uint32_t delivered = dnm.GetDeliveredPacketCount();
	uint32_t dropped = dnm.GetDroppedPacketCount();

	//these are only meaningful once we've actually sent something
	int deliveredPercent = dispatched > 0 ? static_cast<int>((100 * delivered) / dispatched) : 0;
	int droppedPercent = dispatched > 0 ? static_cast<int>((100 * dropped) / dispatched) : 0;

	float deltaTime = Timing::sInstance.GetDeltaTime();
	int clientFps = deltaTime > 0.f ? static_cast<int>(1.f / deltaTime) : 0;

	//how far behind the server is on our input- the gap between the move it last acknowledged
	//and where our clock is now, plus how many moves are still waiting on an acknowledgement
	float moveLag = Timing::sInstance.GetFrameStartTime() -
		NetworkManagerClient::sInstance->GetLastMoveProcessedByServerTimestamp();
	int pendingMoves = InputManager::sInstance->GetMoveList().GetMoveCount();

	vector< string > lines;
	lines.push_back("SERVER");
	lines.push_back(StringUtils::Sprintf("  tick rate         %d /s", static_cast<int>(NetworkManagerClient::sInstance->GetServerTickRate())));
	lines.push_back(StringUtils::Sprintf("  players           %d", static_cast<int>(ScoreBoardManager::sInstance->GetEntries().size())));
	lines.push_back(StringUtils::Sprintf("  you are player    %d", NetworkManagerClient::sInstance->GetPlayerId()));
	lines.push_back("");
	lines.push_back("SYNCHRONISATION");
	lines.push_back(StringUtils::Sprintf("  round trip        %d ms", static_cast<int>(NetworkManagerClient::sInstance->GetRoundTripTime() * 1000.f)));
	lines.push_back(StringUtils::Sprintf("  server behind by  %d ms", static_cast<int>(moveLag * 1000.f)));
	lines.push_back(StringUtils::Sprintf("  moves unacked     %d", pendingMoves));
	lines.push_back(StringUtils::Sprintf("  simulated latency %d ms", static_cast<int>(NetworkManagerClient::sInstance->GetSimulatedLatency() * 1000.f)));
	lines.push_back("");
	lines.push_back("PACKETS");
	lines.push_back(StringUtils::Sprintf("  sent              %u", dispatched));
	lines.push_back(StringUtils::Sprintf("  delivered         %d%%", deliveredPercent));
	lines.push_back(StringUtils::Sprintf("  dropped           %d%%", droppedPercent));
	lines.push_back(StringUtils::Sprintf("  in                %d B/s", static_cast<int>(NetworkManagerClient::sInstance->GetBytesReceivedPerSecond().GetValue())));
	lines.push_back(StringUtils::Sprintf("  out               %d B/s", static_cast<int>(NetworkManagerClient::sInstance->GetBytesSentPerSecond().GetValue())));
	lines.push_back("");
	lines.push_back("CLIENT");
	lines.push_back(StringUtils::Sprintf("  frame rate        %d fps", clientFps));

	const int characterSize = 22;
	const float lineHeight = 26.f;
	const float panelWidth = 420.f;
	float panelHeight = lines.size() * lineHeight + 24.f;

	sf::Vector2f viewSize = WindowManager::sInstance->getView().getSize();
	float panelX = (viewSize.x - panelWidth) * 0.5f;
	float panelY = 70.f;

	//a dark panel so the text stays readable over whatever is on screen
	sf::RectangleShape panel(sf::Vector2f(panelWidth, panelHeight));
	panel.setPosition(panelX, panelY);
	panel.setFillColor(sf::Color(0, 0, 0, 200));
	panel.setOutlineColor(sf::Color(255, 255, 255, 90));
	panel.setOutlineThickness(2.f);
	WindowManager::sInstance->draw(panel);

	Vector3 origin(panelX + 16.f, panelY + 12.f, 0.f);
	for (const string& line : lines)
	{
		//headings in white, the numbers under them dimmer
		bool isHeading = !line.empty() && line[0] != ' ';
		RenderText(line, origin, isHeading ? Colors::White : Colors::LightBlue, characterSize);
		origin.mY += lineHeight;
	}
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

