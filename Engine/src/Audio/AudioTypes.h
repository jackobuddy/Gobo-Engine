#pragma once

#include <cstddef>
#include <cstdint>
#include <string>

using AudioPlaybackID = std::uint64_t; 
inline constexpr AudioPlaybackID InvalidAudioPlaybackID = 0; 

enum class AudioOverflowPolicy { RejectNew, StopOldest };

struct AudioBusConfig
{
	float volume = 1.0f; 
	std::size_t maxVoices = 16; 
	AudioOverflowPolicy overflow = AudioOverflowPolicy::RejectNew; 
};

struct AudioPlayConfig
{
	std::string bus = "SFX";
	float volume = 1.0f; 
	int loops = 0; // 0 = once, -1 = infinite
};