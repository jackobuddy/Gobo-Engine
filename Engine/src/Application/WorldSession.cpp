#include "WorldSession.h"
#include "EngineSystemPipeline.h"
#include "../ECS/World.h"
#include "../Events/EventManager.h"
#include <stdexcept>
#include <utility>


WorldSession::WorldSession(const EngineSystemPipeline& pipeline, std::function<void()> quitRequestHandler): 
    events(std::make_unique<EventManager>()), world(std::make_unique<World>())
{
    if (!quitRequestHandler)
        throw std::invalid_argument(
            "WorldSession requires a quit request handler.");
    world->SetQuitRequestHandler(std::move(quitRequestHandler));
    pipeline.Install(*world, *events);
}

WorldSession::~WorldSession() = default;
World& WorldSession::GetWorld() const { return *world; }
EventManager& WorldSession::GetEvents() const { return *events; }
