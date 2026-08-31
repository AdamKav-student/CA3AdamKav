#include "RoboCatClientPCH.hpp"

PlayerSpriteComponent::PlayerSpriteComponent(GameObject* inGameObject) :
	SpriteComponent(inGameObject)
{}

sf::Sprite& PlayerSpriteComponent::GetSprite()
{
	// Update the sprite based on the game object stuff.
	auto pos = mGameObject->GetLocation();
	auto rot = mGameObject->GetRotation();
	m_sprite.setPosition(pos.mX, pos.mY);
	m_sprite.setRotation(rot);
	//GetAsCat() already tells us this cheaply, and an unchecked dynamic_cast would take the whole
	//client down the first time this component ends up on something that isn't a player
	Vector3 playerColor = mGameObject->GetAsCat() ? mGameObject->GetColor() : Colors::White;
	m_sprite.setColor(sf::Color(playerColor.mX, playerColor.mY, playerColor.mZ, 255));

	return m_sprite;
}
