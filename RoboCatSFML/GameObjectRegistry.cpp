#include "RoboCatPCH.hpp"


std::unique_ptr< GameObjectRegistry >	GameObjectRegistry::sInstance;

void GameObjectRegistry::StaticInit()
{
	sInstance.reset(new GameObjectRegistry());
}

GameObjectRegistry::GameObjectRegistry()
{
}

void GameObjectRegistry::RegisterCreationFunction(uint32_t inFourCCName, GameObjectCreationFunc inCreationFunction)
{
	mNameToGameObjectCreationFunctionMap[inFourCCName] = inCreationFunction;
}

GameObjectPtr GameObjectRegistry::CreateGameObject(uint32_t inFourCCName)
{
	//an unregistered name used to default construct a null creation function and then call it,
	//which crashed instead of telling the caller anything useful
	auto it = mNameToGameObjectCreationFunctionMap.find(inFourCCName);
	if (it == mNameToGameObjectCreationFunctionMap.end())
	{
		LOG("No creation function registered for object type %d", inFourCCName);
		return nullptr;
	}

	GameObjectCreationFunc creationFunc = it->second;

	GameObjectPtr gameObject = creationFunc();

	//should the registry depend on the world? this might be a little weird
	//perhaps you should ask the world to spawn things? for now it will be like this
	World::sInstance->AddGameObject(gameObject);

	return gameObject;
}