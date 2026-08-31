#include "RoboCatClientPCH.hpp"

std::unique_ptr< TextureManager >		TextureManager::sInstance;

void TextureManager::StaticInit()
{
	sInstance.reset(new TextureManager());
}

TextureManager::TextureManager()
{
	//Model for Allied Team Chassis and Turret
	CacheTexture("sherman", "../Assets/Sherman B.png");
	CacheTexture("sherman_turret", "../Assets/Sherman T.png");
	

	//Model for Axis Team Chassis and Turret
	CacheTexture("panzer", "../Assets/Panzer IV B.png");
	CacheTexture("panzer_turret", "../Assets/Panzer IV T.png");

	//Collectable - planned to restore Health 
	CacheTexture("mouse", "../Assets/OilDrum.png");

	//Projectile
	CacheTexture("yarn", "../Assets/Bullet.png");

	CacheTexture("background", "../Assets/background.png");

	//Menu backgrounds
	CacheTexture("menu_background", "../Assets/Menu New.png");
	CacheTexture("instructions_background", "../Assets/Settings new.png");

	//Menu button images
	CacheTexture("button_play", "../Assets/Play.png");
	CacheTexture("button_info", "../Assets/Question.png");
	CacheTexture("multiplayer", "../Assets/Multiplayer.png");

	//Instructions/Rules/Controls image
	CacheTexture("instructions_image", "../Assets/Control text.png");

	//Victory/Defeat screens
	CacheTexture("victory", "../Assets/Victory Image.png");
	CacheTexture("defeat", "../Assets/Defeat Image.png");
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
