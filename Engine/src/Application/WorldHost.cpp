#include "WorldHost.h"
#include "WorldSession.h"
#include <stdexcept>
#include <utility>


WorldHost::WorldHost() = default; 

WorldHost::~WorldHost() = default;

bool WorldHost::HasActiveSession() const noexcept
{
    return activeSession != nullptr;
}

void WorldHost::SetActiveSession(std::unique_ptr<WorldSession> session)
{
    if (!session)
        throw std::invalid_argument(
            "WorldHost cannot activate a null WorldSession.");
    // A future SceneManager will call this only at a safe frame boundary.
    activeSession = std::move(session);
}

void WorldHost::ClearActiveSession()
{
    activeSession.reset();
}

World& WorldHost::GetWorld() const
{
    return RequireActiveSession().GetWorld();
}

EventManager& WorldHost::GetEvents() const
{
    return RequireActiveSession().GetEvents();
}

WorldSession& WorldHost::RequireActiveSession() const
{
    if (!activeSession)
        throw std::logic_error("WorldHost has no active WorldSession.");
    return *activeSession;
}