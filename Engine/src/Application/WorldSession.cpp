#include "WorldSession.h"
#include "EngineSystemPipeline.h"
#include "../ECS/World.h"
#include "../Events/EventManager.h"
#include <stdexcept>
#include <utility>


WorldSession::WorldSession(const EngineSystemPipeline& pipeline, std::function<void()> quitRequestHandler,
    std::function<void()> debugModeToggleRequestHandler): events(std::make_unique<EventManager>()),
    world(std::make_unique<World>())
{
    if (!quitRequestHandler || !debugModeToggleRequestHandler)
    {
        throw std::invalid_argument(
            "WorldSession requires request handlers.");
    }

    world->SetQuitRequestHandler(std::move(quitRequestHandler));
    world->SetDebugModeToggleRequestHandler(
        std::move(debugModeToggleRequestHandler)
    );

    pipeline.Install(*world, *events);
}

WorldSession::~WorldSession() = default;
World& WorldSession::GetWorld() const { return *world; }
EventManager& WorldSession::GetEvents() const { return *events; }
