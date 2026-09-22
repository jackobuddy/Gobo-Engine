#include "AudioManager.h"
#include "../Logger/Logger.h"
#include "../Math/Math.h"
#include <algorithm>
#include <utility>


bool AudioManager::Initialize()
{
	if (mixer) return true; 
	if (not MIX_Init()) { Logger::SDLError("Error during MIX_Init", SDL_GetError()); return false; }
	mixerLibraryInitialized = true; 

	mixer = MIX_CreateMixerDevice(SDL_AUDIO_DEVICE_DEFAULT_PLAYBACK, nullptr);
	if (not mixer) 
	{ 
		Logger::SDLError("Failed to initialize Mixer Device", SDL_GetError()); 
		MIX_Quit();
		mixerLibraryInitialized = false; 
		return false; 
	}

	const bool busesCreated = 
		CreateBus("Music", {1.0f, 2, AudioOverflowPolicy::RejectNew}) and 
		CreateBus("SFX", {1.0f, 32, AudioOverflowPolicy::StopOldest}) and
		CreateBus("Dialogue", {1.0f, 4, AudioOverflowPolicy::RejectNew}) and
		CreateBus("Custom1", { 1.0f, 8, AudioOverflowPolicy::StopOldest }) and
		CreateBus("Custom2", { 1.0f, 8, AudioOverflowPolicy::StopOldest }) and 
		CreateBus("Custom3", { 1.0f, 8, AudioOverflowPolicy::StopOldest });

	if (not busesCreated)
	{
		Logger::Error("Failed to create the default audio buses.");
		Shutdown(); 
		return false; 
	}

	Logger::Log("Audio Manager Initialized successfully."); 
	return true; 
}

void AudioManager::Update()
{
	for (auto& [busName, bus] : buses)
	{
		for (AudioVoice& voice : bus.voices)
		{
			if (voice.active and not MIX_TrackPlaying(voice.track) and not MIX_TrackPaused(voice.track))
				ResetVoice(voice); 
		}
	}
}

void AudioManager::Shutdown()
{
	if (mixer)
	{
		MIX_StopAllTracks(mixer, 0); 

		for (auto& [busName, bus] : buses)
		{
			for (AudioVoice& voice : bus.voices)
			{
				if (voice.track)
				{
					MIX_DestroyTrack(voice.track); 
					voice.track = nullptr; 
				}
			}
		}
		buses.clear(); 

		for (auto& [id, audio] : audioAssets)
			MIX_DestroyAudio(audio); 
		audioAssets.clear(); 

		MIX_DestroyMixer(mixer); 
		mixer = nullptr; 
	}

	if (mixerLibraryInitialized)
	{
		MIX_Quit(); 
		mixerLibraryInitialized = false; 
	}
}

bool AudioManager::LoadAudio(const std::string& id, const std::string& path, bool predecode)
{
	if (not mixer)
	{
		Logger::Error("Cannot load audio before AudioManager is initialized.");
		return false; 
	}
	if (id.empty() or path.empty())
	{
		Logger::Error("Audio ID and path must not be empty."); 
		return false; 
	}
	if (audioAssets.contains(id))
	{
		Logger::Error("Audio ID already exists: " + id); 
		return false; 
	}

	MIX_Audio* audio = MIX_LoadAudio(mixer, path.c_str(), predecode); 
	if (not audio)
	{
		Logger::SDLError("Failed to load audio" + path, SDL_GetError());
		return false; 
	}

	audioAssets.emplace(id, audio); 
	return true; 
}

void AudioManager::UnloadAudio(const std::string& id)
{
	auto audioIt = audioAssets.find(id); 
	if (audioIt == audioAssets.end())
		return; 

	for (auto& [busName, bus] : buses)
	{
		for (AudioVoice& voice : bus.voices)
		{
			if (voice.active and voice.audioID == id)
			{
				MIX_StopTrack(voice.track, 0); 
				ResetVoice(voice); 
			}
		}
	}

	MIX_DestroyAudio(audioIt->second); 
	audioAssets.erase(audioIt); 
}

bool AudioManager::CreateBus(const std::string& name, const AudioBusConfig& config)
{
	if (not mixer or name.empty() or config.maxVoices == 0) return false; 
	if (buses.contains(name)) return false; 

	AudioBusState bus; 
	bus.config = config; 
	bus.config.volume = ClampVolume(config.volume); 
	bus.voices.reserve(config.maxVoices); 

	const std::string tag = MakeBusTag(name); 

	for (std::size_t i = 0; i < config.maxVoices; ++i)
	{
		MIX_Track* track = MIX_CreateTrack(mixer); 

		if (not track or not MIX_TagTrack(track, tag.c_str()))
		{
			if (track) MIX_DestroyTrack(track); 
			for (AudioVoice& voice : bus.voices) MIX_DestroyTrack(voice.track); 
			Logger::Error("Failed to create audio bus: " + name);
			return false; 
		}
		AudioVoice voice;
		voice.track = track; 
		bus.voices.push_back(voice);
	}

	buses.emplace(name, std::move(bus)); 
	Logger::Log("Created audio bus: " + name); 
	return true; 
}

bool AudioManager::SetMasterVolume(float volume)
{
	if (not mixer)
		return false; 
	return MIX_SetMixerGain(mixer, ClampVolume(volume)); 
}

void AudioManager::SetBusVolume(const std::string& bus, float volume)
{
	auto busIt = buses.find(bus); 
	if (busIt == buses.end())
	{
		Logger::Error("Audio bus was not found: " + bus); 
		return; 
	}

	busIt->second.config.volume = ClampVolume(volume); 
	RefreshBusGains(busIt->second); 
}

void AudioManager::PauseBus(const std::string& bus)
{
	auto busIt = buses.find(bus); 
	if (busIt == buses.end())
		return; 

	MIX_PauseTag(mixer, MakeBusTag(bus).c_str());
	busIt->second.paused = true; 
}

void AudioManager::ResumeBus(const std::string& bus)
{
	auto busIt = buses.find(bus); 
	if (busIt == buses.end())
		return; 

	MIX_ResumeTag(mixer, MakeBusTag(bus).c_str()); 
	busIt->second.paused = false; 
}

void AudioManager::StopBus(const std::string& bus)
{
	auto busIt = buses.find(bus); 
	if (busIt == buses.end())
		return; 

	MIX_StopTag(mixer, MakeBusTag(bus).c_str(), 0); 
	for (AudioVoice& voice: busIt->second.voices)
		ResetVoice(voice); 
	busIt->second.paused = false; 
}

AudioPlaybackID AudioManager::Play(const std::string& audioID, const AudioPlayConfig& config)
{
	auto audioIt = audioAssets.find(audioID); 
	if (audioIt == audioAssets.end())
	{
		Logger::Error("Audio asset was not found: " + audioID); 
		return InvalidAudioPlaybackID;
	}

	auto busIt = buses.find(config.bus);
	if (busIt == buses.end())
	{
		Logger::Error("Audio bus was not found: " + config.bus); 
		return InvalidAudioPlaybackID; 
	}

	if (config.loops < -1)
	{
		Logger::Error("Audio loops must be -1 or greater.");
		return InvalidAudioPlaybackID; 
	}

	AudioBusState& bus = busIt->second; 
	AudioVoice* voice = FindVoiceForPlayback(bus); 

	if (not voice) return InvalidAudioPlaybackID; 

	const float instanceVolume = ClampVolume(config.volume); 
	if (not MIX_SetTrackAudio(voice->track, audioIt->second) or
		not MIX_SetTrackGain(voice->track, bus.config.volume * instanceVolume))
	{
		Logger::SDLError("Failed to prepare audio track", SDL_GetError()); 
		ResetVoice(*voice); 
		return InvalidAudioPlaybackID; 
	}

	SDL_PropertiesID properties = SDL_CreateProperties();
	if (properties == 0 or not SDL_SetNumberProperty(properties, MIX_PROP_PLAY_LOOPS_NUMBER, config.loops))
	{
		Logger::SDLError("Failed to create audio playback properties", SDL_GetError()); 
		if (properties != 0)
			SDL_DestroyProperties(properties); 
		ResetVoice(*voice); 
		return InvalidAudioPlaybackID;
	}

	const bool started = MIX_PlayTrack(voice->track, properties); 
	SDL_DestroyProperties(properties); 

	if (not started)
	{
		Logger::SDLError("Failed to play audio: " + audioID, SDL_GetError()); 
		ResetVoice(*voice); 
		return InvalidAudioPlaybackID;
	}

	voice->playbackID = nextPlaybackID; 
	if (nextPlaybackID == InvalidAudioPlaybackID) ++nextPlaybackID; 
	voice->audioID = audioID; 
	voice->instanceVolume = instanceVolume; 
	voice->startOrder = nextStartOrder++; 
	voice->active = true; 
	return voice->playbackID;
}

AudioPlaybackID AudioManager::PlaySFX(const std::string& id, float volume)
{
	return Play(id, { .bus = "SFX", .volume = volume, .loops = 0 });
}

AudioPlaybackID AudioManager::PlayMusic(const std::string& id, bool loop, float volume)
{
	return Play(id, { .bus = "Music", .volume = volume, .loops = loop ? -1 : 0 });
}

AudioPlaybackID AudioManager::PlayDialogue(const std::string& id, float volume)
{
	return Play(id, { .bus = "Dialogue", .volume = volume, .loops = 0 });
}

AudioPlaybackID AudioManager::PlayCustom1(const std::string& id, float volume)
{
	return Play(id, { .bus = "Custom1", .volume = volume, .loops = 0 });
}

AudioPlaybackID AudioManager::PlayCustom2(const std::string& id, float volume)
{
	return Play(id, { .bus = "Custom2", .volume = volume, .loops = 0 });
}

AudioPlaybackID AudioManager::PlayCustom3(const std::string& id, float volume)
{
	return Play(id, { .bus = "Custom3", .volume = volume, .loops = 0 });
}

void AudioManager::Stop(AudioPlaybackID id)
{
	AudioVoice* voice = FindVoice(id); 
	if (not voice) return;
	MIX_StopTrack(voice->track, 0); 
	ResetVoice(*voice); 
}

void AudioManager::Pause(AudioPlaybackID id)
{
	AudioVoice* voice = FindVoice(id); 
	if (voice) MIX_PauseTrack(voice->track); 
}

void AudioManager::Resume(AudioPlaybackID id)
{
	AudioVoice* voice = FindVoice(id); 
	if (voice) MIX_ResumeTrack(voice->track); 
}

AudioManager::AudioVoice* AudioManager::FindVoice(AudioPlaybackID id)
{
	if (id == InvalidAudioPlaybackID)
		return nullptr;

	for (auto& [busName, bus] : buses)
	{
		for (AudioVoice& voice : bus.voices)
		{
			if (voice.active and voice.playbackID == id)
				return &voice; 
		}
	}
	return nullptr; 
}

AudioManager::AudioVoice* AudioManager::FindVoiceForPlayback(AudioBusState& bus)
{
	for (AudioVoice& voice : bus.voices)
	{
		if (not voice.active) return &voice; 
	}

	if (bus.config.overflow == AudioOverflowPolicy::RejectNew) return nullptr; 

	auto oldest = std::min_element(bus.voices.begin(), bus.voices.end(), 
		[](const AudioVoice& left, const AudioVoice& right) { return left.startOrder < right.startOrder; });

	if (oldest == bus.voices.end()) return nullptr; 

	MIX_StopTrack(oldest->track, 0); 
	ResetVoice(*oldest); 
	return &*oldest; 
}

void AudioManager::ResetVoice(AudioVoice& voice)
{
	if (voice.track)
		MIX_SetTrackAudio(voice.track, nullptr); 
	voice.playbackID = InvalidAudioPlaybackID; 
	voice.audioID.clear(); 
	voice.instanceVolume = 1.0f; 
	voice.startOrder = 0; 
	voice.active = false; 
}

void AudioManager::RefreshBusGains(AudioBusState& bus)
{
	for (AudioVoice& voice : bus.voices)
		MIX_SetTrackGain(voice.track, bus.config.volume * voice.instanceVolume); 
}
