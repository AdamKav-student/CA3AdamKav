#include "RoboCatClientPCH.hpp"

TurretSpriteComponent::TurretSpriteComponent(GameObject* inGameObject) : SpriteComponent(inGameObject), mOffset(Vector3::Zero), mOriginYOffset(0.f), mOriginAdjusted(false)
{
}

sf::Sprite& TurretSpriteComponent::GetSprite()
{
    // Ensure origin adjusted once after texture is set
    if (!mOriginAdjusted)
    {
        auto tex = m_sprite.getTexture();
        if (tex)
        {
            sf::Vector2u tSize = tex->getSize();
            // default origin already center; lower origin by mOriginYOffset pixels
            m_sprite.setOrigin(tSize.x / 2.f, tSize.y / 2.f + mOriginYOffset);
        }
        mOriginAdjusted = true;
    }

    // Position turret relative to the base game object location
    auto pos = mGameObject->GetLocation();
    float baseRot = mGameObject->GetRotation(); // degrees

    // rotate offset by base rotation
    float rad = baseRot * 3.14159265f / 180.0f;
    float cosr = cosf(rad);
    float sinr = sinf(rad);
    float ox = mOffset.mX * cosr - mOffset.mY * sinr;
    float oy = mOffset.mX * sinr + mOffset.mY * cosr;

    m_sprite.setPosition(pos.mX + ox, pos.mY + oy);

    // use turret rotation from RoboCat if available
    RoboCat* cat = dynamic_cast<RoboCat*>(mGameObject);
    if (cat)
    {
        m_sprite.setRotation(cat->GetTurretRotation());
    }
    else
    {
        m_sprite.setRotation(baseRot);
    }

    return m_sprite;
}
