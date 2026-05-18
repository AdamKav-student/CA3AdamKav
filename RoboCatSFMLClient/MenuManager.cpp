#include "RoboCatClientPCH.hpp"
#include <algorithm>

std::unique_ptr<MenuManager> MenuManager::sInstance;

// recompute layout (implementation)
void MenuManager::UpdateLayout()
{
	if (!WindowManager::sInstance)
		return;

	auto size = WindowManager::sInstance->getSize();
	float screenW = static_cast<float>(size.x);
	float screenH = static_cast<float>(size.y);

	const float baseW = 1280.f;
	const float baseH = 720.f;
	float scaleX = screenW / baseW;
	float scaleY = screenH / baseH;
	float uniformScale = std::min(scaleX, scaleY);

	// background
	TexturePtr bg = TextureManager::sInstance->GetTexture("menu_background");
	if (bg)
	{
		mBackgroundSprite.setTexture(*bg);
		auto tb = mBackgroundSprite.getLocalBounds();
		float sX = screenW / tb.width;
		float sY = screenH / tb.height;
		float s = std::min(sX, sY);
		mBackgroundSprite.setScale(s, s);
		mBackgroundSprite.setPosition((screenW - tb.width * s) / 2.f, (screenH - tb.height * s) / 2.f);
	}

	// instructions background
	TexturePtr ibg = TextureManager::sInstance->GetTexture("instructions_background");
	if (ibg)
	{
		mInstructionsBackgroundSprite.setTexture(*ibg);
		auto tb = mInstructionsBackgroundSprite.getLocalBounds();
		float sX = screenW / tb.width;
		float sY = screenH / tb.height;
		float s = std::min(sX, sY);
		mInstructionsBackgroundSprite.setScale(s, s);
		mInstructionsBackgroundSprite.setPosition((screenW - tb.width * s) / 2.f, (screenH - tb.height * s) / 2.f);
	}

	// instructions image
	TexturePtr inst = TextureManager::sInstance->GetTexture("instructions_image");
	if (inst)
	{
		mInstructionsSprite.setTexture(*inst);
		auto tb = mInstructionsSprite.getLocalBounds();
		float targetW = screenW * 0.94f;
		float targetH = screenH * 0.9f;
		float sX = targetW / tb.width;
		float sY = targetH / tb.height;
		float s = std::min(sX, sY);
		mInstructionsSprite.setScale(s, s);
		mInstructionsSprite.setPosition((screenW - tb.width * s) / 2.f, (screenH - tb.height * s) / 2.f);
	}

	// Update button scaled bounds and sprites
	auto updateButtons = [&](std::vector<Button>& list)
	{
		for (auto& b : list)
		{
			b.bounds.left = b.baseBounds.left * uniformScale;
			b.bounds.top = b.baseBounds.top * uniformScale;
			b.bounds.width = b.baseBounds.width * uniformScale;
			b.bounds.height = b.baseBounds.height * uniformScale;

			if (b.sprite.getTexture())
			{
				b.sprite.setPosition(b.bounds.left, b.bounds.top);
				b.sprite.setScale(b.bounds.width / b.sprite.getLocalBounds().width, b.bounds.height / b.sprite.getLocalBounds().height);
			}
		}
	};

	updateButtons(mMainMenuButtons);
	updateButtons(mInstructionsMenuButtons);
}

MenuManager::MenuManager()
	: mCurrentState(MENU_MAIN)
{
	// Defer layout update to UpdateLayout which will use actual window size
}

void MenuManager::StaticInit()
{
	sInstance.reset(new MenuManager());
	// initial layout update
	sInstance->UpdateLayout();
}

void MenuManager::AddButton(const sf::FloatRect& baseBounds, const std::string& textureKey, std::function<void()> onClick)
{
	Button button(baseBounds, textureKey);
	button.onClick = onClick;

	// Load the texture for this button
	TexturePtr buttonTexture = TextureManager::sInstance->GetTexture(textureKey);
	if (buttonTexture)
	{
		button.sprite.setTexture(*buttonTexture);
	}

	// Add to appropriate menu based on current state
	if (mCurrentState == MENU_MAIN)
	{
		mMainMenuButtons.push_back(button);
		// ensure layout updated
		sInstance->UpdateLayout();
	}
	else if (mCurrentState == MENU_INSTRUCTIONS)
	{
		mInstructionsMenuButtons.push_back(button);
		sInstance->UpdateLayout();
	}
}

void MenuManager::HandleMouseMove(const sf::Vector2f& mousePos)
{
	std::vector<Button>* buttons = nullptr;

	if (mCurrentState == MENU_MAIN)
		buttons = &mMainMenuButtons;
	else if (mCurrentState == MENU_INSTRUCTIONS)
		buttons = &mInstructionsMenuButtons;

	if (buttons)
	{
		for (auto& button : *buttons)
		{
			button.hovered = button.bounds.contains(mousePos);
		}
	}
}

void MenuManager::HandleMouseClick(const sf::Vector2f& mousePos)
{
	std::vector<Button>* buttons = nullptr;

	if (mCurrentState == MENU_MAIN)
		buttons = &mMainMenuButtons;
	else if (mCurrentState == MENU_INSTRUCTIONS)
		buttons = &mInstructionsMenuButtons;

	if (buttons)
	{
		for (auto& button : *buttons)
		{
			if (button.bounds.contains(mousePos))
			{
				button.onClick();
			}
		}
	}
}

void MenuManager::RenderMainMenu()
{
	// Draw background
	WindowManager::sInstance->draw(mBackgroundSprite);

	// Draw buttons
	for (auto& button : mMainMenuButtons)
	{
		WindowManager::sInstance->draw(button.sprite);

		// Add hover effect
		if (button.hovered)
		{
			sf::RectangleShape hoverOverlay(sf::Vector2f(button.bounds.width, button.bounds.height));
			hoverOverlay.setPosition(button.bounds.left, button.bounds.top);
			hoverOverlay.setFillColor(sf::Color(255, 255, 255, 80));
			WindowManager::sInstance->draw(hoverOverlay);
		}
	}
}

void MenuManager::RenderInstructions()
{
	// Draw instructions background
	WindowManager::sInstance->draw(mInstructionsBackgroundSprite);

	// Draw instructions image
	WindowManager::sInstance->draw(mInstructionsSprite);

	// Draw back button on the right side
	for (auto& button : mInstructionsMenuButtons)
	{
		WindowManager::sInstance->draw(button.sprite);

		// Add hover effect
		if (button.hovered)
		{
			sf::RectangleShape hoverOverlay(sf::Vector2f(button.bounds.width, button.bounds.height));
			hoverOverlay.setPosition(button.bounds.left, button.bounds.top);
			hoverOverlay.setFillColor(sf::Color(255, 255, 255, 80));
			WindowManager::sInstance->draw(hoverOverlay);
		}
	}
}

void MenuManager::Render()
{
	// Ensure layout matches current window (handles resize and initialization order)
	UpdateLayout();

	// Clear the back buffer
	WindowManager::sInstance->clear(sf::Color(50, 50, 50, 255));

	if (mCurrentState == MENU_MAIN)
	{
		RenderMainMenu();
	}
	else if (mCurrentState == MENU_INSTRUCTIONS)
	{
		RenderInstructions();
	}

	// Present back buffer
	WindowManager::sInstance->display();
}