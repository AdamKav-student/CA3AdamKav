#pragma once
#include <SFML/Audio/Music.hpp>
#include <SFML/Audio/Sound.hpp>
#include <SFML/Audio/SoundBuffer.hpp>
#include <unordered_map>
#include <string>

enum class SoundEffect {
    ButtonClick,
    PlayerMove,
    WeaponFire,
    ItemCollect,
    HitLight,
    HitHeavy,
    Death
};


class AudioManager {
public:
    static AudioManager& Instance();

    void LoadSoundEffect(SoundEffect effect, const std::string& filepath);
    void PlaySoundEffect(SoundEffect effect);

    void PlaySoundEffectLooped(SoundEffect effect);
    void StopSoundEffect(SoundEffect effect);
    bool IsSoundEffectPlaying(SoundEffect effect);

    void LoadMusic(const std::string& filepath);
    void PlayMusic(bool loop = true);
    void StopMusic();
    void SetMusicVolume(float volume);
    void SetSFXVolume(float volume);

private:
    AudioManager() = default;

    std::unordered_map<SoundEffect, sf::SoundBuffer> mBuffers;
    std::unordered_map<SoundEffect, sf::Sound>       mSounds;
    sf::Music                                         mMusic;
    float                                             mSFXVolume = 100.f;
};