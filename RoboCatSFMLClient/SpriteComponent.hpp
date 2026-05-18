typedef shared_ptr< sf::Texture > TexturePtr;
typedef shared_ptr<sf::Font> FontPtr;

class SpriteComponent
{
public:

	SpriteComponent(GameObject* inGameObject);
	~SpriteComponent();


	void SetTexture(TexturePtr inTexture);
	virtual sf::Sprite& GetSprite();

	void SetDrawOrder(int inOrder) { mDrawOrder = inOrder; }
	int GetDrawOrder() const { return mDrawOrder; }
	
protected:

	sf::Sprite m_sprite;

	//don't want circular reference...
	GameObject* mGameObject;

	int mDrawOrder = 0;
};

typedef shared_ptr< SpriteComponent >	SpriteComponentPtr;

