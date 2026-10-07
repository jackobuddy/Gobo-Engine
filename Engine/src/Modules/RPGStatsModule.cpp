#include "RPGStatsModule.h"
#include "../Application/GameContext.h"
#include "../ECS/World.h"
#include "../Logger/Logger.h"
#include <algorithm>
#include <cmath>
#include <exception>
#include <functional>
#include <stdexcept>
#include <utility>


StatBlock::StatBlock(RPGStatsModule& module, const StatProfile& profile)
    : owner(&module), profileID(profile.id), baseValues(profile.baseValues) {
}

bool StatBlock::AddPoints(const std::string& statID, double amount)
{
    auto definition = owner->definitions.find(statID);
    if (definition == owner->definitions.end() || !std::isfinite(amount))
        return false;
    if (definition->second.type == StatValueType::Integer &&
        std::floor(amount) != amount) return false;
    const auto previous = allocatedPoints.find(statID);
    const double next = (previous == allocatedPoints.end() ? 0.0 : previous->second)
        + amount;
    if (!std::isfinite(next) || next < 0.0) return false;
    allocatedPoints[statID] = next;
    dirty = true;
    return true;
}

std::optional<double> StatBlock::GetPoints(const std::string& statID) const
{
    if (!owner->definitions.contains(statID)) return std::nullopt;
    auto found = allocatedPoints.find(statID);
    return found == allocatedPoints.end() ? 0.0 : found->second;
}

bool StatBlock::SetPermanentSource(const std::string& sourceID, std::vector<StatModifier> modifiers)
{
    if (sourceID.empty() || !owner->ValidModifiers(modifiers)) return false;
    permanentSources[sourceID] = std::move(modifiers);
    dirty = true;
    return true;
}

bool StatBlock::RemovePermanentSource(const std::string& sourceID)
{
    if (permanentSources.erase(sourceID) == 0) return false;
    dirty = true;
    return true;
}

bool StatBlock::ApplyEffect(const std::string& effectID, float seconds,
    std::vector<StatModifier> modifiers)
{
    if (effectID.empty() || !std::isfinite(seconds) || seconds <= 0.0f ||
        !owner->ValidModifiers(modifiers)) return false;
    effects.insert_or_assign(effectID,
        TimedEffect{ seconds, std::move(modifiers) });
    dirty = true;
    return true;
}

bool StatBlock::RemoveEffect(const std::string& effectID)
{
    if (effects.erase(effectID) == 0) return false;
    dirty = true;
    return true;
}

void StatBlock::Tick(float deltaSeconds)
{
    if (!std::isfinite(deltaSeconds) || deltaSeconds <= 0.0f) return;
    for (auto it = effects.begin(); it != effects.end();)
    {
        it->second.remainingSeconds -= deltaSeconds;
        if (it->second.remainingSeconds <= 0.0f)
        {
            it = effects.erase(it);
            dirty = true;
        }
        else ++it;
    }
}

bool StatBlock::Recalculate()
{
    return owner->Calculate(*this);
}

std::optional<double> StatBlock::Get(const std::string& statID, bool ignoreTempModifiers)
{
    if (!owner->definitions.contains(statID)) return std::nullopt;
    if (dirty && !Recalculate()) return std::nullopt;
    const StatValues& values = ignoreTempModifiers ? stableCache : effectiveCache;
    return values.at(statID);
}

bool RPGStatsModule::DefineCategory(const std::string& id)
{
    return !started && !id.empty() && categories.insert(id).second;
}

bool RPGStatsModule::DefineStat(StatDefinition definition)
{
    if (started || definition.id.empty() ||
        !categories.contains(definition.category) ||
        definitions.contains(definition.id) ||
        (!definition.formula && !definition.dependencies.empty())) return false;
    if (definition.minimum && !std::isfinite(*definition.minimum)) return false;
    if (definition.maximum && !std::isfinite(*definition.maximum)) return false;
    if (definition.minimum && definition.maximum &&
        *definition.minimum > *definition.maximum) return false;
    if (definition.type == StatValueType::Integer &&
        ((definition.minimum && std::floor(*definition.minimum) != *definition.minimum) ||
            (definition.maximum && std::floor(*definition.maximum) != *definition.maximum)))
        return false;
    const std::string id = definition.id;
    return definitions.emplace(id, std::move(definition)).second;
}

bool RPGStatsModule::DefineProfile(StatProfile profile)
{
    if (started || profile.id.empty() || profiles.contains(profile.id)) return false;
    const std::string id = profile.id;
    return profiles.emplace(id, std::move(profile)).second;
}

bool RPGStatsModule::DefineDamageType(const std::string& id)
{
    return !started && !id.empty() && damageTypes.insert(id).second;
}

bool RPGStatsModule::HasDamageType(const std::string& id) const
{
    return damageTypes.contains(id);
}

const StatDefinition* RPGStatsModule::FindDefinition(const std::string& id) const
{
    auto found = definitions.find(id);
    return found == definitions.end() ? nullptr : &found->second;
}

std::vector<std::string> RPGStatsModule::GetStatIDsInCategory(const std::string& category) const
{
    std::vector<std::string> ids;
    for (const auto& [id, definition] : definitions)
        if (definition.category == category) ids.push_back(id);
    return ids;
}

void RPGStatsModule::Start(ModuleServices&)
{
    if (started) throw std::logic_error("RPGStatsModule already started");
    evaluationOrder.clear();
    std::map<std::string, int> state; // 0 unvisited, 1 visiting, 2 complete.
    std::function<void(const std::string&)> visit = [&](const std::string& id)
        {
            if (state[id] == 1) throw std::logic_error("Stat formula cycle: " + id);
            if (state[id] == 2) return;
            state[id] = 1;
            const StatDefinition& definition = definitions.at(id);
            std::set<std::string> seen;
            for (const std::string& dependency : definition.dependencies)
            {
                if (!definitions.contains(dependency) || !seen.insert(dependency).second)
                    throw std::logic_error("Invalid dependency on stat " + id);
                visit(dependency);
            }
            state[id] = 2;
            evaluationOrder.push_back(id);
        };
    for (const auto& [id, definition] : definitions) visit(id);
    for (const auto& [profileID, profile] : profiles)
        for (const auto& [statID, value] : profile.baseValues)
            if (!definitions.contains(statID) || !std::isfinite(value))
                throw std::logic_error("Invalid base value in profile " + profileID);
    started = true;
}

bool RPGStatsModule::ValidModifiers(const std::vector<StatModifier>& modifiers) const
{
    for (const StatModifier& modifier : modifiers)
        if (!definitions.contains(modifier.statID) ||
            !std::isfinite(modifier.flat) || !std::isfinite(modifier.percent))
            return false;
    return true;
}

bool RPGStatsModule::Calculate(StatBlock& block)
{
    if (!started) return false;
    try
    {
        auto calculatePass = [&](bool withEffects)
            {
                StatValues values;
                for (const std::string& id : evaluationOrder)
                {
                    const StatDefinition& definition = definitions.at(id);
                    double result = block.baseValues.contains(id)
                        ? block.baseValues.at(id) : 0.0;
                    if (block.allocatedPoints.contains(id))
                        result += block.allocatedPoints.at(id);
                    if (definition.formula)
                    {
                        StatValues inputs;
                        for (const std::string& dependency : definition.dependencies)
                            inputs.emplace(dependency, values.at(dependency));
                        result += definition.formula(inputs);
                    }
                    double flat = 0.0;
                    double percent = 0.0;
                    const auto apply = [&](const std::vector<StatModifier>& source)
                        {
                            for (const StatModifier& modifier : source)
                                if (modifier.statID == id)
                                {
                                    flat += modifier.flat;
                                    percent += modifier.percent;
                                }
                        };
                    for (const auto& [sourceID, modifiers] : block.permanentSources)
                        apply(modifiers);
                    if (withEffects)
                        for (const auto& [effectID, effect] : block.effects)
                            apply(effect.modifiers);
                    result = (result + flat) * std::max(0.0, 1.0 + percent);
                    if (!std::isfinite(result))
                        throw std::runtime_error("Non-finite stat: " + id);
                    if (definition.minimum) result = std::max(result, *definition.minimum);
                    if (definition.maximum) result = std::min(result, *definition.maximum);
                    if (definition.type == StatValueType::Integer) result = std::round(result);
                    values.emplace(id, result);
                }
                return values;
            };
        StatValues stable = calculatePass(false);
        StatValues effective = calculatePass(true);
        block.stableCache.swap(stable);
        block.effectiveCache.swap(effective);
        block.dirty = false;
        return true;
    }
    catch (const std::exception& error)
    {
        Logger::Error(std::string("RPG stat calculation failed: ") + error.what());
        return false;
    }
    catch (...)
    {
        Logger::Error("RPG stat calculation failed with an unknown exception");
        return false;
    }
}

StatBlock* RPGStatsModule::CreatePersistent(const std::string& uniqueID,
    const std::string& profileID)
{
    if (!started || uniqueID.empty()) return nullptr;
    auto profile = profiles.find(profileID);
    if (profile == profiles.end()) return nullptr;
    auto existing = persistentBlocks.find(uniqueID);
    if (existing != persistentBlocks.end())
        return existing->second->GetProfileID() == profileID
        ? existing->second.get() : nullptr;
    auto block = std::unique_ptr<StatBlock>(new StatBlock(*this, profile->second));
    StatBlock* result = block.get();
    persistentBlocks.emplace(uniqueID, std::move(block));
    return result;
}

StatBlock* RPGStatsModule::FindPersistent(const std::string& uniqueID)
{
    auto found = persistentBlocks.find(uniqueID);
    return found == persistentBlocks.end() ? nullptr : found->second.get();
}

bool RPGStatsModule::ForgetPersistent(const std::string& uniqueID)
{
    PruneDeadEntities();
    for (const auto& [entity, boundID] : persistentBindings)
        if (boundID == uniqueID) return false;
    return persistentBlocks.erase(uniqueID) != 0;
}

bool RPGStatsModule::CanAttach(Entity entity) const
{
    return started && activeWorld && entity.GetWorld() == activeWorld &&
        activeWorld->CanRunCallbacks(entity);
}

void RPGStatsModule::PruneDeadEntities()
{
    for (auto it = sceneBlocks.begin(); it != sceneBlocks.end();)
        if (!activeWorld || !activeWorld->CanRunCallbacks(it->first))
            it = sceneBlocks.erase(it);
        else ++it;
    for (auto it = persistentBindings.begin(); it != persistentBindings.end();)
        if (!activeWorld || !activeWorld->CanRunCallbacks(it->first))
            it = persistentBindings.erase(it);
        else ++it;
}

StatBlock* RPGStatsModule::Attach(Entity entity, const std::string& profileID)
{
    if (!CanAttach(entity) || persistentBindings.contains(entity)) return nullptr;
    auto profile = profiles.find(profileID);
    if (profile == profiles.end()) return nullptr;
    auto existing = sceneBlocks.find(entity);
    if (existing != sceneBlocks.end())
        return existing->second->GetProfileID() == profileID
        ? existing->second.get() : nullptr;
    auto block = std::unique_ptr<StatBlock>(new StatBlock(*this, profile->second));
    StatBlock* result = block.get();
    sceneBlocks.emplace(entity, std::move(block));
    return result;
}

StatBlock* RPGStatsModule::AttachPersistent(Entity entity,
    const std::string& uniqueID, const std::string& profileID)
{
    if (!CanAttach(entity) || sceneBlocks.contains(entity)) return nullptr;
    PruneDeadEntities();
    auto existingBinding = persistentBindings.find(entity);
    if (existingBinding != persistentBindings.end())
    {
        StatBlock* block = FindPersistent(uniqueID);
        return existingBinding->second == uniqueID && block &&
            block->GetProfileID() == profileID ? block : nullptr;
    }
    for (const auto& [other, boundID] : persistentBindings)
        if (boundID == uniqueID) return nullptr;
    StatBlock* block = CreatePersistent(uniqueID, profileID);
    if (!block) return nullptr;
    persistentBindings.emplace(entity, uniqueID);
    return block;
}

StatBlock* RPGStatsModule::Find(Entity entity)
{
    if (!CanAttach(entity)) return nullptr;
    auto local = sceneBlocks.find(entity);
    if (local != sceneBlocks.end()) return local->second.get();
    auto binding = persistentBindings.find(entity);
    return binding == persistentBindings.end()
        ? nullptr : FindPersistent(binding->second);
}

bool RPGStatsModule::Detach(Entity entity)
{
    const bool removedLocal = sceneBlocks.erase(entity) != 0;
    const bool removedBinding = persistentBindings.erase(entity) != 0;
    return removedLocal || removedBinding;
}

void RPGStatsModule::EnterScene(GameContext& context)
{
    sceneBlocks.clear();
    persistentBindings.clear();
    activeWorld = &context.GetWorld();
}

void RPGStatsModule::Update(GameContext&, float deltaSeconds)
{
    PruneDeadEntities();
    for (auto& [entity, block] : sceneBlocks) block->Tick(deltaSeconds);
    for (auto& [id, block] : persistentBlocks) block->Tick(deltaSeconds);
}

void RPGStatsModule::LeaveScene(GameContext&)
{
    sceneBlocks.clear();
    persistentBindings.clear();
    activeWorld = nullptr;
}

void RPGStatsModule::Shutdown()
{
    sceneBlocks.clear();
    persistentBindings.clear();
    persistentBlocks.clear();
    activeWorld = nullptr;
    evaluationOrder.clear();
    definitions.clear();
    profiles.clear();
    categories.clear();
    damageTypes.clear();
    started = false;
}