#include "RPGStatsModule.h"
#include "../Application/GameContext.h"
#include "../ECS/World.h"
#include "../Logger/Logger.h"
#include <algorithm>
#include <cmath>
#include <exception>
#include <fstream>
#include <functional>
#include <iomanip>
#include <ostream>
#include <SDL3/SDL.h>
#include <stdexcept>
#include <string_view>
#include <system_error>
#include <utility>


StatBlock::StatBlock(RPGStatsModule& module, const StatProfile& profile):
    owner(&module), profileID(profile.id), baseValues(profile.baseValues),
    stringValues(profile.stringValues) {
}

bool StatBlock::AddPoints(const std::string& statID, double amount)
{
    auto definition = owner->definitions.find(statID);
    if (definition == owner->definitions.end() || !std::isfinite(amount)) return false;
    if (definition->second.type == StatValueType::String) return false;
    if (definition->second.type == StatValueType::Integer && std::floor(amount) != amount) 
        return false;

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
    if (owner->definitions.at(statID).type == StatValueType::String)
        return std::nullopt;
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
    if (owner->definitions.at(statID).type == StatValueType::String) return std::nullopt;
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
    const std::string label =
        definition.id.empty() ? "<empty>" : definition.id;

    const auto fail = [&](const std::string& reason)
        {
            Logger::Error("RPG stat '" + label + "': " + reason);
            return false;
        };

    if (started)
        return fail("cannot be defined after module startup");
    if (definition.id.empty())
        return fail("ID is empty");
    if (!categories.contains(definition.category))
        return fail("category '" + definition.category + "' is not defined");
    if (definitions.contains(definition.id))
        return fail("ID is already defined");

    if (definition.type == StatValueType::String)
    {
        if (definition.expression)
            return fail("string stats cannot have an expression");
        if (definition.minimum || definition.maximum)
            return fail("string stats cannot have numeric bounds");
    }
    else
    {
        if (definition.minimum && !std::isfinite(*definition.minimum))
            return fail("minimum is not finite");
        if (definition.maximum && !std::isfinite(*definition.maximum))
            return fail("maximum is not finite");
        if (definition.minimum && definition.maximum &&
            *definition.minimum > *definition.maximum)
            return fail("minimum exceeds maximum");

        if (definition.type == StatValueType::Integer)
        {
            if (definition.minimum &&
                std::floor(*definition.minimum) != *definition.minimum)
                return fail("integer stat has a fractional minimum");
            if (definition.maximum &&
                std::floor(*definition.maximum) != *definition.maximum)
                return fail("integer stat has a fractional maximum");
        }
    }

    const std::string id = definition.id;
    return definitions.emplace(id, std::move(definition)).second;
}

bool RPGStatsModule::DefineProfile(StatProfile profile)
{
    if (started || profile.id.empty() || profiles.contains(profile.id)) return false;
    const std::string id = profile.id;
    return profiles.emplace(id, std::move(profile)).second;
}

bool RPGStatsModule::UpdateProfile(const std::string& profileID, const std::string& statID, double value)
{
    const auto profile = profiles.find(profileID);
    if (profile == profiles.end())
    {
        Logger::Error("Unknown RPG profile '" + profileID + "'");
        return false;
    }

    const auto definition = definitions.find(statID);
    if (definition == definitions.end() ||
        definition->second.type == StatValueType::String)
    {
        Logger::Error("Profile '" + profileID +
            "' requires a defined numeric stat '" + statID + "'");
        return false;
    }

    if (!std::isfinite(value))
    {
        Logger::Error("Profile '" + profileID +
            "' has a non-finite value for '" + statID + "'");
        return false;
    }

    profile->second.baseValues[statID] = value;
    return true;
}

bool RPGStatsModule::UpdateProfile(const std::string& profileID, const std::string& statID, std::string value)
{
    const auto profile = profiles.find(profileID);
    if (profile == profiles.end())
    {
        Logger::Error("Unknown RPG profile '" + profileID + "'");
        return false;
    }

    const auto definition = definitions.find(statID);
    if (definition == definitions.end() ||
        definition->second.type != StatValueType::String)
    {
        Logger::Error("Profile '" + profileID +
            "' requires a defined string stat '" + statID + "'");
        return false;
    }

    profile->second.stringValues[statID] = std::move(value);
    return true;
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
    std::map<std::string, int> state;
    std::function<void(const std::string&)> visit =
        [&](const std::string& id)
        {
            if (state[id] == 1)
                throw std::logic_error("Stat formula cycle: " + id);
            if (state[id] == 2) return;
            state[id] = 1;
            const StatDefinition& definition = definitions.at(id);
            std::set<std::string> seen;
            for (const std::string& dependency : definition.expression.Dependencies())
            {
                const auto found = definitions.find(dependency);
                if (found == definitions.end())
                    throw std::logic_error("Stat '" + id + "' references undefined stat '" +
                        dependency + "'");
                if (found->second.type == StatValueType::String)
                    throw std::logic_error("Stat '" + id + "' references string stat '" +
                        dependency + "'");
                visit(dependency);
            }
            state[id] = 2;
            evaluationOrder.push_back(id);
        };
    for (const auto& [id, definition] : definitions)
        if (definition.type != StatValueType::String) visit(id);
    for (const auto& [profileID, profile] : profiles)
    {
        for (const auto& [statID, value] : profile.baseValues)
        {
            const auto found = definitions.find(statID);
            const std::string entry =
                "RPG profile '" + profileID + "', numeric stat '" + statID + "': ";

            if (found == definitions.end())
                throw std::logic_error(entry + "stat is not defined");
            if (found->second.type == StatValueType::String)
                throw std::logic_error(entry + "stat is defined as String");
            if (!std::isfinite(value))
                throw std::logic_error(entry + "value is not finite");
        }

        for (const auto& entry : profile.stringValues)
        {
            const std::string& statID = entry.first;
            const auto found = definitions.find(statID);
            const std::string label = "RPG profile '" + profileID + "', string stat '" + statID + "': ";

            if (found == definitions.end())
                throw std::logic_error(label + "stat is not defined");
            if (found->second.type != StatValueType::String)
                throw std::logic_error(label + "stat is not defined as String");
        }
    }
    started = true;
}

bool RPGStatsModule::ValidModifiers(const std::vector<StatModifier>& modifiers) const
{
    for (const StatModifier& modifier : modifiers)
    {
        auto found = definitions.find(modifier.statID);
        if (found == definitions.end() ||
            found->second.type == StatValueType::String ||
            !std::isfinite(modifier.flat) ||
            !std::isfinite(modifier.percent)) return false;
    }
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
                    if (definition.expression)
                        result += definition.expression.Evaluate(values);
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
    reportRoot.clear(); 
    started = false;
}

bool StatBlock::SetString(const std::string& statID, std::string value)
{
    auto found = owner->definitions.find(statID);
    if (found == owner->definitions.end() ||
        found->second.type != StatValueType::String) return false;
    stringValues[statID] = std::move(value);
    return true;
}

std::optional<std::string> StatBlock::GetString(
    const std::string& statID) const
{
    auto found = owner->definitions.find(statID);
    if (found == owner->definitions.end() ||
        found->second.type != StatValueType::String) return std::nullopt;
    auto value = stringValues.find(statID);
    return value == stringValues.end()
        ? std::nullopt : std::optional<std::string>(value->second);
}

bool StatBlock::ClearString(const std::string& statID)
{
    auto found = owner->definitions.find(statID);
    if (found == owner->definitions.end() ||
        found->second.type != StatValueType::String) return false;
    return stringValues.erase(statID) != 0;
}

bool RPGStatsModule::SetDebugReportRoot(std::filesystem::path projectRoot)
{
    namespace fs = std::filesystem;
    if (projectRoot.empty() || !projectRoot.is_absolute()) return false;
    std::error_code error;
    fs::path resolved = fs::weakly_canonical(projectRoot, error);
    if (error || !fs::is_directory(resolved, error) || error)
        return false;
    reportRoot = std::move(resolved);
    return true;
}

bool RPGStatsModule::CreateDebugReport()
{
    namespace fs = std::filesystem;

    fs::path destination = reportRoot; // Optional explicit override.
    if (destination.empty())
    {
        const char* base = SDL_GetBasePath();
        if (!base || !*base)
        {
            Logger::SDLError("Cannot locate RPG report directory", SDL_GetError());
            return false;
        }
        destination = fs::u8path(base);
    }

    std::error_code error;
    if (!fs::is_directory(destination, error) || error)
    {
        Logger::Error("RPG report directory is unavailable.");
        return false;
    }

    const fs::path path = destination / "RPGStatsModuleReport.txt";
    std::ofstream file(path, std::ios::out | std::ios::trunc);
    if (!file)
    {
        Logger::Error("Cannot open RPGStatsModuleReport.txt.");
        return false;
    }
    file << std::setprecision(17);
    const auto quoted = [](std::string_view text)
        {
            std::string result = "\"";
            for (char ch : text)
            {
                if (ch == '\\') result += "\\\\";
                else if (ch == '"') result += "\\\"";
                else if (ch == '\n') result += "\\n";
                else if (ch == '\r') result += "\\r";
                else if (ch == '\t') result += "\\t";
                else result += ch;
            }
            result += '"';
            return result;
        };
    const auto numbers = [&](const StatValues& values)
        {
            const std::map<std::string, double> sorted(
                values.begin(), values.end());
            for (const auto& [id, value] : sorted)
                file << "    " << id << " = " << value << '\n';
        };
    const auto strings = [&](const StringStatValues& values)
        {
            const std::map<std::string, std::string> sorted(
                values.begin(), values.end());
            for (const auto& [id, value] : sorted)
                file << "    " << id << " = " << quoted(value) << '\n';
        };
    const auto modifiers = [&](const std::vector<StatModifier>& list)
        {
            for (const auto& item : list)
                file << "    " << item.statID << " flat=" << item.flat
                << " percent=" << item.percent << '\n';
        };
    file << "RPGStatsModuleReport\n"
        << "started=" << started << "\n\n";
    file << "Categories\n";
    for (const auto& id : categories) file << "  " << id << '\n';
    file << "Damage types\n";
    for (const auto& id : damageTypes) file << "  " << id << '\n';
    file << "Definitions\n";
    for (const auto& [id, def] : definitions)
    {
        const char* type = def.type == StatValueType::String ? "String"
            : def.type == StatValueType::Integer ? "Integer" : "Float";
        file << "  " << id << " category=" << def.category  << " type=" << type << '\n';
        file << "    minimum=";
        if (def.minimum) file << *def.minimum; else file << "none";
        file << " maximum=";
        if (def.maximum) file << *def.maximum; else file << "none";
        file << "\n    formula=";
        file << "\n    expression=";
        if (def.expression)
            file << quoted(def.expression.ToString());
        else
            file << "none";

        file << "\n    dependencies=";
        for (const std::string& dependency : def.expression.Dependencies())
            file << ' ' << dependency;
        file << '\n';
    }
    file << "Evaluation order (numeric only)\n";
    for (const auto& id : evaluationOrder) file << "  " << id << '\n';
    file << "Profiles\n";
    for (const auto& [id, profile] : profiles)
    {
        file << "  " << id << "\n  numeric base values\n";
        numbers(profile.baseValues);
        file << "  string base values\n";
        strings(profile.stringValues);
    }

    const auto block = [&](const std::string& label, StatBlock& value)
        {
            file << "  " << label << " profile=" << value.profileID << '\n';
            const bool calculated = started &&
                (!value.dirty || Calculate(value));
            file << "    calculated=" << calculated
                << " dirty=" << value.dirty << '\n';
            file << "    numeric base\n"; numbers(value.baseValues);
            file << "    string values\n"; strings(value.stringValues);
            file << "    allocated points\n";
            numbers(value.allocatedPoints);
            file << "    permanent sources\n";
            for (const auto& [id, items] : value.permanentSources)
            {
                file << "      " << id << '\n'; modifiers(items);
            }
            file << "    timed effects\n";
            for (const auto& [id, effect] : value.effects)
            {
                file << "      " << id << " remaining="
                    << effect.remainingSeconds << '\n';
                modifiers(effect.modifiers);
            }
            if (calculated)
            {
                file << "    stable values\n";
                numbers(value.stableCache);
                file << "    effective values\n";
                numbers(value.effectiveCache);
            }
            else file << "    stable/effective values unavailable\n";
        };
    file << "Persistent blocks\n";
    for (auto& [id, value] : persistentBlocks)
        block(id, *value);
    file << "Scene blocks\n";
    for (auto& [entity, value] : sceneBlocks)
    {
        std::string label = "entity#" + std::to_string(entity.GetID());
        if (activeWorld && activeWorld->CanRunCallbacks(entity))
            if (auto* object = activeWorld->GetObject(entity))
                label += " name=" + quoted(object->GetName());
        block(label, *value);
    }
    file << "Persistent bindings\n";
    for (const auto& [entity, id] : persistentBindings)
        file << "  entity#" << entity.GetID() << " -> " << id << '\n';
    file.flush();
    file.close();
    if (!file)
    {
        Logger::Error("RPG report write failed.");
        return false;
    }
    Logger::Log("RPGStatsModuleReport.txt created.");
    return true;
}