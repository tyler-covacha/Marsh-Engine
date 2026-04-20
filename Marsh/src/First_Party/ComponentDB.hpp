//
//  ComponentDB.hpp
//  game_engine
//
//  Created by Tyler Covacha on 3/3/26.
//

#ifndef ComponentDB_hpp
#define ComponentDB_hpp

#include <stdio.h>
#include <unordered_map>
#include <filesystem>
#include "lua/lua.hpp"
#include "LuaBridge/LuaBridge.h"
#include "EngineUtils.h"
#include "Helper.h"
#include "TextDB.hpp"
#include "box2d/box2d.h"
#include "Physics.hpp"
#include "Particles.hpp"
//#include "Actor.hpp"

class Actor;
class SceneDB;


class ComponentDB {
 private:
    static inline std::unordered_map<std::string, luabridge::LuaRef*> parent_tables;
    static lua_State* lua_state;
    
 public:
    void LuaInit();
    void CreateBaseTables();
    static luabridge::LuaRef& GetParentTable(const std::string& lua_class_name);
    static void InsertInstanceTable(const std::string& component_name, luabridge::LuaRef* instance_table, Actor& actor);
    static void EstablishInheritence(luabridge::LuaRef & instance_table, luabridge::LuaRef & parent_table);
    
    static void CppLog(std::string message);
    static void CppLogError(std::string message);
    static void SetTableValue(luabridge::LuaRef& table, const std::string& key, const rapidjson::Value& val);
    void CallAllOnStart(std::vector<Actor*>& scene_actors);
    static void CallOnStart(luabridge::LuaRef* instance_table, Actor*& actor);
    void InsertActorClass();
    static lua_State* GetLuaState();
    void static InsertComponentVariables(luabridge::LuaRef& component_instance);
    static bool ComponentIsEnabled(luabridge::LuaRef& component_instance);
    
};

#endif /* ComponentDB_hpp */
