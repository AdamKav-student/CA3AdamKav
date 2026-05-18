#pragma once

#include <SFML/Graphics.hpp>
#include <vector>
#include <memory>
#include <functional>
#include <string>

enum MenuState
{
	MENU_MAIN,
	MENU_INSTRUCTIONS
};

struct Button
{
	// baseBounds are the coordinates provided in base 1280x720 layout
	sf::FloatRect baseBounds;
	sf::FloatRect bounds; // scaled bounds for current window
	std::function<void()> onClick;
	bool hovered;
	sf::Sprite sprite;
	std::string textureName;

	Button(const sf::FloatRect& baseB, const std::string& texName = "")
		: baseBounds(baseB), bounds(baseB), onClick(nullptr), hovered(false), textureName(texName) {}
};

class MenuManager
{
public:
	static void StaticInit();
	static std::unique_ptr<MenuManager> sInstance;

	void Render();
	void HandleMouseClick(const sf::Vector2f& mousePos);
	void HandleMouseMove(const sf::Vector2f& mousePos);

	// Add button with base coords (for 1280x720 layout)
	void AddButton(const sf::FloatRect& baseBounds, const std::string& textureKey, std::function<void()> onClick);
	void SetMenuState(MenuState state) { mCurrentState = state; }
	MenuState GetMenuState() const { return mCurrentState; }

private:
	MenuManager();

	std::vector<Button> mMainMenuButtons;
	std::vector<Button> mInstructionsMenuButtons;

	sf::Sprite mBackgroundSprite;
	sf::Sprite mInstructionsBackgroundSprite;
	sf::Sprite mInstructionsSprite;
	MenuState mCurrentState;

	// recompute layout for current window size
	void UpdateLayout();

	void RenderMainMenu();
	void RenderInstructions();
};