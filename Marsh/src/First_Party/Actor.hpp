//
//  Actor.hpp
//  game_engine
//
//  Created by Tyler Covacha on 1/29/26.
//

#ifndef Actor_hpp
#define Actor_hpp

#include <stdio.h>

#include <iostream>
#include <string>
#include "glm/glm.hpp"
#include <optional>
#include "lua/lua.hpp"
#include "LuaBridge/LuaBridge.h"
#include <map>
#include <set>
#include <unordered_map>
#include "ComponentDB.hpp"

#pragma once
class ActorComponentsAdd {
public:
    Actor* actor;
    std::string type_name;
    std::string new_type_name;
    luabridge::LuaRef* instance_table;
    
    ActorComponentsAdd(Actor* _actor, std::string _type_Name, std::string _new_type_name, luabridge::LuaRef* _instance_table) {
        actor = _actor;
        type_name = _type_Name;
        new_type_name = _new_type_name;
        instance_table = _instance_table;
    }
};

class ActorComponentsRemove {
public:
    Actor* actor;
    luabridge::LuaRef* instance_table;
    
    ActorComponentsRemove() {
        actor = nullptr;
        instance_table = nullptr;
    }
    
    ActorComponentsRemove(Actor* _actor, luabridge::LuaRef* _instance_table) {
        actor = _actor;
        instance_table = _instance_table;
    }
};

class Actor {
 public:
    std::string name = "";
    int id = -1;
    std::map<std::string, luabridge::LuaRef*> instance_tables; // key, table
    std::unordered_map<std::string, luabridge::LuaRef*> instance_table_find; // key, table
    std::unordered_map<std::string, std::set<std::string>> table_types; // type, key
    bool loaded = false;
    bool dontDestroy = false;
    
    static inline int n = 0;
    static inline std::vector<ActorComponentsAdd> components_to_add;
    static inline std::unordered_map<std::string, ActorComponentsRemove> components_to_remove; // key, comopnent_to_remove
    
    std::string GetName();
    int GetID();
    void InjectConvenienceReferences(luabridge::LuaRef* component_ref);
    luabridge::LuaRef GetComponentByKey(std::string key);
    luabridge::LuaRef GetComponent(std::string type_name);
    luabridge::LuaRef GetComponents(std::string type_name);
    luabridge::LuaRef AddComponent(std::string type_name);
    void RemoveComponent(luabridge::LuaRef component_ref);
};

#endif /* Actor_hpp */
