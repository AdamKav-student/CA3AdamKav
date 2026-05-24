#include "RoboCatClientPCH.hpp"
#include "AudioManager.hpp"

std::unique_ptr<MenuManager> MenuManager::sInstance;

MenuManager::MenuManager()
	: mCurrentState(MENU_MAIN)
{
	// Initialize background sprite
	TexturePtr backgroundTexture = TextureManager::sInstance->GetTexture("menu_background");
	if (backgroundTexture)
	{
		mBackgroundSprite.setTexture(*backgroundTexture);
		// Scale to fit screen
		float textureWidth = mBackgroundSprite.getLocalBounds().width;
		float textureHeight = mBackgroundSprite.getLocalBounds().height;
		
		if (textureWidth > 0 && textureHeight > 0)
		{
			float scaleX = 1280.0f / textureWidth;
			float scaleY = 720.0f / textureHeight;
			float scale = (scaleX < scaleY) ? scaleX : scaleY;
			mBackgroundSprite.setScale(scale, scale);
			
			// Center the sprite
			float scaledWidth = textureWidth * scale;
			float scaledHeight = textureHeight * scale;
			mBackgroundSprite.setPosition((1280.0f - scaledWidth) / 2.0f, (720.0f - scaledHeight) / 2.0f);
		}
	}

	// Initialize instructions background sprite
	TexturePtr instructionsBackgroundTexture = TextureManager::sInstance->GetTexture("instructions_background");
	if (instructionsBackgroundTexture)
	{
		mInstructionsBackgroundSprite.setTexture(*instructionsBackgroundTexture);
		// Scale to fit screen
		float textureWidth = mInstructionsBackgroundSprite.getLocalBounds().width;
		float textureHeight = mInstructionsBackgroundSprite.getLocalBounds().height;
		
		if (textureWidth > 0 && textureHeight > 0)
		{
			float scaleX = 1280.0f / textureWidth;
			float scaleY = 720.0f / textureHeight;
			float scale = (scaleX < scaleY) ? scaleX : scaleY;
			mInstructionsBackgroundSprite.setScale(scale, scale);
			
			// Center the sprite
			float scaledWidth = textureWidth * scale;
			float scaledHeight = textureHeight * scale;
			mInstructionsBackgroundSprite.setPosition((1280.0f - scaledWidth) / 2.0f, (720.0f - scaledHeight) / 2.0f);
		}
	}

	// Initialize instructions sprite
	TexturePtr instructionsTexture = TextureManager::sInstance->GetTexture("instructions_image");
	if (instructionsTexture)
	{
		mInstructionsSprite.setTexture(*instructionsTexture);
		// Scale to fit screen - scale up if needed
		float textureWidth = mInstructionsSprite.getLocalBounds().width;
		float textureHeight = mInstructionsSprite.getLocalBounds().height;
		
		if (textureWidth > 0 && textureHeight > 0)
		{
			float scaleX = 1200.0f / textureWidth;  // Leave 40px margin
			float scaleY = 650.0f / textureHeight;  // Leave space for button
			float scale = (scaleX < scaleY) ? scaleX : scaleY;
			mInstructionsSprite.setScale(scale, scale);
			
			// Center the sprite
			float scaledWidth = textureWidth * scale;
			float scaledHeight = textureHeight * scale;
			mInstructionsSprite.setPosition((1280.0f - scaledWidth) / 2.0f, (680.0f - scaledHeight) / 2.0f);
		}
	}
}

void MenuManager::StaticInit()
{
	sInstance.reset(new MenuManager());
}

void MenuManager::AddButton(const sf::FloatRect& bounds, const std::string& textureKey, std::function<void()> onClick)
{
	Button button(bounds, textureKey);
	button.onClick = onClick;

	// Load the texture for this button
	TexturePtr buttonTexture = TextureManager::sInstance->GetTexture(textureKey);
	if (buttonTexture)
	{
		button.sprite.setTexture(*buttonTexture);
		button.sprite.setPosition(bounds.left, bounds.top);
		// Scale button image to fit button bounds
		button.sprite.setScale(bounds.width / button.sprite.getLocalBounds().width,
		                        bounds.height / button.sprite.getLocalBounds().height);
	}

	// Add to appropriate menu based on current state
	if (mCurrentState == MENU_MAIN)
	{
		mMainMenuButtons.push_back(button);
	}
	else if (mCurrentState == MENU_INSTRUCTIONS)
	{
		mInstructionsMenuButtons.push_back(button);
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
				AudioManager::Instance().PlaySoundEffect(SoundEffect::ButtonClick); // ADD THIS
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