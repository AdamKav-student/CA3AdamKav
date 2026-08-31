#include "RoboCatClientPCH.hpp"
#include "GameStateManager.hpp"

#include <algorithm>

std::unique_ptr<GameStateManager> GameStateManager::sInstance;

void GameStateManager::StaticInit()
{
    sInstance.reset(new GameStateManager());
}

GameStateManager::GameStateManager() :
    mGameState(EGS_Playing),
    mStateStartTime(0.f),
    mStateDuration(10.f),
    mShouldClose(false),
    mImageLoaded(false)
{
}

void GameStateManager::Update()
{
    if (mGameState == EGS_Playing)
    {
        return;
    }

    // Check if 10 seconds have passed
    float currentTime = Timing::sInstance.GetFrameStartTime();
    float elapsedTime = currentTime - mStateStartTime;

    if (elapsedTime >= mStateDuration)
    {
        mShouldClose = true;
    }
}

void GameStateManager::SetGameState(EGameState inState)
{
    if (mGameState == inState)
    {
        return;
    }

    mGameState = inState;
    mStateStartTime = Timing::sInstance.GetFrameStartTime();
    mImageLoaded = false;

    if (inState == EGS_Victory)
    {
        LoadStateImage("victory");
    }
    else if (inState == EGS_Defeat)
    {
        LoadStateImage("defeat");
    }
}

void GameStateManager::CheckForGameOver()
{
    //first player to the kill goal ends it. once we've decided we stop looking, so a score
    //that keeps climbing afterwards can't flip the result
    if (mGameState != EGS_Playing)
    {
        return;
    }

    int myPlayerId = NetworkManagerClient::sInstance->GetPlayerId();

    for (const ScoreBoardManager::Entry& entry : ScoreBoardManager::sInstance->GetEntries())
    {
        if (entry.GetScore() >= ScoreBoardManager::kKillsToWin)
        {
            SetGameState(static_cast<int>(entry.GetPlayerId()) == myPlayerId ? EGS_Victory : EGS_Defeat);
            return;
        }
    }
}

void GameStateManager::LoadStateImage(const string& inTextureName)
{
    TexturePtr texture = TextureManager::sInstance->GetTexture(inTextureName);
    if (!texture)
    {
        return;
    }

    mStateSprite.setTexture(*texture, true);

    auto bounds = mStateSprite.getLocalBounds();
    mStateSprite.setOrigin(bounds.width / 2.f, bounds.height / 2.f);

    //centre on the view we actually draw into. this used to use the desktop resolution, which
    //put the image wherever the monitor happened to be bigger than the window
    sf::Vector2f viewSize = WindowManager::sInstance->getView().getSize();
    mStateSprite.setPosition(viewSize.x / 2.f, viewSize.y / 2.f);

    //shrink it if it would spill off the view, but never blow a small image up
    if (bounds.width > 0.f && bounds.height > 0.f)
    {
        float scale = std::min(viewSize.x / bounds.width, viewSize.y / bounds.height);
        if (scale < 1.f)
        {
            mStateSprite.setScale(scale, scale);
        }
        else
        {
            mStateSprite.setScale(1.f, 1.f);
        }
    }

    mImageLoaded = true;
}

void GameStateManager::Render()
{
    if (mGameState != EGS_Playing && mImageLoaded)
    {
        WindowManager::sInstance->draw(mStateSprite);
    }
}
