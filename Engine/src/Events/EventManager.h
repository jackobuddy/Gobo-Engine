#pragma once

#include "../Logger/Logger.h"
#include "Event.h"
#include <map>
#include <typeindex>
#include <list>
#include <memory>
#include <functional>
#include <vector>


// A parent class for Event Callbacks so that it can be stored into a unique pointer. 
// Otherwise, the template class version would not be able to be stored. 
class IEventCallback 
{
public:
	virtual ~IEventCallback() = default; 

	// Executes the Callback Function from its Owning Instance. 
	void Execute(Event& e) { Call(e); };
	virtual const void* GetOwner() const = 0; 

private:
	// Invokes the owning instance's callback function. 
	virtual void Call(Event& e) = 0; 
};

// A wrapper around a function pointer stored into our Handler List. These functions
// are invoked when an appropriate Event is triggered. 
template <typename TOwner, typename TEvent> 
class EventCallback: public IEventCallback
{
private: 
	typedef void (TOwner::* CallbackFunction)(TEvent&);

public:
	EventCallback(TOwner* ownerInstance, CallbackFunction callbackFunction)
	{
		this->ownerInstance = ownerInstance; 
		this->callbackFunction = callbackFunction; 
	}
	virtual ~EventCallback() override = default; 

	// Returns the owner of the Event Callback. 
	const void* GetOwner() const override { return ownerInstance; }

private:
	// The owning instance of the Event Callback. 
	TOwner* ownerInstance; 
	// The function for the owning instance of the Event Callback to invoke. 
	CallbackFunction callbackFunction; 

	virtual void Call(Event& e) override
	{
		std::invoke(callbackFunction, ownerInstance, static_cast<TEvent&>(e)); 
	}
};

// Sets up our list of event callbacks to be saved into our Subscribers map. 
typedef std::list<std::unique_ptr<IEventCallback>> HandlerList; 

// The Event Manger is responsible for handling the core mechanisms of the 
// engine's Event System. Every game instance will have exactly 1 Event Manager. 
class EventManager
{
public:
	EventManager() { Logger::Log("EventManager created. "); };
	~EventManager() { Logger::Log("EventManager destroyed. "); };

	// Clears the subscriber list
	void Reset() { subscribers.clear(); };

	// Subscribe to an Event Type <T>.
	// In our implementation, a listener subscribes to an Event, triggering
	// a callback function when that Event occurs. 
	template <typename TEvent, typename TOwner>
	void SubscribeToEvent(TOwner* ownerInstance, void (TOwner::*callbackFunction)(TEvent&))
	{
		// Ensure we have a valid Handler List before trying to add the subscriber to it. 
		if (not subscribers[typeid(TEvent)].get())
			subscribers[typeid(TEvent)] = std::make_unique<HandlerList>(); 

		// Get our subscriber and add them to the Handler List stored in subscribers. 
		auto subscriber = std::make_unique<EventCallback<TOwner, TEvent>>(ownerInstance, callbackFunction);
		subscribers[typeid(TEvent)]->push_back(std::move(subscriber));
	}; 

	// Unsubscribes an owner from events, used in the case of destruction. 
	void UnsubscribeOwner(const void* owner)
	{
		if (dispatchDepth > 0)
		{
			pendingUnsubscribers.push_back(owner); 
			return; 
		}

		for (auto& [eventType, handlers] : subscribers)
		{
			handlers->remove_if(
				[owner](const std::unique_ptr<IEventCallback>& callback)
				{
					return callback->GetOwner() == owner; 
				}
			);
		}
	}

	// Emits an Event of Type <T>.
	// In our implementation, when an Event is emitted, we go ahead and execute
	// all listener callbacks immediately. 
	template <typename TEvent, typename ...TArgs> 
	void EmitEvent(TArgs&& ...args) 
	{
		const auto found = subscribers.find(typeid(TEvent)); 
		if (found == subscribers.end() or not found->second)
			return; 

		TEvent event(std::forward<TArgs>(args)...);
		++dispatchDepth; 

		for (const auto& handler : *found->second)
			handler->Execute(event); 
		
		--dispatchDepth; 

		if (dispatchDepth == 0)
			ApplyPendingUnsubscribers(); 
	}; 

private:
	// A map that tracks all subscribers, sorted by the type of event. 
	std::map<std::type_index, std::unique_ptr<HandlerList>> subscribers;

	int dispatchDepth = 0; 
	std::vector<const void*> pendingUnsubscribers; 

	// Processes all pending unsubscribe requests. 
	void ApplyPendingUnsubscribers()
	{
		for (const void* owner : pendingUnsubscribers)
		{
			for (auto& [eventType, handlers] : subscribers)
			{
				handlers->remove_if(
					[owner](const std::unique_ptr<IEventCallback>& callback)
					{
						return callback->GetOwner() == owner; 
					}
				);
			}
		}
	
		pendingUnsubscribers.clear(); 
	}
};