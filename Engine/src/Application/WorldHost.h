#pragma once

#include <memory>

class EventManager;
class World;
class WorldSession;


class WorldHost final
{
public:
    WorldHost();
    ~WorldHost();

    WorldHost(const WorldHost&) = delete;
    WorldHost& operator=(const WorldHost&) = delete;

    bool HasActiveSession() const noexcept;
    void SetActiveSession(std::unique_ptr<WorldSession> session);
    void ClearActiveSession();
    World& GetWorld() const;
    EventManager& GetEvents() const;

private:
    WorldSession& RequireActiveSession() const;

    std::unique_ptr<WorldSession> activeSession;
};
