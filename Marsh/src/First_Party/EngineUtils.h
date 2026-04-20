//
//  EngineUtils.h
//  game_engine
//
//  Created by Tyler Covacha on 1/29/26.
//

#ifndef EngineUtils_h
#define EngineUtils_h
#include <SDL2/SDL.h>
#include <string>
#include <fstream>
#include <sstream>
#include <stdio.h>
#include <iostream>
#include <filesystem>

#include "rapidjson/filereadstream.h"
#include "rapidjson/document.h"
#include "glm/glm.hpp"
#include "lua/lua.hpp"
#include "glm/glm.hpp"
#include "LuaBridge/LuaBridge.h"

class EngineUtils {
 public:
    static const glm::vec2 ZERO_VECTOR;
    static const inline std::string lua_scripts_filepath = "resources_game/component_types/";
    static const inline std::string actor_template_filepath = "resources_game/actor_templates/";
    static inline SDL_Window *window = nullptr;
    static inline glm::ivec2 window_size;
    
    static void ReadJsonFile(const std::string& path, rapidjson::Document & out_document)
    {
        std::string full_path = GetResourcePath(path);
        
        FILE* file_pointer = nullptr;
    #ifdef _WIN32
        fopen_s(&file_pointer, full_path.c_str(), "rb");
    #else
        file_pointer = fopen(full_path.c_str(), "rb");
    #endif
        if (!file_pointer) {
            std::cout << "FAILED to open: " << full_path << std::endl;
            exit(0);
        }
        char buffer[65536];
        rapidjson::FileReadStream stream(file_pointer, buffer, sizeof(buffer));
        out_document.ParseStream(stream);
        std::fclose(file_pointer);

        if (out_document.HasParseError()) {
            rapidjson::ParseErrorCode errorCode = out_document.GetParseError();
            std::cout << "error parsing json at [" << full_path << "]" << std::endl;
            exit(0);
        }
    }
    
    static bool CheckDirectoryExists(const std::string& path) {
        return std::filesystem::exists(path);
    }
    
    static uint64_t CreateCompositeKey(float x, float y, int tile_size) {
        int ix = static_cast<int>(std::floor(x/tile_size));
        int iy = static_cast<int>(std::floor(y/tile_size));
        uint32_t ux = static_cast<uint32_t>(ix);
        uint32_t uy = static_cast<uint32_t>(iy);
        return static_cast<uint64_t>(ux) << 32 | static_cast<uint64_t>(uy);
    }
    
    static std::pair<int, int> CompositeKeyToCoordinates(uint64_t key) {
        uint32_t ux = static_cast<uint32_t>((key | 0xFFFF0000) >> 32);
        uint32_t uy = static_cast<uint32_t>(key | 0x0000FFFF);
        
        std::pair<int,int> result;
        result.first = static_cast<int>(ux);
        result.second = static_cast<int>(uy);
        
        return result;
    }
    
    static std::string ObtainWordAfterPhrase(const std::string& input, const std::string& phrase) {
        size_t pos = input.find(phrase);
        if (pos == std::string::npos) return "";
        pos += phrase.length();
        while (pos < input.size() && std::isspace(input[pos])) {
            ++pos;
        }
        if (pos == input.size()) return "";
        size_t endPos = pos;
        while (endPos < input.size() && !std::isspace(input[endPos])) {
            ++endPos;
        }
        return input.substr(pos, endPos - pos);
    }
    
    static void Print(const std::string& message) {
        std::cout << message << std::endl;
    }
    
    static void ReportError(const std::string& actor_name, const luabridge::LuaException& e) {
        std::string error_message = e.what();
        
        std::replace(error_message.begin(), error_message.end(), '\\', '/');
        
        std::cout << "\033[31m" << actor_name << " : " << error_message << "\033[0m" << std::endl;
    }
    
    ///
    /// IOS FUNCTIONS
    ///
    static std::string GetResourcePath(const std::string& filename) {
    #if TARGET_OS_IOS
        const char* base = SDL_GetBasePath();
        if (base) {
            std::string result = std::string(base) + filename;
            SDL_free((void*)base);
            return result;
        }
        return filename;
    #else
        return filename;
    #endif
    }
    // Then when loading:
    //std::string full_path = GetResourcePath(GAME_CONFIG_FILEPATH);
    // full_path on iOS = "/path/to/YourApp.app/resources_game/game.config"
    // full_path on desktop = "resources_game/game.config"

};

#endif /* EngineUtils_h */
