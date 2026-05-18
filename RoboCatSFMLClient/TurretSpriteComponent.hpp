#pragma once

class TurretSpriteComponent : public SpriteComponent
{
public:
    TurretSpriteComponent(GameObject* inGameObject);
    virtual sf::Sprite& GetSprite() override;

    void SetOffset(const Vector3& inOffset) { mOffset = inOffset; }
    void SetOriginYOffset(float inYOffset) { mOriginYOffset = inYOffset; mOriginAdjusted = false; }

private:
    Vector3 mOffset;
    float mOriginYOffset = 0.f;
    bool mOriginAdjusted = false;
};
