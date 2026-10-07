#pragma once

#include "Module.h"
#include <concepts>
#include <cstddef>
#include <memory>
#include <typeindex>
#include <typeinfo>
#include <utility>
#include <vector>

class GameContext;
struct ModuleServices;

class ModuleManager final
{
public:
	ModuleManager() = default; 
	~ModuleManager();

	ModuleManager(const ModuleManager&) = delete; 
	ModuleManager& operator=(const ModuleManager&) = delete; 

	template <class T, class... Args> 
	requires std::derived_from<T, Module> and std::constructible_from<T, Args...>
	bool Add(Args&&... args)
	{
		if (not registrationOpen or Get<T>() != nullptr) return false; 
		entries.push_back({std::type_index(typeid(T)), 
			std::make_unique<T>(std::forward<Args>(args)...)});
		return true; 
	}

	template<class T> requires std::derived_from<T, Module>
	T* Get()
	{
		for (auto& entry : entries)
			if (entry.type == std::type_index(typeid(T)))
				return static_cast<T*>(entry.module.get()); 
		return nullptr; 
	}

	template<class T> requires std::derived_from<T, Module>
	const T* Get() const
	{
		for (const auto& entry : entries)
			if (entry.type == std::type_index(typeid(T))) 
				return static_cast<const T*>(entry.module.get());
		return nullptr; 
	}

	template <class T> requires std::derived_from<T, Module>
	bool Has() const { return Get<T>() != nullptr; }

	bool StartAll(ModuleServices& servies); 
	bool EnterSceneAll(GameContext& context); 
	void UpdateAll(GameContext& context, float deltaSeconds); 
	void RenderAll(GameContext& context); 
	void LeaveSceneAll(GameContext& context) noexcept; 
	void ShutdownAll() noexcept; 

private:
	struct Entry
	{
		std::type_index type;
		std::unique_ptr<Module> module; 
	};

	std::vector<Entry> entries; 
	std::size_t startedCount = 0; 
	std::size_t attachedCount = 0; 
	bool registrationOpen = true; 
	bool running = false; 
	bool sceneAttached = false; 
};