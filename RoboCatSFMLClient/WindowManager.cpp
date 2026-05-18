#include "RoboCatClientPCH.hpp"

// forward declaration to update world size at runtime
extern void SetWorldSize(float inWidth, float inHeight);

std::unique_ptr<sf::RenderWindow>	WindowManager::sInstance;

bool WindowManager::StaticInit()
{
	auto dm = sf::VideoMode::getDesktopMode();
	unsigned int w = dm.width;
	unsigned int h = dm.height;
	// Create a window sized to the desktop resolution
	sInstance.reset(new sf::RenderWindow(sf::VideoMode(w, h), "Armoured Warfare 1944 ", sf::Style::Default));

	// update world size to match window
	SetWorldSize(static_cast<float>(w), static_cast<float>(h));

	return true;
}




