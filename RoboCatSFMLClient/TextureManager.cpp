#include "RoboCatClientPCH.hpp"

std::unique_ptr< TextureManager >		TextureManager::sInstance;

void TextureManager::StaticInit()
{
	sInstance.reset(new TextureManager());
}

TextureManager::TextureManager()
{
	//Model for Allied Team Chassis and Turret
	CacheTexture("cat", "../Assets/Sherman B.png");
	//CacheTexture("cat", "../Assets/Sherman T.png");

	//Model for Axis Team Chassis and Turret
	//CacheTexture("cat", "../Assets/Panzer IV B.png");
	//CacheTexture("cat", "../Assets/Panzer IV T.png");

	//Collectable - planned to restore Health 
	CacheTexture("mouse", "../Assets/OilDrum.png");

	//Projectile
	CacheTexture("yarn", "../Assets/missile.png");

	CacheTexture("background", "../Assets/background.png");

	//Menu background
	CacheTexture("menu_background", "../Assets/Headless.png");
}

TexturePtr	TextureManager::GetTexture(const string& inTextureName)
{
	return mNameToTextureMap[inTextureName];
}

bool TextureManager::CacheTexture(string inTextureName, const char* inFileName)
{
	TexturePtr newTexture(new sf::Texture());
	if (!newTexture->loadFromFile(inFileName))
	{
		return false;
	}

	mNameToTextureMap[inTextureName] = newTexture;

	return true;

}
