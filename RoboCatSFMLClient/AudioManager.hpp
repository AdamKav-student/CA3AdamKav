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

    //mute is local to this client- it silences what you hear and nothing else
    void ToggleMute() { SetMuted(!mMuted); }
    void SetMuted(bool inMuted);
    bool IsMuted() const { return mMuted; }

private:
    AudioManager() = default;

    //everything that sets a volume goes through here so muting can't be undone by a later
    //volume change, and unmuting restores whatever the volumes were
    void ApplyVolumes();

    std::unordered_map<SoundEffect, sf::SoundBuffer> mBuffers;
    std::unordered_map<SoundEffect, sf::Sound>       mSounds;
    sf::Music                                         mMusic;
    float                                             mSFXVolume = 100.f;
    float                                             mMusicVolume = 100.f;
    bool                                              mMuted = false;
};