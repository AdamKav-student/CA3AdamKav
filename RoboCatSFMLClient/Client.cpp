#include "RoboCatClientPCH.hpp"
#include "AudioManager.hpp"          

static void InitAudio()
{
	auto& audio = AudioManager::Instance();
	audio.LoadSoundEffect(SoundEffect::ButtonClick, "Assets/Sound/Confirmation.wav");
	audio.LoadSoundEffect(SoundEffect::PlayerMove, "Assets/Sound/Player_Start.wav");
	audio.LoadSoundEffect(SoundEffect::WeaponFire, "Assets/Sound/Player_Fire.wav");
	audio.LoadSoundEffect(SoundEffect::ItemCollect, "Assets/Sound/Menu ping 1.wav");
	audio.LoadSoundEffect(SoundEffect::HitLight, "Assets/Sound/Damage.wav");
	audio.LoadSoundEffect(SoundEffect::HitHeavy, "Assets/Sound/Damage_2.wav");
	audio.LoadSoundEffect(SoundEffect::Death, "Assets/Sound/Explosion2.wav");
	audio.LoadMusic("Assets/Sound/Menu_Ambience.wav");
	audio.PlayMusic(true);
	audio.SetMusicVolume(50.f);
}

bool Client::StaticInit()
{
	// Create the Client pointer first because it initializes SDL
	Client* client = new Client();
	InputManager::StaticInit();

	WindowManager::StaticInit();
	FontManager::StaticInit();
	TextureManager::StaticInit();
	RenderManager::StaticInit();

	HUD::StaticInit();
	MenuManager::StaticInit();
	InitAudio();

	s_instance.reset(client);

	// Set up menu buttons after everything is initialized
	Client* clientPtr = static_cast<Client*>(s_instance.get());
	clientPtr->InitializeMenuButtons();

	return true;
}

Client::Client()
	: mInMenu(true)
{
	GameObjectRegistry::sInstance->RegisterCreationFunction('RCAT', RoboCatClient::StaticCreate);
	GameObjectRegistry::sInstance->RegisterCreationFunction('MOUS', MouseClient::StaticCreate);
	GameObjectRegistry::sInstance->RegisterCreationFunction('YARN', YarnClient::StaticCreate);

	string destination = StringUtils::GetCommandLineArg(1);
	string name = StringUtils::GetCommandLineArg(2);

	SocketAddressPtr serverAddress = SocketAddressFactory::CreateIPv4FromString(destination);

	NetworkManagerClient::StaticInit(*serverAddress, name);

	//NetworkManagerClient::sInstance->SetSimulatedLatency(0.0f);
}

void Client::InitializeMenuButtons()
{
	// Main Menu Buttons
	// Play button - top left
	MenuManager::sInstance->AddButton(sf::FloatRect(220, 320, 200, 90), "button_play", [this]() {
		StartGame();
		});

	// Info button - top right
	MenuManager::sInstance->AddButton(sf::FloatRect(860, 320, 200, 90), "button_info", [this]() {
		MenuManager::sInstance->SetMenuState(MENU_INSTRUCTIONS);
		});

	// Instructions Menu - Back Button (Play button) on right side
	MenuManager::sInstance->SetMenuState(MENU_INSTRUCTIONS);
	MenuManager::sInstance->AddButton(sf::FloatRect(1160, 660, 100, 50), "button_play", [this]() {
		MenuManager::sInstance->SetMenuState(MENU_MAIN);
		});

	// Set back to main menu state
	MenuManager::sInstance->SetMenuState(MENU_MAIN);
}

void Client::StartGame()
{
	mInMenu = false;
}

void Client::DoFrame()
{
	InputManager::sInstance->Update();

	if (mInMenu)
	{
		// Menu is rendered by MenuManager
		WindowManager::sInstance->clear(sf::Color(100, 149, 237, 255));
		MenuManager::sInstance->Render();
	}
	else
	{
		Engine::DoFrame();

		NetworkManagerClient::sInstance->ProcessIncomingPackets();

		RenderManager::sInstance->Render();

		NetworkManagerClient::sInstance->SendOutgoingPackets();
	}
}

void Client::HandleEvent(sf::Event& p_event)
{
	if (mInMenu)
	{
		switch (p_event.type)
		{
		case sf::Event::MouseButtonPressed:
		{
			sf::Vector2f mousePos(static_cast<float>(p_event.mouseButton.x), static_cast<float>(p_event.mouseButton.y));
			MenuManager::sInstance->HandleMouseClick(mousePos);
			break;
		}
		case sf::Event::MouseMoved:
		{
			sf::Vector2f mousePos(static_cast<float>(p_event.mouseMove.x), static_cast<float>(p_event.mouseMove.y));
			MenuManager::sInstance->HandleMouseMove(mousePos);
			break;
		}
		default:
			break;
		}
	}
	else
	{
		switch (p_event.type)
		{
		case sf::Event::KeyPressed:
			// Allow Escape to quit the application while playing
			if (p_event.key.code == sf::Keyboard::Escape)
			{
				Engine::s_instance->SetShouldKeepRunning(false);
			}
			else
			{
				InputManager::sInstance->HandleInput(EIA_Pressed, p_event.key.code);
			}
			break;
		case sf::Event::KeyReleased:
			InputManager::sInstance->HandleInput(EIA_Released, p_event.key.code);
			break;
		default:
			break;
		}
	}
}

bool Client::PollEvent(sf::Event& p_event)
{
	return WindowManager::sInstance->pollEvent(p_event);
}