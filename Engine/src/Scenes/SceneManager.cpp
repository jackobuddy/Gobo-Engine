#include "SceneManager.h"
#include "../Application/EngineSystemPipeline.h"
#include "../Application/GameContext.h"
#include "../Application/WorldHost.h"
#include "../Application/WorldSession.h"
#include "../Audio/AudioManager.h"
#include "../Events/EventManager.h"
#include "../Events/SceneStartedEvent.h"
#include "../Logger/Logger.h"
#include "../Rendering/Camera.h"
#include "../Rendering/TextManager.h"
#include "../ECS/World.h"
#include <exception>
#include <utility>


SceneManager::SceneManager(WorldHost& worldHost, const EngineSystemPipeline& pipeline,
    Vector2 defaultCameraPosition, Vector2 defaultCameraViewSize, std::function<void()> quitRequestHandler,
    std::function<void()> debugModeToggleRequestHandler): worldHost(worldHost), pipeline(pipeline),
    defaultCameraPosition(defaultCameraPosition), defaultCameraViewSize(defaultCameraViewSize),
    quitRequestHandler(std::move(quitRequestHandler)), 
    debugModeToggleRequestHandler(std::move(debugModeToggleRequestHandler))
{
}

bool SceneManager::RegisterFactory(const std::string& id, SceneFactory factory)
{
    if (id.empty() || !factory)
    {
        Logger::Error("Scene ID and factory must be valid.");
        return false;
    }
    if (factories.contains(id))
    {
        Logger::Error("Scene ID is already registered: " + id);
        return false;
    }
    factories.emplace(id, std::move(factory));
    return true;
}

bool SceneManager::HasScene(const std::string& id) const
{
    return factories.contains(id);
}

std::size_t SceneManager::GetRegisteredSceneCount() const
{
    return factories.size();
}

bool SceneManager::HasActiveScene() const
{
    return activeScene != nullptr;
}

const std::string& SceneManager::GetActiveSceneID() const
{
    return activeSceneID;
}

bool SceneManager::GoTo(const std::string& id)
{
    if (!HasActiveScene())
    {
        Logger::Error("Cannot change Scene before the starting Scene is active.");
        return false;
    }
    if (!HasScene(id))
    {
        Logger::Error("Scene is not registered: " + id);
        return false;
    }
    if (pendingSceneID)
    {
        Logger::Warning("A Scene transition is already pending.");
        return false;
    }
    pendingSceneID = id;
    return true;
}

bool SceneManager::Restart()
{
    if (!HasActiveScene())
    {
        Logger::Error("Cannot restart without an active Scene.");
        return false;
    }
    return GoTo(activeSceneID);
}

bool SceneManager::StartInitial(const std::string& id, GameContext& context)
{
    if (HasActiveScene())
    {
        Logger::Error("The starting Scene is already active.");
        return false;
    }
    return Activate(id, context);
}

bool SceneManager::Activate(const std::string& id, GameContext& context)
{
    const auto found = factories.find(id);
    if (found == factories.end())
    {
        Logger::Error("Scene is not registered: " + id);
        return false;
    }
    std::unique_ptr<Scene> nextScene;
    std::unique_ptr<WorldSession> nextSession;
    try
    {
        nextScene = found->second();
        nextSession = std::make_unique<WorldSession>(pipeline, quitRequestHandler, 
            debugModeToggleRequestHandler);
    }
    catch (const std::exception& error)
    {
        Logger::Error(
            "Failed to prepare Scene '" + id + "': " + error.what()
        );
        return false;
    }
    catch (...)
    {
        Logger::Error(
            "Failed to prepare Scene '" + id + "'."
        );
        return false;
    }
    if (!nextScene || !nextSession)
    {
        Logger::Error("Scene factory returned an invalid Scene: " + id);
        return false;
    }
    if (activeScene)
    {
        try
        {
            activeScene->OnEnd(context);
        }
        catch (const std::exception& error)
        {
            Logger::Error(
                "Scene OnEnd failed for '" + activeSceneID +
                "': " + error.what()
            );
        }
        catch (...)
        {
            Logger::Error(
                "Scene OnEnd failed for '" + activeSceneID + "'."
            );
        }

        activeScene->DetachContext(); 
    }
    // Destroy the old Scene object while its old World still exists.
    activeScene.reset();
    // These services are Scene-scoped in the first implementation.
    context.GetText().Clear();
    // Swapping the session destroys the old World and EventManager.
    worldHost.SetActiveSession(std::move(nextSession));
    context.GetCamera().position = defaultCameraPosition;
    if (!context.SetViewSize(static_cast<int>(defaultCameraViewSize.x),
        static_cast<int>(defaultCameraViewSize.y)))
    {
        Logger::Error("Could not restore the default logical view size");
        context.RequestQuit();
    }
    activeSceneID = id;
    activeScene = std::move(nextScene);
    activeScene->AttachContext(context); 
    try
    {
        activeScene->OnStart(context);
        context.GetEvents().EmitEvent<SceneStartedEvent>(activeSceneID);
        context.GetWorld().Update(); 
    }
    catch (const std::exception& error)
    {
        Logger::Error(
            "Scene OnStart failed for '" + activeSceneID +
            "': " + error.what()
        );
        activeScene->DetachContext(); 
        activeScene.reset();
        worldHost.ClearActiveSession();
        activeSceneID.clear();
        context.GetText().Clear();
        quitRequestHandler();
        return false;
    }
    catch (...)
    {
        Logger::Error(
            "Scene OnStart failed for '" + activeSceneID + "'."
        );
        activeScene->DetachContext();
        activeScene.reset();
        worldHost.ClearActiveSession();
        activeSceneID.clear();
        context.GetText().Clear();
        quitRequestHandler();
        return false;
    }
    Logger::Log("Scene started: " + activeSceneID);
    return true;
}

void SceneManager::ProcessPending(GameContext& context)
{
    if (!pendingSceneID)
        return;
    std::string requestedID = std::move(*pendingSceneID);
    pendingSceneID.reset();
    if (!Activate(requestedID, context))
    {
        Logger::Error(
            "Scene transition failed: " + requestedID
        );
    }
}

void SceneManager::BeginFrame(GameContext& context)
{
    if (activeScene) activeScene->BeginFrame(context);
}

void SceneManager::Update(GameContext& context, float deltaSeconds)
{
    if (!activeScene) return;
    activeScene->Update(context, deltaSeconds);
    if (context.IsRunning()) context.GetWorld().UpdateObjects(deltaSeconds);
}

void SceneManager::Render(GameContext& context)
{
    if (activeScene) activeScene->Render(context);
}

void SceneManager::Shutdown(GameContext& context)
{
    pendingSceneID.reset();
    if (activeScene)
    {
        try { activeScene->OnEnd(context); }
        catch (const std::exception& error)
        {
            Logger::Error("Scene OnEnd failed: " + std::string(error.what()));
        }
        catch (...)
        {
            Logger::Error("Scene OnEnd failed.");
        }

        activeScene->DetachContext();
    }
    activeScene.reset();
    worldHost.ClearActiveSession();
    activeSceneID.clear();
    context.GetText().Clear();
    context.GetCamera().position = defaultCameraPosition;
    if (!context.SetViewSize(static_cast<int>(defaultCameraViewSize.x),
        static_cast<int>(defaultCameraViewSize.y)))
    {
        Logger::Error("Could not restore the default logical view size");
        context.RequestQuit();
    }
}