#include "ModuleManager.h"
#include "ModuleServices.h"
#include "../Application/GameContext.h"
#include "../Logger/Logger.h"
#include <exception>
#include <string>


ModuleManager::~ModuleManager() { ShutdownAll(); }

bool ModuleManager::StartAll(ModuleServices& services)
{
    if (not registrationOpen) return false;
    registrationOpen = false;
    for (auto& entry : entries)
    {
        try
        {
            entry.module->Start(services);
            ++startedCount;
        }
        catch (const std::exception& error)
        {
            Logger::Error(std::string("Module Start failed: ") + error.what());
            ShutdownAll();
            return false;
        }
        catch (...)
        {
            Logger::Error("Module Start failed with an unknown exception.");
            ShutdownAll();
            return false;
        }
    }
    running = true;
    return true;
}

bool ModuleManager::EnterSceneAll(GameContext& context)
{
    if (!running or sceneAttached) return false;
    for (std::size_t i = 0; i < startedCount; ++i)
    {
        try
        {
            entries[i].module->EnterScene(context);
            ++attachedCount;
        }
        catch (const std::exception& error)
        {
            Logger::Error(std::string("Module EnterScene failed: ") + error.what());
            LeaveSceneAll(context);
            return false;
        }
        catch (...)
        {
            Logger::Error("Module EnterScene failed with an unknown exception.");
            LeaveSceneAll(context);
            return false;
        }
    }
    sceneAttached = true;
    return true;
}

void ModuleManager::UpdateAll(GameContext& context, float deltaSeconds)
{
    if (!sceneAttached) return;
    for (std::size_t i = 0; i < attachedCount; ++i)
    {
        if (!context.IsRunning()) break;
        entries[i].module->Update(context, deltaSeconds);
    }
}

void ModuleManager::RenderAll(GameContext& context)
{
    if (!sceneAttached) return;
    for (std::size_t i = 0; i < attachedCount; ++i)
    {
        if (!context.IsRunning()) break;
        entries[i].module->Render(context);
    }
}

void ModuleManager::LeaveSceneAll(GameContext& context) noexcept
{
    while (attachedCount > 0)
    {
        auto& entry = entries[--attachedCount];
        try { entry.module->LeaveScene(context); }
        catch (const std::exception& error)
        {
            Logger::Error(std::string("Module LeaveScene failed: ") +
                error.what());
        }
        catch (...) { Logger::Error("Module LeaveScene failed."); }
    }
    sceneAttached = false;
}

void ModuleManager::ShutdownAll() noexcept
{
    if (sceneAttached)
        Logger::Error("Module shutdown reached before scene detach.");
    sceneAttached = false;
    attachedCount = 0;
    while (startedCount > 0)
    {
        auto& entry = entries[--startedCount];
        try { entry.module->Shutdown(); }
        catch (const std::exception& error)
        {
            Logger::Error(std::string("Module Shutdown failed: ") +
                error.what());
        }
        catch (...) { Logger::Error("Module Shutdown failed."); }
    }
    running = false;
}