#include "RoboCatClientPCH.hpp"

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
        LoadVictoryImage();
    }
    else if (inState == EGS_Defeat)
    {
        LoadDefeatImage();
    }
}

void GameStateManager::LoadVictoryImage()
{
    TexturePtr victoryTexture = TextureManager::sInstance->GetTexture("victory");
    if (victoryTexture)
    {
        mStateSprite.setTexture(*victoryTexture);
        auto bounds = mStateSprite.getLocalBounds();
        mStateSprite.setOrigin(bounds.width / 2.f, bounds.height / 2.f);

        auto dm = sf::VideoMode::getDesktopMode();
        mStateSprite.setPosition(dm.width / 2.f, dm.height / 2.f);
        mImageLoaded = true;
    }
}

void GameStateManager::LoadDefeatImage()
{
    TexturePtr defeatTexture = TextureManager::sInstance->GetTexture("defeat");
    if (defeatTexture)
    {
        mStateSprite.setTexture(*defeatTexture);
        auto bounds = mStateSprite.getLocalBounds();
        mStateSprite.setOrigin(bounds.width / 2.f, bounds.height / 2.f);

        auto dm = sf::VideoMode::getDesktopMode();
        mStateSprite.setPosition(dm.width / 2.f, dm.height / 2.f);
        mImageLoaded = true;
    }
}

void GameStateManager::Render()
{
    if (mGameState != EGS_Playing && mImageLoaded)
    {
        WindowManager::sInstance->draw(mStateSprite);
    }
}
