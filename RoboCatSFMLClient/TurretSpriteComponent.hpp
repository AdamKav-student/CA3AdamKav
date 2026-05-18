#pragma once

class TurretSpriteComponent : public SpriteComponent
{
public:
    TurretSpriteComponent(GameObject* inGameObject);
    virtual sf::Sprite& GetSprite() override;

    void SetOffset(const Vector3& inOffset) { mOffset = inOffset; }

private:
    Vector3 mOffset;
};
