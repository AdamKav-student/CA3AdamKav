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

private:
	MenuManager();

	std::vector<Button> mMainMenuButtons;
	std::vector<Button> mInstructionsMenuButtons;

	sf::Sprite mBackgroundSprite;
	sf::Sprite mInstructionsBackgroundSprite;
	sf::Sprite mInstructionsSprite;
	MenuState mCurrentState;

	void RenderMainMenu();
	void RenderInstructions();
};