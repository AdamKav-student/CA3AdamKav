#pragma once

#include <memory>
#include <SFML/Graphics.hpp>

enum EGameState
{
    EGS_Playing,
    EGS_Victory,
    EGS_Defeat
};

class GameStateManager
{
public:
    static void StaticInit();
    static std::unique_ptr<GameStateManager> sInstance;

    void Update();
    void Render();

    //every client gets the same replicated scoreboard, so they all reach the same verdict
    //without the server having to tell them separately
    void CheckForGameOver();

    void SetGameState(EGameState inState);
    EGameState GetGameState() const { return mGameState; }

    bool ShouldCloseApplication() const { return mShouldClose; }

private:
    GameStateManager();

    void LoadStateImage(const string& inTextureName);

    EGameState mGameState;
    float mStateStartTime;
    float mStateDuration; // 10 seconds
    bool mShouldClose;

    sf::Sprite mStateSprite;
    bool mImageLoaded;
};

typedef std::unique_ptr<GameStateManager> GameStateManagerPtr;
