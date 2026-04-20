//
//  SceneDB.cpp
//  game_engine
//
//  Created by Tyler Covacha on 1/29/26.
//

#include "SceneDB.hpp"

std::vector<Actor*> SceneDB::scene_actors;
std::vector<std::tuple<Actor*, luabridge::LuaRef*>> SceneDB::scene_actors_late_update;

void SceneDB::InitialLoadScene() {
    EngineUtils::ReadJsonFile("resources_game/game.config", Doc);
    
    if (!Doc.HasMember("initial_scene")) {
        std::cout << "error: initial_scene unspecified";
        exit(0);
    }
    
    scene_name = Doc["initial_scene"].GetString();
    LoadScene(scene_name);
}

void SceneDB::LoadScene(const std::string& scene_name) {
    SceneDB::scene_name = scene_name;
    
    std::string scene_file_path = EngineUtils::GetResourcePath("resources_game/scenes/" + scene_name + ".scene");
    
    if (!EngineUtils::CheckDirectoryExists(scene_file_path)) {
        std::cout << "error: scene " << scene_name << " is missing";
        exit(0);
    }
    
    EngineUtils::ReadJsonFile("resources_game/scenes/" + scene_name + ".scene", Doc);
    LoadActors();
}

void SceneDB::LoadActors() {
    Actor::components_to_add.clear();
    Actor::components_to_remove.clear();
    std::vector<Actor*> actors_dont_destroy;
    for (Actor* actor : scene_actors) {
        if (actor->dontDestroy) {
            actors_dont_destroy.push_back(actor);
        }
    }
    scene_actors.clear();
    
    for (Actor* actor : actors_dont_destroy) {
        scene_actors.push_back(actor);
    }
    
    const rapidjson::Value& scene = Doc["actors"];
    
    componentDB.CreateBaseTables();
    
    for (const rapidjson::Value& actor : scene.GetArray()) {
        Actor* new_actor = new Actor();
        
        if (CheckActorValue(actor, "name")) {
            new_actor->name = actor["name"].GetString();
        }
        
        if (CheckActorValue(actor, "template")) {
            TemplateHandling(actor, new_actor);
        }
        
        if (CheckActorValue(actor, "components")) {
            ComponentHandling(actor, new_actor);
        }
        
        new_actor->id = actor_counter++;
        scene_actors.push_back(new_actor);
    }
    
    componentDB.CallAllOnStart(scene_actors);
}

void SceneDB::CheckAllActorValues(const rapidjson::Value& actor, Actor*& new_actor) {
    if (actor.HasMember("name")) new_actor->name = actor["name"].GetString();
    
    if (CheckActorValue(actor, "components")) {
        ComponentHandling(actor, new_actor);
    }
}

void SceneDB::ComponentHandling(const rapidjson::Value& actor, Actor*& new_actor) {
    std::string table_name;
    
    for (auto& component : actor["components"].GetObject()) {
        table_name = component.name.GetString();
        
        std::string component_type = "";
        
        if (!CheckActorValue(component.value, "type")) {
            // do nothing
        }
        else {
            component_type = component.value["type"].GetString();
            
            if (component_type == "Rigidbody") {
                Rigidbody* rb = new Rigidbody();
                rb->key = component.name.GetString();
                rb->actor = new_actor;
                luabridge::LuaRef* instance_table = new luabridge::LuaRef(ComponentDB::GetLuaState(), rb);
                ComponentDB::InsertInstanceTable(component.name.GetString(), instance_table, *new_actor);
                new_actor->table_types[component_type].insert(component.name.GetString());
            }
            else if (component_type == "ParticleSystem") {
                ParticleSystem* ps = new ParticleSystem();
                ps->key = component.name.GetString();
                ps->actor = new_actor;
                luabridge::LuaRef* instance_table = new luabridge::LuaRef(ComponentDB::GetLuaState(), ps);
                ComponentDB::InsertInstanceTable(component.name.GetString(), instance_table, *new_actor);
                new_actor->table_types[component_type].insert(component.name.GetString());
            } else {
                const std::string path = EngineUtils::GetResourcePath(lua_scripts_filepath + component_type + ".lua");
                if (!EngineUtils::CheckDirectoryExists(path)) {
                    std::cout << "error: failed to locate component " << component_type;
                    exit(0);
                }
                
                luabridge::LuaRef* instance_table = new luabridge::LuaRef(luabridge::newTable(ComponentDB::GetLuaState()));
                ComponentDB::EstablishInheritence(*instance_table, ComponentDB::GetParentTable(component_type));
                (*instance_table)["key"] = component.name.GetString();
                ComponentDB::InsertComponentVariables(*instance_table);
                ComponentDB::InsertInstanceTable(component.name.GetString(), instance_table, *new_actor);
            }
        }
        
        std::string component_name = component.name.GetString();
        for (auto& component_property : component.value.GetObject()) {
            std::string component_property_name = component_property.name.GetString();
            
            if (component_property_name == "type") continue;
            
            const rapidjson::Value& component_property_value = component_property.value;
            luabridge::LuaRef& table = *(new_actor->instance_tables[component_name]);
            
            ComponentDB::SetTableValue(table, component_property_name, component_property_value);
        }
        
        if (component_type != "Rigidbody" && component_type != "ParticleSystem")
            new_actor->InjectConvenienceReferences(new_actor->instance_tables[component_name]);
        if (component_type != "")
            new_actor->table_types[component_type].insert(component_name);
    }
}

bool SceneDB::CheckActorValue(const rapidjson::Value& actor, const std::string& component) {
    return actor.HasMember(component.c_str());
}

void SceneDB::TemplateHandling(const rapidjson::Value& actor, Actor*& new_actor) {
    const std::string& template_string = actor["template"].GetString();
    const std::string template_path = EngineUtils::GetResourcePath("resources_game/actor_templates/" + template_string + ".template");
    
    if (!EngineUtils::CheckDirectoryExists(template_path)) {
        std::cout << "error: template " + template_string + " is missing";
        exit(0);
    }
    
    rapidjson::Document local_doc;
    EngineUtils::ReadJsonFile("resources_game/actor_templates/" + template_string + ".template", local_doc);
    CheckAllActorValues(local_doc, new_actor);
}

luabridge::LuaRef SceneDB::Find(const std::string& actor_name) {
    for (Actor*& actor : scene_actors) {
        if (actor->name == actor_name && !FindIfActorDestroyed(actor)) {
            return luabridge::LuaRef(ComponentDB::GetLuaState(), actor);
        }
    }
    for (Actor*& actor : scene_actors_to_be_added) {
        if (actor->name == actor_name) {
            return luabridge::LuaRef(ComponentDB::GetLuaState(), actor);
        }
    }
    return luabridge::LuaRef(ComponentDB::GetLuaState());
}

luabridge::LuaRef SceneDB::FindAll(const std::string& actor_name) {
    luabridge::LuaRef table_actors = luabridge::newTable(ComponentDB::GetLuaState());
    
    int i = 1;
    for (Actor*& actor : scene_actors) {
        if (actor->name == actor_name && !FindIfActorDestroyed(actor)) {
            table_actors[i++] = actor;
        }
    }
    for (Actor*& actor : scene_actors_to_be_added) {
        if (actor->name == actor_name) {
            table_actors[i++] = actor;
        }
    }
    
    return table_actors;
}

void SceneDB::UpdateActors() {
    scene_actors_late_update.clear();
    
    for (Actor*& actor : scene_actors) {
        if (!actor->instance_tables.empty()) {
            for (auto& [key, instance_table] : actor->instance_tables) {
                
                try {
                    ParticleSystem* ps = (*instance_table).cast<ParticleSystem*>();
                    if (ps && ps->enabled) {
                        ps->OnUpdate();
                        std::tuple<Actor*, luabridge::LuaRef*> actor_onLateUpdateTable(actor, instance_table);
                        scene_actors_late_update.push_back(actor_onLateUpdateTable);
                        continue;
                    }
                } catch (...) {}
                
                luabridge::LuaRef onUpdate = (*instance_table)["OnUpdate"];
                try {
                    if (onUpdate.isFunction() && ComponentDB::ComponentIsEnabled(*instance_table)) {
                        onUpdate(*instance_table);
                    }
                } catch (luabridge::LuaException const& e) {
                    EngineUtils::ReportError(actor->name, e);
                }
                
                luabridge::LuaRef onLateUpdate = (*instance_table)["OnLateUpdate"];
                if (onLateUpdate.isFunction()) {
                    std::tuple<Actor*, luabridge::LuaRef*> actor_onLateUpdateTable(actor, instance_table);
                    scene_actors_late_update.push_back(actor_onLateUpdateTable);
                }
            }
        }
    }
    
    for (auto& actor_table : scene_actors_late_update) {
        Actor* actor = std::get<0>(actor_table);
        luabridge::LuaRef* instance_table = std::get<1>(actor_table);
        
        try {
            ParticleSystem* ps = (*instance_table).cast<ParticleSystem*>();
            if (ps && ps->enabled) {
                ps->OnLateUpdate();
                continue;
            }
        } catch (...) {}
        
        luabridge::LuaRef onLateUpdate = (*instance_table)["OnLateUpdate"];
        try {
            if (ComponentDB::ComponentIsEnabled(*instance_table))
                onLateUpdate(*instance_table);
        } catch (luabridge::LuaException const& e) {
            EngineUtils::ReportError(actor->name, e);
        }
    }
}

void SceneDB::HandleAddedAndRemovedComponents() {
    for (auto& component_to_add : Actor::components_to_add) {
        Actor*& actor = component_to_add.actor;
        std::string& type_name = component_to_add.type_name;
        std::string& new_type_name = component_to_add.new_type_name;
        auto& table_types = actor->table_types;
        auto& instance_tables = actor->instance_tables;
        auto& instance_table = component_to_add.instance_table;
        
        table_types[type_name].insert(new_type_name);
        instance_tables[new_type_name] = instance_table;
        actor->instance_table_find[new_type_name] = instance_table;
        
        if (type_name != "Rigidbody")
            actor->InjectConvenienceReferences(instance_table);
        
        ComponentDB::CallOnStart(instance_table, actor);
    }
    
    for (auto& component_to_remove_key_component : Actor::components_to_remove) {
        std::string key = component_to_remove_key_component.first;
        auto& component_to_remove = component_to_remove_key_component.second;
        Actor*& actor = component_to_remove.actor;
        auto& instance_tables = actor->instance_tables;
        auto& instance_table = instance_tables[key];

        try {
            Rigidbody* rb = (*instance_table).cast<Rigidbody*>();
            if (rb && rb->body) {
                Physics::world->DestroyBody(rb->body);
                rb->body = nullptr;
            }
        } catch (...) {}
        
        for (auto& [type, keys] : actor->table_types) {
            keys.erase(key);
        }
        
        instance_tables.erase(key);
        actor->instance_table_find.erase(key);
    }
}

Actor* SceneDB::Instantiate(std::string actor_template_name) {
    std::string template_path = EngineUtils::GetResourcePath(
        EngineUtils::actor_template_filepath + actor_template_name + ".template"
    );
    
    if (!EngineUtils::CheckDirectoryExists(template_path)) {
        std::cout << "error: template " + actor_template_name + " is missing";
        exit(0);
    }
    
    rapidjson::Document local_doc;
    EngineUtils::ReadJsonFile(EngineUtils::actor_template_filepath + actor_template_name + ".template", local_doc);
    Actor* new_actor = new Actor();
    new_actor->id = actor_counter++;
    CheckAllActorValues(local_doc, new_actor);
    scene_actors_to_be_added.push_back(new_actor);
    return new_actor;
}

void SceneDB::Destroy(Actor* actor) {
    for (auto& [key, instance_table] : actor->instance_tables) {
        (*instance_table)["enabled"] = false;
    }
    scene_actors_to_be_destroyed.push_back(actor);
}

bool SceneDB::FindIfActorDestroyed(Actor* actor) {
    for (Actor* actor_to_be_destroyed : scene_actors_to_be_destroyed) {
        if (actor->id == actor_to_be_destroyed->id) {
            return true;
        }
    }
    return false;
}
