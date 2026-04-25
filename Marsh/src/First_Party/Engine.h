//
//  Engine.h
//  game_engine
//
//  Created by Tyler Covacha on 1/21/26.
//

#ifndef Engine_h
#define Engine_h

#ifndef PROJECT_DIR
// #error PROJECT_DIR not defined — check build settings
#endif


#include "glm/glm.hpp"
#include "EngineUtils.h"
#include "SceneDB.hpp"
#include "Actor.hpp"
#include <vector>
#include <set>
#include <string>
#include <algorithm> // for std::sort
#include "SDL2/SDL.h"
#include <SDL2_ttf/SDL_ttf.h>
#include <SDL2_image/SDL_image.h>
#include <SDL2_mixer/SDL_mixer.h>
#include "Helper.h"
#include "AudioHelper.h"
#include "TextDB.hpp"
#include "AudioDB.hpp"
#include "Rendering.hpp"
#include "Input.hpp"
#include <unordered_set>
#include "glm/glm.hpp"
#include <thread>
#include <queue>
#include <functional>
#include "box2d/box2d.h"
#include "Particles.hpp"

class DrawTextVariables {
public:
    std::string str_content;
    int x;
    int y;
    std::string font_name;
    int font_size;
    int r;
    int g;
    int b;
    int a;
    
    DrawTextVariables() {
        std::string str_content = "";
        int x = 0;
        int y = 0;
        std::string font_name = "";
        int font_size = 0;
        int r = 0;
        int g = 0;
        int b = 0;
        int a = 0;
    }
    
    DrawTextVariables(std::string _str_content, int _x, int _y, std::string _font_name, int _font_size, int _r, int _g, int _b, int _a)
    : str_content(_str_content), x(_x), y(_y), font_name(_font_name), font_size(_font_size), r(_r), g(_g), b(_b), a(_a) {}
};

class DrawImageUIVariables {
public:
    std::string image_name;
    int x = 0;
    int y = 0;
    int r = 0;
    int g = 0;
    int b = 0;
    int a = 0;
    float scale_x;
    float scale_y;
    int rendering_order;
    int id;
    
    static inline int UI_id = 0;

    DrawImageUIVariables(std::string _image_name, int _x, int _y) : image_name(_image_name), x(_x), y(_y) {
        r = 255;
        g = 255;
        b = 255;
        a = 255;
        id = UI_id++;
        rendering_order = 0;
    }
    DrawImageUIVariables(std::string _image_name, int _x, int _y, int _r, int _g, int _b, int _a, int _rendering_order, float _scale_x, float _scale_y) : image_name(_image_name), x(_x), y(_y), r(_r), g(_g), b(_b), a(_a), scale_x(_scale_x), scale_y(_scale_y) {
        id = UI_id++;
        rendering_order = _rendering_order;
    }
};

class DrawImageVariables {
public:
    std::string image_name;
    float x, y, scale_x, scale_y, pivot_x, pivot_y;
    int r, g, b, a, sorting_order, id;
    float rotation_degrees;

    static inline int draw_id = 0;

    // Image.Draw defaults
    DrawImageVariables(std::string _image_name, float _x, float _y)
        : image_name(_image_name), x(_x), y(_y), scale_x(1.0f), scale_y(1.0f),
          pivot_x(0.5f), pivot_y(0.5f), rotation_degrees(0), r(255), g(255), b(255), a(255), sorting_order(0) {
        id = draw_id++;
    }

    // Image.DrawEx
    DrawImageVariables(std::string _image_name, float _x, float _y, float _rotation_degrees, float _scale_x, float _scale_y, float _pivot_x, float _pivot_y, int _r, int _g, int _b, int _a, int _sorting_order)
        : image_name(_image_name), x(_x), y(_y), rotation_degrees(_rotation_degrees), scale_x(_scale_x), scale_y(_scale_y),
          pivot_x(_pivot_x), pivot_y(_pivot_y), r(_r), g(_g), b(_b), a(_a), sorting_order(_sorting_order) {
        id = draw_id++;
    }
};

class DrawPixelVariables {
public:
    int x, y, r, g, b, a;
    
    DrawPixelVariables(float _x, float _y, float _r, float _g, float _b, float _a)
        : x((int)_x), y((int)_y), r((int)_r), g((int)_g), b((int)_b), a((int)_a) {}
};

class Engine {
 public:
    void GameStart();
    void GameLoop();
    static inline Camera camera;
    SceneDB sceneDB;
    
    static Actor* Find(const std::string& actor_name);
    static luabridge::LuaRef FindAll(const std::string& actor_name);

 private:
    static inline std::queue<DrawTextVariables> draw_text_queue;
    static inline std::vector<DrawImageUIVariables> draw_imageUI_vector;
    static inline std::vector<DrawImageVariables> draw_image_vector;
    static inline std::vector<DrawPixelVariables> draw_pixel_vector;
    
    void Input();
    void Update();
    void Render();
    
    static std::string GAME_CONFIG_FILEPATH;
    static std::string RENDERING_CONFIG_FILEPATH;
    static std::string IMAGES_FILEPATH;
    static inline std::string AUDIO_FILEPATH = "resources_game/audio/";

    bool running = true;
    std::string current_input;
    
    std::string current_render;

    /// GamePlay Related
    /// Move to lua scripts later
    ///  Temporary Variables
    ///  Homework 3
    
    /// Homework 4
    bool loading_scene = false;
    
    ///
    /// Homework 5
    ///
    bool intro_scene = true;
    std::string font_filepath;
    
    ///
    /// Homework 6
    ///
    float zoom_factor = 1.0f;
    float cam_ease_factor = 1.0f;
    
    ///
    /// Homework 7
    ///
    std::string lua_scripts_filepath = "resources_game/component_types/";
    static inline std::string current_scene_name;
    static inline std::string next_scene_name;
    
    ///
    /// Homework 8
    ///
    static inline std::unordered_map<std::string, std::vector<std::pair<luabridge::LuaRef, luabridge::LuaRef>>> event_bus_subs;
    
    static inline std::vector<std::pair<std::string, std::pair<luabridge::LuaRef, luabridge::LuaRef>>> pending_subs;
    static inline std::vector<std::pair<std::string, std::pair<luabridge::LuaRef, luabridge::LuaRef>>> pending_unsubs;
    
    ///
    /// ---------------------------------------
    ///
    
    /// Temporary Functions
    /// Homework 3
    
    ///
    /// Homework 4 Functions
    ///
    
    static void LoadNextScene(const std::string& next_scene);
    
    ///
    /// Homework 5 Functions
    ///
    void SDLRendering();
    void SetFontFilePath(rapidjson::Document& game_config_doc);
    
    ///
    /// Homework 6 Functions
    ///
    
    void PrintAllNearbyDialogue(SDL_Surface*& text_surface, SDL_FRect& text_rect, SDL_Texture*& text_texture, SDL_Renderer*& renderer, rapidjson::Document& game_config_doc, int& y_resolution);
    
    ///
    /// Homework 7 Functions
    ///
    static void QuitProgram();
    static void SleepForMilliSeconds(int ms);
    static void OpenWebURL(std::string url);
    void VariablesToReset();
    void CallOnStart(Actor*& actor);
    static void TextDrawFuncForLua(std::string str_content, float x, float y, std::string font_name, float font_size, float r, float g, float b, float a);
    static void PlayAudio(int channel, std::string clip_name, bool does_loop);
    static void HaltAudio(int channel);
    static void SetVolume(int channel, int volume);
    static void ImageDrawUIFuncForLua(std::string image_name, float x, float y);
    static void ImageDrawExUIFuncForLua(std::string image_name, float x, float y, float r, float g, float b, float a, float sorting_order, float scale_x, float scale_y);
    static void ImageDrawFuncForLua(std::string image_name, float x, float y);
    static void ImageDrawExFuncForLua(std::string image_name, float x, float y, float rotation_degrees, float scale_x, float scale_y, float pivot_x, float pivot_y, float r, float g, float b, float a, float sorting_order);
    static void ImageDrawPixelFuncForLua(float x, float y, float r, float g, float b, float a);
    static void SetCameraPosition(float x, float y);
    static void SetCameraRotation(float degrees);
    static float GetCameraPositionX();
    static float GetCameraPositionY();
    static void SetCameraZoomFactor(float set_zoom_factor);
    static float GetCameraZoomFactor();
    static std::string GetCurrentSceneName();
    static void DontDestroyActor(Actor* actor);
    
    static void EventBusPublish(std::string event_type, luabridge::LuaRef event_object);
    static void EventBusSubscribe(std::string event_type, luabridge::LuaRef component, luabridge::LuaRef function);
    static void EventBusUnSubscribe(std::string event_type, luabridge::LuaRef component, luabridge::LuaRef function);
    static void FlushEventBus();
    
    
    ///
    /// IOS FUNCTIONS
    ///
};

#endif /* Engine_h */
