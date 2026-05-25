#pragma once

#include <SFML/Graphics.hpp>
#include <vector>
#include <memory>
#include <functional>
#include <string>

enum MenuState
{
    MENU_MAIN,
    MENU_INSTRUCTIONS,
    MENU_TEAM_SELECT
};

enum TankType
{
    TANK_SHERMAN,
    TANK_PANZER
};

struct Button
{
    sf::FloatRect bounds;
    std::function<void()> onClick;
    bool hovered;
    sf::Sprite sprite;
    std::string textureName;

    Button(const sf::FloatRect& b, const std::string& texName = "")
        : bounds(b), hovered(false), textureName(texName) {}
};

class MenuManager
{
public:
    static void StaticInit();
    static std::unique_ptr<MenuManager> sInstance;

    void Render();
    void HandleMouseClick(const sf::Vector2f& mousePos);
    void HandleMouseMove(const sf::Vector2f& mousePos);

    void AddButton(const sf::FloatRect& bounds, const std::string& textureKey, std::function<void()> onClick);
    void SetMenuState(MenuState state) { mCurrentState = state; }
    MenuState GetMenuState() const { return mCurrentState; }
    void SelectTank(TankType inTank) { mSelectedTank = inTank; }
    TankType GetSelectedTank() const { return mSelectedTank; }

private:
    MenuManager();

    std::vector<Button> mMainMenuButtons;
    std::vector<Button> mInstructionsMenuButtons;
    std::vector<Button> mTeamSelectButtons;

    sf::Sprite mBackgroundSprite;
    sf::Sprite mInstructionsBackgroundSprite;
    sf::Sprite mInstructionsSprite;
    sf::Sprite mShermanPreviewSprite;
    sf::Sprite mPanzerPreviewSprite;
    sf::Sprite mMultiplayerIconSprite;

    MenuState mCurrentState;
    TankType mSelectedTank;

    void RenderMainMenu();
    void RenderInstructions();
    void RenderTeamSelect();
};