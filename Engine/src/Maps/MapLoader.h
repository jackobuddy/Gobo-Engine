#pragma once 

#include "../ECS/World.h"
#include "../Math/Math.h"
#include <concepts>
#include <functional>
#include <istream>
#include <string>
#include <unordered_map>
#include <utility>
#include <vector>

class GameContext;

struct MapPlacement {
    std::string name;
    std::string type;
    Vector2 position;
};

struct MapDefinition {
    int viewWidth = 0;
    int viewHeight = 0;
    Vector2 worldSize;
    std::vector<MapPlacement> entities;
};


class MapTypeRegistry {
public:
    template <typename T> requires std::derived_from<T, Actor>
    void RegisterActor(std::string id) {
        static_assert(std::constructible_from<T, Entity, std::string, const Actor::Config&>);
        Add(std::move(id), [](World& world, const MapPlacement& p) {
            Actor::Config c{};
            c.position = p.position;
            world.CreateActor<T>(p.name, c);
            });
    }
    template <typename T> requires std::derived_from<T, Character>
    void RegisterCharacter(std::string id) {
        static_assert(std::constructible_from<T, Entity, std::string,
            const Character::Config&>);
        Add(std::move(id), [](World& world, const MapPlacement& p) {
            Character::Config c{};
            c.actor.position = p.position;
            world.CreateCharacter<T>(p.name, c);
            });
    }
    template <typename T> requires std::derived_from<T, ClickTarget>
    void RegisterClickTarget(std::string id) {
        static_assert(std::constructible_from<T, Entity, std::string,
            const ClickTarget::Config&>);
        Add(std::move(id), [](World& world, const MapPlacement& p) {
            ClickTarget::Config c{};
            c.actor.position = p.position;
            world.CreateClickTarget<T>(p.name, c);
            });
    }

    bool Contains(const std::string& id) const;
    void Spawn(World& world, const MapPlacement& p) const;

private:
    using Factory = std::function<void(World&, const MapPlacement&)>;
    void Add(std::string id, Factory factory);
    std::unordered_map<std::string, Factory> factories;
};


class MapLoader {
public:
    static MapDefinition Parse(std::istream& input);
    static void LoadIntoScene(GameContext& context, const std::string& mapID); 
    static void LoadIntoScene(GameContext& context, const std::string& mapId,
        const MapTypeRegistry& registry);
};