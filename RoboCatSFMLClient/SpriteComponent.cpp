#include "RoboCatClientPCH.hpp"


SpriteComponent::SpriteComponent(GameObject* inGameObject) :
	mGameObject(inGameObject)
{
	//and add yourself to the rendermanager...
	RenderManager::sInstance->AddComponent(this);
}

SpriteComponent::~SpriteComponent()
{
	//don't render me, I'm dead!
	RenderManager::sInstance->RemoveComponent(this);
}

void SpriteComponent::SetTexture(TexturePtr inTexture)
{
	//TextureManager hands back an empty pointer for a texture that failed to load ( the asset
	//paths are relative, so this happens whenever the working directory isn't what we expect )
	if (!inTexture)
	{
		LOG("Tried to set a texture that isn't loaded- leaving this sprite as it was", 0);
		return;
	}

	auto tSize = inTexture->getSize();
	m_sprite.setTexture(*inTexture);
	m_sprite.setOrigin(tSize.x / 2, tSize.y / 2);
	m_sprite.setScale(sf::Vector2f(1.f * mGameObject->GetScale(), 1.f * mGameObject->GetScale()));

	// if texture indicates a projectile ("yarn"), set draw order low so it's drawn above background
	if (inTexture && inTexture->getSize().x > 0)
	{
		// no-op but kept for extension
	}
}

sf::Sprite& SpriteComponent::GetSprite()
{
	// Update the sprite based on the game object stuff.
	auto pos = mGameObject->GetLocation();
	auto rot = mGameObject->GetRotation();
	m_sprite.setPosition(pos.mX, pos.mY);
	m_sprite.setRotation(rot);

	return m_sprite;
}

