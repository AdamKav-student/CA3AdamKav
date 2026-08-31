#include "RoboCatClientPCH.hpp"

std::unique_ptr<FontManager> FontManager::sInstance;

void FontManager::StaticInit()
{
	sInstance.reset(new FontManager());
}

FontManager::FontManager()
{
	//the HUD draws with "carlito", which nothing was caching
	CacheFont("carlito", "../Assets/Carlito-Regular.ttf");

	//the file is BOMBARD_.ttf- BOMBARDMENT.ttf doesn't exist, so this never loaded
	CacheFont("BOMBARDMENT", "../Assets/BOMBARD_.ttf");
}

FontPtr FontManager::GetFont(const string& p_fontName)
{
	return mNameToFontMap[p_fontName];
}

bool FontManager::CacheFont(string inName, const char* inFileName)
{
	FontPtr newFont(new sf::Font());
	if (!newFont->loadFromFile(inFileName))
	{
		//without this a missing font is silent, and every piece of text just fails to draw
		LOG("FontManager: Failed to load font: %s", inFileName);
		return false;
	}

	mNameToFontMap[inName] = newFont;
	return true;
}

