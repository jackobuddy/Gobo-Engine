#pragma once

#include "../Logger/Logger.h"
#include "Event.h"
#include <algorithm>
#include <cstdint>
#include <functional>
#include <memory>
#include <stdexcept>
#include <typeindex>
#include <unordered_map>
#include <utility>
#include <vector>


// A parent class for Event Callbacks so that it can be stored into a unique pointer. 
// Otherwise, the template class version would not be able to be stored. 
class IEventCallback 
{
public:
	virtual ~IEventCallback() = default; 

	// Executes the Callback Function from its Owning Instance. 
	virtual void Execute(Event&) = 0;
	virtual const void* GetOwner() const = 0; 
	bool active = true;
	std::uint64_t firstEpoch = 0; 
};

// A wrapper around a function pointer stored into our Handler List. These functions
// are invoked when an appropriate Event is triggered. 
template <typename TOwner, typename TEvent> 
class EventCallback final : public IEventCallback
{
	using Callback = void (TOwner::*)(TEvent&);
	TOwner* owner; 
	Callback callback; 

public:
	EventCallback(TOwner* owner, Callback callback):
		owner(owner), callback(callback) {}
	// Returns the owner of the Event Callback. 
	const void* GetOwner() const override { return owner; }
	void Execute(Event& e) override
	{
		std::invoke(callback, owner, static_cast<TEvent&>(e));
	}
};

// The Event Manger is responsible for handling the core mechanisms of the 
// engine's Event System. Every game instance will have exactly 1 Event Manager. 
class EventManager
{
	using List = std::vector<std::shared_ptr<IEventCallback>>;
	std::unordered_map<std::type_index, List> subscribers; 
	unsigned dispatchDepth = 0; 
	std::uint64_t epoch = 0; 
	
	struct DispatchScope
	{
		unsigned& depth;
		explicit DispatchScope(unsigned& value) : depth(value)	{ depth++; }
		~DispatchScope() { --depth; }
	};

public:
	bool IsDispatching() const noexcept { return dispatchDepth != 0;}

	template<class TEvent, class TOwner>
	void SubscribeToEvent(TOwner* owner, void (TOwner::* callback)(TEvent&))
	{
		if (not owner or not callback)
			throw std::invalid_argument("Invalid event listener."); 
		auto record = std::make_shared<EventCallback<TOwner,TEvent>>(owner, callback); 
		record->firstEpoch = epoch + (IsDispatching() ? 1 : 0); 
		subscribers[typeid(TEvent)].push_back(std::move(record)); 
	}

	void UnsubscribeOwner(const void* owner)
	{
		for (auto& [type, list] : subscribers)
		{
			// First mark matching callbacks as inactive.
			for (const auto& record : list)
			{
				if (record->GetOwner() == owner)
					record->active = false;
			}

			// Then remove them AFTER finishing the inner loop.
			std::erase_if(list, [](const auto& record)
				{
					return !record->active;
				});
		}
	}

	template<class TEvent, class... Args>
	void EmitEvent(Args&&... args)
	{
		if (not IsDispatching()) ++epoch; 
		const auto found = subscribers.find(typeid(TEvent));
		if (found == subscribers.end()) return; 
		const List snapshot = found->second; 
		TEvent event(std::forward<Args>(args)...); 
		DispatchScope scope(dispatchDepth); 
		for (const auto& record : snapshot)
		{
			if (record->active and record->firstEpoch <= epoch)
				record->Execute(event); 
		}
	}
};