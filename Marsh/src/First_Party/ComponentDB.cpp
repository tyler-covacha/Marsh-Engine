//
//  ComponentDB.cpp
//  game_engine
//
//  Created by Tyler Covacha on 3/3/26.
//

#include "ComponentDB.hpp"
#include "Actor.hpp"
#include "SceneDB.hpp"

lua_State* ComponentDB::lua_state = nullptr;

void ComponentDB::LuaInit() {
    lua_state = luaL_newstate();
    luaL_openlibs(lua_state);
    
    luabridge::getGlobalNamespace(lua_state)
        .beginNamespace("Debug")
        .addFunction("Log", &ComponentDB::CppLog)
        .addFunction("LogError", &ComponentDB::CppLogError)
        .endNamespace();
    
    InsertActorClass();
    Physics::InsertVector2(lua_state);
    Physics::InsertRigidbody(lua_state);
    ParticleSystem::InsertParticleSystem(lua_state);
}

void ComponentDB::CreateBaseTables() {
    std::string lua_scripts_path = EngineUtils::GetResourcePath(EngineUtils::lua_scripts_filepath);
    
    if (!EngineUtils::CheckDirectoryExists(lua_scripts_path)) {
        return;
    }
    
    for (const auto& lua_file : std::filesystem::directory_iterator(lua_scripts_path)) {
        std::string lua_class_name = lua_file.path().stem().string();
        if (luaL_dofile(lua_state, lua_file.path().string().c_str()) != LUA_OK) {
            std::cout << "problem with lua file " << lua_class_name;
            exit(0);
        }
        parent_tables.emplace(lua_class_name,
            new luabridge::LuaRef(luabridge::getGlobal(lua_state, lua_class_name.c_str())));
    }
}

luabridge::LuaRef& ComponentDB::GetParentTable(const std::string& lua_class_name) {
    return *parent_tables.at(lua_class_name);
}

void ComponentDB::InsertInstanceTable(const std::string& component_name, luabridge::LuaRef* instance_table, Actor& actor) {
    actor.instance_tables.emplace(component_name, instance_table);
    actor.instance_table_find.emplace(component_name, instance_table);
}

void ComponentDB::EstablishInheritence(luabridge::LuaRef& instance_table, luabridge::LuaRef& parent_table) {
    luabridge::LuaRef new_metatable = luabridge::newTable(lua_state);
    new_metatable["__index"] = parent_table;
    
    instance_table.push(lua_state);
    new_metatable.push(lua_state);
    lua_setmetatable(lua_state, -2);
    lua_pop(lua_state, 1);
}

void ComponentDB::CppLog(std::string message) {
    std::cout << message << std::endl;
}

void ComponentDB::CppLogError(std::string message) {
    std::cout << message << std::endl;
}

void ComponentDB::SetTableValue(luabridge::LuaRef& table, const std::string& key, const rapidjson::Value& val) {
    if (val.IsString())
        table[key] = std::string(val.GetString());
    else if (val.IsInt())
        table[key] = val.GetInt();
    else if (val.IsFloat())
        table[key] = val.GetFloat();
    else if (val.IsDouble())
        table[key] = val.GetDouble();
    else if (val.IsBool())
        table[key] = val.GetBool();
    else if (val.IsNull())
        table[key] = luabridge::LuaRef(table.state());
    else if (val.IsObject()) {
        luabridge::LuaRef nested = luabridge::newTable(table.state());
        for (auto& m : val.GetObject())
            SetTableValue(nested, m.name.GetString(), m.value);
        table[key] = nested;
    }
    else if (val.IsArray()) {
        luabridge::LuaRef nested = luabridge::newTable(table.state());
        int i = 1;
        for (auto& v : val.GetArray())
            SetTableValue(nested, std::to_string(i++), v);
        table[key] = nested;
    }
    else std::cout << "not valid type" << std::endl;
}

void ComponentDB::CallAllOnStart(std::vector<Actor*>& scene_actors) {
    for (Actor*& actor : scene_actors) {
        for (auto& [key, instance_table] : actor->instance_tables) {
            luabridge::LuaRef onStart = (*instance_table)["OnStart"];
            try {
                if (onStart.isFunction() && ComponentDB::ComponentIsEnabled(*instance_table) && !actor->loaded) {
                    onStart(*instance_table);
                }
            } catch (luabridge::LuaException const& e) {
                EngineUtils::ReportError(actor->name, e);
            }
        }
        actor->loaded = true;
    }
}

void ComponentDB::CallOnStart(luabridge::LuaRef* instance_table, Actor*& actor) {
    try {
        ParticleSystem* ps = (*instance_table).cast<ParticleSystem*>();
        if (ps) {
            ps->OnStart();
            return;
        }
    } catch (...) {}

    luabridge::LuaRef onStart = (*instance_table)["OnStart"];
    try {
        if (onStart.isFunction() && ComponentDB::ComponentIsEnabled(*instance_table)) {
            onStart(*instance_table);
        }
    } catch (luabridge::LuaException const& e) {
        EngineUtils::ReportError(actor->name, e);
    }
}

void ComponentDB::InsertActorClass() {
    luabridge::getGlobalNamespace(lua_state)
        .beginClass<Actor>("Actor")
        .addFunction("GetName", &Actor::GetName)
        .addFunction("GetID", &Actor::GetID)
        .addFunction("GetComponentByKey", &Actor::GetComponentByKey)
        .addFunction("GetComponent", &Actor::GetComponent)
        .addFunction("GetComponents", &Actor::GetComponents)
        .addFunction("AddComponent", &Actor::AddComponent)
        .addFunction("RemoveComponent", &Actor::RemoveComponent)
        .addStaticFunction("Find", SceneDB::Find)
        .addStaticFunction("FindAll", SceneDB::FindAll)
        .addStaticFunction("Instantiate", SceneDB::Instantiate)
        .addStaticFunction("Destroy", SceneDB::Destroy)
        .endClass();
}

void ComponentDB::InsertComponentVariables(luabridge::LuaRef& component_instance) {
    component_instance["enabled"] = true;
}

lua_State* ComponentDB::GetLuaState() {
    return lua_state;
}

bool ComponentDB::ComponentIsEnabled(luabridge::LuaRef& component_instance) {
    return component_instance["enabled"];
}
