//
//  Actor.cpp
//  game_engine
//
//  Created by Tyler Covacha on 1/29/26.
//

#include "Actor.hpp"

std::string Actor::GetName() {
    return name;
}

int Actor::GetID() {
    return id;
}

void Actor::InjectConvenienceReferences(luabridge::LuaRef* component_ref) {
    (*component_ref)["actor"] = this;
}

luabridge::LuaRef Actor::GetComponentByKey(std::string key) {
    if (instance_table_find.find(key) != instance_table_find.end()) {
        if (components_to_remove.find(key) == components_to_remove.end() ||
            components_to_remove[key].actor != this)
            return *instance_table_find[key];
    }
    return luabridge::LuaRef(ComponentDB::GetLuaState());
}

luabridge::LuaRef Actor::GetComponent(std::string type_name) {
    if (table_types.find(type_name) != table_types.end() && !table_types[type_name].empty()) {
        luabridge::LuaRef* table = instance_table_find[*table_types[type_name].begin()];
        std::string key = (*table)["key"];
        if (components_to_remove.find(key) == components_to_remove.end() ||
            components_to_remove[key].actor != this)
            return *instance_table_find[*table_types[type_name].begin()];
    }
    return luabridge::LuaRef(ComponentDB::GetLuaState());
}

luabridge::LuaRef Actor::GetComponents(std::string type_name) {
    luabridge::LuaRef table_of_components = luabridge::newTable(ComponentDB::GetLuaState());
    if (table_types.find(type_name) != table_types.end()) {
        int i = 1;
        for (const std::string& key : table_types[type_name]) {
            if (components_to_remove.find(key) == components_to_remove.end() ||
                components_to_remove[key].actor != this)
                table_of_components[i++] = *instance_table_find[key];
        }
    }
    return table_of_components;
}

luabridge::LuaRef Actor::AddComponent(std::string type_name) {
    if (type_name == "Rigidbody") {
        Rigidbody* rb = new Rigidbody();
        std::string new_type_name = "r" + std::to_string(n++);
        rb->key = new_type_name;
        rb->actor = this;
        luabridge::LuaRef* instance_table = new luabridge::LuaRef(ComponentDB::GetLuaState(), rb);
        ActorComponentsAdd component_to_add(this, type_name, new_type_name, instance_table);
        components_to_add.push_back(component_to_add);
        return *instance_table;
    }
    else if (type_name == "ParticleSystem") {
        ParticleSystem* ps = new ParticleSystem();
        std::string new_type_name = "r" + std::to_string(n++);
        ps->key = new_type_name;
        ps->actor = this;
        luabridge::LuaRef* instance_table = new luabridge::LuaRef(ComponentDB::GetLuaState(), ps);
        ActorComponentsAdd component_to_add(this, type_name, new_type_name, instance_table);
        components_to_add.push_back(component_to_add);
        return *instance_table;
    }

    luabridge::LuaRef* instance_table = new luabridge::LuaRef(luabridge::newTable(ComponentDB::GetLuaState()));
    ComponentDB::EstablishInheritence(*instance_table, ComponentDB::GetParentTable(type_name));
    
    std::string new_type_name = "r" + std::to_string(n++);
//    table_types[type_name].insert(new_type_name);
//    
//    instance_tables[new_type_name] = instance_table;
    ActorComponentsAdd component_to_add(this, type_name, new_type_name, instance_table);
    (*instance_table)["key"] = new_type_name;
    ComponentDB::InsertComponentVariables(*instance_table);
    components_to_add.push_back(component_to_add);
    
    return *instance_table;
}

void Actor::RemoveComponent(luabridge::LuaRef component_ref) {
    component_ref["enabled"] = false;
    std::string key = component_ref["key"];
    luabridge::LuaRef* ref = new luabridge::LuaRef(component_ref);  // heap copy
    components_to_remove[key] = ActorComponentsRemove(this, ref);
}
