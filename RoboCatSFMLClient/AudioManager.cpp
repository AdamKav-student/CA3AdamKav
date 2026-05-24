#include "RoboCatClientPCH.hpp" 

AudioManager& AudioManager::Instance() {
	static AudioManager instance;
	return instance;
}

void AudioManager::LoadSoundEffect(SoundEffect effect, const std::string& filepath) {
	sf::SoundBuffer buffer;
	if (!buffer.loadFromFile(filepath))
	{
		LOG("AudioManager: Failed to load sound: %s", filepath.c_str());
		return;
	}
	mBuffers[effect] = std::move(buffer);
	mSounds[effect].setBuffer(mBuffers[effect]);
	mSounds[effect].setVolume(mSFXVolume);
}

void AudioManager::PlaySoundEffect(SoundEffect effect) {
	auto it = mSounds.find(effect);
	if (it != mSounds.end())
		it->second.play();
}

void AudioManager::PlaySoundEffectLooped(SoundEffect effect)
{
	auto it = mSounds.find(effect);
	if (it != mSounds.end())
	{
		it->second.setLoop(true);
		if (it->second.getStatus() != sf::Sound::Playing)
			it->second.play();
	}
}

void AudioManager::StopSoundEffect(SoundEffect effect)
{
	auto it = mSounds.find(effect);
	if (it != mSounds.end())
		it->second.stop();
}

bool AudioManager::IsSoundEffectPlaying(SoundEffect effect)
{
	auto it = mSounds.find(effect);
	if (it != mSounds.end())
		return it->second.getStatus() == sf::Sound::Playing;
	return false;
}

void AudioManager::LoadMusic(const std::string& filepath) {
	if (!mMusic.openFromFile(filepath))
	{
		LOG("AudioManager: Failed to load music: %s", filepath.c_str());
		return;
	}
}

void AudioManager::PlayMusic(bool loop) {
	mMusic.setLoop(loop);
	mMusic.play();
}

void AudioManager::StopMusic() { mMusic.stop(); }

void AudioManager::SetMusicVolume(float volume) { mMusic.setVolume(volume); }

void AudioManager::SetSFXVolume(float volume) {
	mSFXVolume = volume;
	for (auto it = mSounds.begin(); it != mSounds.end(); ++it)
		it->second.setVolume(volume);
}