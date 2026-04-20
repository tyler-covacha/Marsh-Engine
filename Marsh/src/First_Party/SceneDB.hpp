//
//  SceneDB.hpp
//  game_engine
//
//  Created by Tyler Covacha on 1/29/26.
//

#ifndef SceneDB_hpp
#define SceneDB_hpp

#include <stdio.h>
#include <string>
#include <unordered_map>
#include <vector>
#include <cstdint>
#include "EngineUtils.h"
#include "Actor.hpp"
#include "SDL2_image/SDL_image.h"
#include "SDL2/SDL.h"
#include "Helper.h"
#include <unordered_set>
#include <algorithm>
#include "AudioDB.hpp"
#include <set>
#include "ComponentDB.hpp"
#include <tuple>
#include "Particles.hpp"

#pragma once

class SceneDB {
 public:
    static inline std::string scene_name;
    static std::vector<Actor*> scene_actors;
//    static std::unordered_set<Actor*> actors_dont_destroy;
    static inline std::vector<Actor*> scene_actors_to_be_added;
    static inline std::vector<Actor*> scene_actors_to_be_destroyed;
    static std::vector<std::tuple<Actor*, luabridge::LuaRef*>> scene_actors_late_update;
    std::vector<Actor*> actors_to_render;
    std::unordered_map<std::string, SDL_Texture*> view_images;
    Actor* player_actor = nullptr;
    static inline rapidjson::Document Doc;
    static inline rapidjson::Document Template_Doc;
    static inline int actor_counter = 0;
    static inline const std::string lua_scripts_filepath = "resources_game/component_types/";
    
    ComponentDB componentDB;
    
    void InitialLoadScene();
    void LoadScene(const std::string& scene_name);
    void LoadActors();
    static void CheckAllActorValues(const rapidjson::Value& actor, Actor*& new_actor);
    static bool CheckActorValue(const rapidjson::Value& actor, const std::string& component);
    void UpdateScene(uint64_t& old_key, uint64_t& new_key, Actor& actor);
    
    void TemplateHandling(const rapidjson::Value& actor, Actor*& new_actor);
    
    void LoadActorViewImagesWithAnImage();
    void FindDefaultPivotOffsets();
    
    void SortActorsToRender();
    
    static void ComponentHandling(const rapidjson::Value& actor, Actor*& new_actor);
    
    static luabridge::LuaRef Find(const std::string& actor_name);
    static luabridge::LuaRef FindAll(const std::string& actor_name);
    
    void StartActors();
    void UpdateActors();
    
    static void HandleAddedAndRemovedComponents();
    static Actor* Instantiate(std::string actor_template_name);
    static void Destroy(Actor* actor);
    static bool FindIfActorDestroyed(Actor* actor);
 private:
};

#endif /* SceneDB_hpp */
