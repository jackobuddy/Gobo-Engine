#pragma once

#include "Module.h"
#include "../ECS/Entity.h"
#include "RPGStatsModule/StatExpression.h"
#include <filesystem>
#include <functional>
#include <map>
#include <memory>
#include <optional>
#include <set>
#include <string>
#include <unordered_map>
#include <vector>

class GameContext;
struct ModuleServices;
class World; 
class RPGStatsModule; 

using StatValues = std::unordered_map<std::string, double>; 
using StringStatValues = std::unordered_map<std::string, std::string>;
enum class StatValueType { Integer, Float, String };

struct StatModifier
{
	std::string statID; 
	double flat = 0.0f; 
	// 0.25 means +25%
	double percent = 0.0;
};

struct StatDefinition
{
	std::string category; 
	StatExpression expression; 
	std::string id;
	std::optional<double> maximum; 
	std::optional<double> minimum;
	StatValueType type = StatValueType::Float;
};

struct StatProfile
{
	std::string id; 
	StatValues baseValues; 
	StringStatValues stringValues; 
	std::string statSetID = "default"; 
};

class StatBlock final
{
	friend class RPGStatsModule; 
public:
	~StatBlock() = default; 
	StatBlock(const StatBlock&) = delete; 
	StatBlock& operator=(const StatBlock&) = delete; 
	StatBlock(StatBlock&&) = delete; 
	StatBlock& operator=(StatBlock&&) = delete; 

	bool AddPoints(const std::string& statID, double amount); 
	std::optional<double> GetPoints(const std::string& statID) const; 
	bool SetBase(const std::string& statID, double value);
	bool SetString(const std::string& statID, std::string value); 
	std::optional<std::string> GetString(const std::string& statID) const; 
	bool ClearString(const std::string& statID); 
	bool SetPermanentSource(const std::string& sourceID, std::vector<StatModifier> modifiers); 
	bool RemovePermanentSource(const std::string& sourceID); 
	bool ApplyEffect(const std::string& effectID, float seconds, std::vector<StatModifier> modifiers);
	bool RemoveEffect(const std::string& effectID); 
	bool Recalculate(); 
	std::optional<double> Get(const std::string& statID, bool ignoreTempModifiers = false); 
	const std::string& GetProfileID() const noexcept { return profileID; }
	const std::string& GetStatSetID() const noexcept { return statSetID; }

private:
	struct TimedEffect
	{
		float remainingSeconds; 
		std::vector<StatModifier> modifiers; 
	};

	StatBlock(RPGStatsModule& owner, const StatProfile& profile); 
	void Tick(float deltaSeconds); 

	RPGStatsModule* owner; 
	std::string profileID; 
	std::string statSetID; 
	StatValues baseValues; 
	StringStatValues stringValues; 
	StatValues allocatedPoints; 
	std::map<std::string, std::vector<StatModifier>> permanentSources;
	std::map<std::string, TimedEffect> effects;
	StatValues stableCache; 
	StatValues effectiveCache; 
	bool dirty = true; 
};

class RPGStatsModule final : public Module 
{
	friend class StatBlock; 

public:
	bool DefineCategory(const std::string& id); 
	bool DefineStat(StatDefinition definition); 
	bool DefineStatSet(const std::string& id); 
	bool IncludeStat(const std::string& setID, const std::string& statID); 
	bool OverrideStatExpression(const std::string& setID, const std::string& statID, StatExpression expression);
	bool HasStatInSet(const std::string& setID, const std::string& statID) const; 
	bool DefineProfile(StatProfile profile); 
	bool UpdateProfile(const std::string& profileID, const std::string& statID, double value);
	bool UpdateProfile(const std::string& profileID, const std::string& statID, std::string value);
	bool DefineDamageType(const std::string& id); 
	bool HasDamageType(const std::string& id) const; 
	const StatDefinition* FindDefinition(const std::string& id) const; 
	std::vector<std::string> GetStatIDsInCategory(const std::string& category) const; 

	// Unique Stat Blocks survive scene replacement, not application shutdown. 
	StatBlock* CreatePersistent(const std::string& uniqueID, const std::string& profileID); 
	StatBlock* FindPersistent(const std::string& uniqueID); 
	bool ForgetPersistent(const std::string& uniqueID); 

	// A live scene is required for Entity attachments
	StatBlock* Attach(Entity entity, const std::string& profileID); 
	StatBlock* AttachPersistent(Entity entity, const std::string& uniqueID, 
		const std::string& profileID); 
	StatBlock* Find(Entity entity); 
	bool Detach(Entity entity); 

	void Start(ModuleServices&) override; 
	void EnterScene(GameContext& context) override; 
	void Update(GameContext&, float deltaSeconds) override; 
	void LeaveScene(GameContext&) override; 
	void Shutdown() override; 

	bool SetDebugReportRoot(std::filesystem::path projectRoot);
	bool CreateDebugReport();

private:
	bool ValidModifiers(const StatBlock& block, const std::vector<StatModifier>& modifiers) const;
	bool Calculate(StatBlock& block, const std::string* rawStatID = nullptr, double* rawStableValue = nullptr);
	bool CanAttach(Entity entity) const; 
	void PruneDeadEntities(); 

	struct StatSet
	{
		std::set<std::string> roots; 
		std::map<std::string, StatExpression> expressionOverrides; 
		std::set<std::string> resolvedIDs; 
		std::vector<std::string> evaluationOrder; 
	};

	std::map<std::string, StatSet> statSets{{"default", StatSet{}}}; 
	std::set<std::string> categories;
	std::map<std::string, StatDefinition> definitions;
	std::map<std::string, StatProfile> profiles;
	std::set<std::string> damageTypes;
	std::map<Entity, std::unique_ptr<StatBlock>> sceneBlocks;
	std::map<std::string, std::unique_ptr<StatBlock>> persistentBlocks;
	std::map<Entity, std::string> persistentBindings;
	World* activeWorld = nullptr; // Borrowed; cleared by LeaveScene.
	bool started = false;
	std::filesystem::path reportRoot;
};