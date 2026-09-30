#pragma once

#include <functional>
#include <memory>

class EngineSystemPipeline;
class EventManager;
class World;
class WorldSession final


{
public:
    WorldSession(const EngineSystemPipeline& pipeline, std::function<void()> quitRequestHandler,
        std::function<void()> debugModeToggleRequestHandler);
    ~WorldSession();

    WorldSession(const WorldSession&) = delete;
    WorldSession& operator=(const WorldSession&) = delete;
    WorldSession(WorldSession&&) = delete;
    WorldSession& operator=(WorldSession&&) = delete;

    World& GetWorld() const;
    EventManager& GetEvents() const;

private:
    // Reverse destruction order: World first, EventManager second.
    std::unique_ptr<EventManager> events;
    std::unique_ptr<World> world;
};