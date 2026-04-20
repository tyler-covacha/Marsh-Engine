//
//  main.cpp
//  game_engine
//  Comment to publish Windows build #1
//  Created by Tyler Covacha on 1/21/26.
//
#undef main
#include <TargetConditionals.h>
#include <stdio.h>
#include <iostream>
#include "Engine.h"
#include "EngineUtils.h"
#include "lua/lua.hpp"
#include "LuaBridge/LuaBridge.h"
#include "ComponentDB.hpp"
#include <SDL2/SDL.h>

void DirectoryChecking();
void FindResources(const std::string& searchPath);

extern "C" int SDL_main(int argc, char* argv[]) {
    SDL_Init(SDL_INIT_VIDEO);
    DirectoryChecking();
    Engine engine;
    engine.GameStart();
    engine.GameLoop();
    return 0;
}

#if !TARGET_OS_IOS
int main(int argc, char* argv[]) {
    DirectoryChecking();
    Engine engine;
    engine.GameStart();
    engine.GameLoop();
    return 0;
}
#endif

void DirectoryChecking() {
    std::string resourcesPath = EngineUtils::GetResourcePath("resources_game");
    std::string configPath = EngineUtils::GetResourcePath("resources_game/game.config");

    if (!EngineUtils::CheckDirectoryExists(resourcesPath)) {
        std::cout << resourcesPath << std::endl;
        std::cout << "error: resources_game/ missing";
        exit(0);
    }
    if (!EngineUtils::CheckDirectoryExists(configPath)) {
        std::cout << "error: resources_game/game.config missing";
        exit(0);
    }
}

void FindResources(const std::string& searchPath) {
    for (const auto& entry : std::filesystem::recursive_directory_iterator(searchPath)) {
        if (entry.path().filename() == "resources_game") {
            std::cout << "Found resources at: " << entry.path() << std::endl;
        }
    }
}
