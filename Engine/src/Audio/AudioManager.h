#pragma once

#include "AudioTypes.h"
#include "../Math/Math.h"
#include <string>
#include <vector>
#include <unordered_map>
#include <SDL3_mixer/SDL_mixer.h>

class AudioManager
{
public:
	AudioManager() = default; 
	~AudioManager() { Shutdown(); } 

	AudioManager(const AudioManager&) = delete; 
	AudioManager& operator=(const AudioManager&) = delete; 

	bool Initialize(); 
	void Update(); 
	void Shutdown(); 

	bool LoadAudio(const std::string& id, const std::string& path, bool predecode = true); 
	void UnloadAudio(const std::string& id);

	bool CreateBus(const std::string& name, const AudioBusConfig& config); 
	bool SetMasterVolume(float volume); 
	void SetBusVolume(const std::string& bus, float volume); 
	void PauseBus(const std::string& bus); 
	void ResumeBus(const std::string& bus); 
	bool IsActiveOnBus(const std::string& audioID, const std::string& bus) const;
	void StopBus(const std::string& bus); 
	void StopAll(); 

	AudioPlaybackID Play(const std::string& audioID, const AudioPlayConfig& config = {});
	AudioPlaybackID PlaySFX(const std::string& id, float volume = 1.0f); 
	AudioPlaybackID PlayMusic(const std::string& id, bool loop = true, float volume = 1.0f); 
	AudioPlaybackID PlayDialogue(const std::string& id, float volume = 1.0f); 
	AudioPlaybackID PlayCustom1(const std::string& id, float volume = 1.0f);
	AudioPlaybackID PlayCustom2(const std::string& id, float volume = 1.0f);
	AudioPlaybackID PlayCustom3(const std::string& id, float volume = 1.0f);

	void Stop(AudioPlaybackID id); 
	void Pause(AudioPlaybackID id); 
	void Resume(AudioPlaybackID id); 

private:
	struct AudioVoice
	{
		MIX_Track* track = nullptr; 
		AudioPlaybackID playbackID = InvalidAudioPlaybackID; 
		std::string audioID; 
		float instanceVolume = 1.0f; 
		std::uint64_t startOrder = 0; 
		bool active = false; 
	};

	struct AudioBusState
	{
		AudioBusConfig config; 
		bool paused = false; 
		std::vector<AudioVoice> voices; 
	};

	AudioVoice* FindVoice(AudioPlaybackID id); 
	AudioVoice* FindVoiceForPlayback(AudioBusState& bus); 
	void ResetVoice(AudioVoice& voice); 
	void RefreshBusGains(AudioBusState& bus); 
	static float ClampVolume(float volume) { return Clamp(volume, 0.0f, 1.0f); }
	static std::string MakeBusTag(const std::string& bus) { return "bus:" + bus; }

	MIX_Mixer* mixer = nullptr; 
	std::unordered_map<std::string, MIX_Audio*> audioAssets; 
	std::unordered_map<std::string, AudioBusState> buses; 
	AudioPlaybackID nextPlaybackID = 1; 
	std::uint64_t nextStartOrder = 1; 
	bool mixerLibraryInitialized = false; 
};