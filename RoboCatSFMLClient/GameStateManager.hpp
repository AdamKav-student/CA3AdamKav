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

    void SetGameState(EGameState inState);
    EGameState GetGameState() const { return mGameState; }

    bool ShouldCloseApplication() const { return mShouldClose; }

private:
    GameStateManager();

    void LoadVictoryImage();
    void LoadDefeatImage();

    EGameState mGameState;
    float mStateStartTime;
    float mStateDuration; // 10 seconds
    bool mShouldClose;

    sf::Sprite mStateSprite;
    bool mImageLoaded;
};

typedef std::unique_ptr<GameStateManager> GameStateManagerPtr;
