#include "RoboCatClientPCH.hpp"

std::unique_ptr< RenderManager >	RenderManager::sInstance;

RenderManager::RenderManager()
{
	// make the view match desktop resolution
	auto dm = sf::VideoMode::getDesktopMode();
	view.reset(sf::FloatRect(0, 0, static_cast<float>(dm.width), static_cast<float>(dm.height)));
	WindowManager::sInstance->setView(view);
	
	// Initialize background sprite
	TexturePtr backgroundTexture = TextureManager::sInstance->GetTexture("background");
	if (backgroundTexture)
	{
		mBackgroundSprite.setTexture(*backgroundTexture);
		mBackgroundSprite.setPosition(0, 0);
		// scale background to fit
		auto bounds = mBackgroundSprite.getLocalBounds();
		float scaleX = static_cast<float>(dm.width) / bounds.width;
		float scaleY = static_cast<float>(dm.height) / bounds.height;
		float scale = (scaleX < scaleY) ? scaleX : scaleY;
		mBackgroundSprite.setScale(scale, scale);
	}
}


void RenderManager::StaticInit()
{
	sInstance.reset(new RenderManager());
}


void RenderManager::AddComponent(SpriteComponent* inComponent)
{
	mComponents.emplace_back(inComponent);
}

void RenderManager::RemoveComponent(SpriteComponent* inComponent)
{
	int index = GetComponentIndex(inComponent);

	if (index != -1)
	{
		int lastIndex = mComponents.size() - 1;
		if (index != lastIndex)
		{
			mComponents[index] = mComponents[lastIndex];
		}
		mComponents.pop_back();
	}
}

int RenderManager::GetComponentIndex(SpriteComponent* inComponent) const
{
	for (int i = 0, c = mComponents.size(); i < c; ++i)
	{
		if (mComponents[i] == inComponent)
		{
			return i;
		}
	}

	return -1;
}


//this part that renders the world is really a camera-
//in a more detailed engine, we'd have a list of cameras, and then render manager would
//render the cameras in order
void RenderManager::RenderComponents()
{
	// sort components by draw order so background stays behind projectiles etc.
	sort(mComponents.begin(), mComponents.end(), [](SpriteComponent* a, SpriteComponent* b) {
		return a->GetDrawOrder() < b->GetDrawOrder();
	});

	//Get the logical viewport so we can pass this to the SpriteComponents when it's draw time
	for (SpriteComponent* c : mComponents)
	{	
		WindowManager::sInstance->draw(c->GetSprite());	
	}
}

void RenderManager::Render()
{
	//
	// Clear the back buffer
	//
	WindowManager::sInstance->clear(sf::Color(100, 149, 237, 255));

	// Draw background first`
	WindowManager::sInstance->draw(mBackgroundSprite);

	RenderManager::sInstance->RenderComponents();

	HUD::sInstance->Render();

	//
	// Present our back buffer to our front buffer
	//
	WindowManager::sInstance->display();

}

sf::FloatRect RenderManager::GetBackgroundBounds() const
{
	return mBackgroundSprite.getGlobalBounds();
}
