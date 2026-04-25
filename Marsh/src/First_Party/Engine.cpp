//
//  Engine.cpp
//  game_engine
//
//  Created by Tyler Covacha on 1/21/26.
//

#include <stdio.h>
#include <iostream>
#include "Engine.h"
#include <chrono>

std::string Engine::GAME_CONFIG_FILEPATH = "resources_game/game.config";
std::string Engine::RENDERING_CONFIG_FILEPATH = "resources_game/rendering.config";
std::string Engine::IMAGES_FILEPATH = "resources_game/images/";

void Engine::GameStart() {
    TTF_Init();
    AudioHelper::Mix_OpenAudio(44100, MIX_DEFAULT_FORMAT, 2, 2048);
    Input::Init();
    AudioDB::IncreaseAudioChannels();
    sceneDB.componentDB.LuaInit();
    
    luabridge::getGlobalNamespace(ComponentDB::GetLuaState())
        .beginNamespace("Application")
        .addFunction("GetFrame", &Helper::GetFrameNumber)
        .addFunction("Quit", &Engine::QuitProgram)
        .addFunction("Sleep", &Engine::SleepForMilliSeconds)
        .addFunction("OpenURL", &Engine::OpenWebURL)
        .addFunction("GetResourcePath", &EngineUtils::GetResourcePath)
        .endNamespace()
        .beginNamespace("Input")
        .addFunction("GetKey", &Input::GetKey)
        .addFunction("GetKeyDown", &Input::GetKeyDown)
        .addFunction("GetKeyUp", &Input::GetKeyUp)
        .addFunction("GetMousePosition", &Input::GetMousePosition)
        .addFunction("GetMouseButton", &Input::GetMouseButton)
        .addFunction("GetMouseButtonDown", &Input::GetMouseButtonDown)
        .addFunction("GetMouseButtonUp", &Input::GetMouseButtonUp)
        .addFunction("GetFingerUp", &Input::GetFingerUp)
        .addFunction("GetFingerDown", &Input::GetFingerDown)
        .addFunction("GetFingerPosition", &Input::GetFingerPosition)
        .addFunction("GetMouseScrollDelta", &Input::GetMouseScrollDelta)
        .addFunction("HideCursor", &Input::HideCursor)
        .addFunction("ShowCursor", &Input::ShowCursor)
        .addFunction("GetTap", &Input::GetTap)
        .addFunction("GetSwipe", &Input::GetSwipe)
        .addFunction("GetDoubleTap", &Input::GetDoubleTap)
        .addFunction("GetLongPress", &Input::GetLongPress)
        .addFunction("GetPinchScale", &Input::GetPinchScale)
        .addFunction("IsPinching", &Input::IsPinching)
        .addFunction("GetRotationAngle", &Input::GetRotationAngle)
        .addFunction("IsRotating", &Input::IsRotating)
        .endNamespace()
        .beginClass<glm::vec2>("vec2")
        .addProperty("x", &glm::vec2::x)
        .addProperty("y", &glm::vec2::y)
        .endClass()
        .beginNamespace("Text")
        .addFunction("Draw", &TextDrawFuncForLua)
        .endNamespace()
        .beginNamespace("Audio")
        .addFunction("Play", &PlayAudio)
        .addFunction("Halt", &HaltAudio)
        .addFunction("SetVolume", &SetVolume)
        .endNamespace()
        .beginNamespace("Image")
        .addFunction("DrawUI", &ImageDrawUIFuncForLua)
        .addFunction("DrawUIEx", &ImageDrawExUIFuncForLua)
        .addFunction("Draw", &ImageDrawFuncForLua)
        .addFunction("DrawEx", &ImageDrawExFuncForLua)
        .addFunction("DrawPixel", &ImageDrawPixelFuncForLua)
        .endNamespace()
        .beginNamespace("Camera")
        .addFunction("SetPosition", &SetCameraPosition)
        .addFunction("GetPositionX", &GetCameraPositionX)
        .addFunction("GetPositionY", &GetCameraPositionY)
        .addFunction("SetZoom", &SetCameraZoomFactor)
        .addFunction("GetZoom", &GetCameraZoomFactor)
        .addFunction("SetRotation", &Engine::SetCameraRotation)
        .endNamespace()
        .beginNamespace("Scene")
        .addFunction("Load", &LoadNextScene)
        .addFunction("GetCurrent", &GetCurrentSceneName)
        .addFunction("DontDestroy", &DontDestroyActor)
        .endNamespace()
        .beginNamespace("Event")
        .addFunction("Publish", &EventBusPublish)
        .addFunction("Subscribe", &EventBusSubscribe)
        .addFunction("Unsubscribe", &EventBusUnSubscribe)
        .endNamespace();
        
    b2Vec2* vec2 = new b2Vec2();
}

void Engine::LoadNextScene(const std::string& next_scene) {
    next_scene_name = next_scene;
}

void Engine::SDLRendering() {
    const char* title = "";
    int x = 640;
    int y = 360;
    
    // Game Config Doc
    rapidjson::Document game_config_doc;
    EngineUtils::ReadJsonFile(GAME_CONFIG_FILEPATH, game_config_doc);
    if (game_config_doc.HasMember("game_title")) title = game_config_doc["game_title"].GetString();
    
    // Rendering Config Doc
    rapidjson::Document rendering_config_doc;
    Uint8 r = 255;
    Uint8 g = 255;
    Uint8 b = 255;
    Uint8 a = 255;
    glm::vec2 camera_offset;
    
    std::string rendering_path = EngineUtils::GetResourcePath(RENDERING_CONFIG_FILEPATH);
    if (EngineUtils::CheckDirectoryExists(rendering_path)){
        Rendering::SetRenderingConfig(RENDERING_CONFIG_FILEPATH, rendering_config_doc, x, y, r, g, b, camera_offset, zoom_factor, cam_ease_factor);
    }
    camera.zoom_factor = zoom_factor;
    camera.width = x;
    camera.height = y;
    
    // IPhone Window
//#ifdef __IPHONEOS__
//    EngineUtils::window = Helper::SDL_CreateWindow(
//        title,
//        SDL_WINDOWPOS_UNDEFINED,
//        SDL_WINDOWPOS_UNDEFINED,
//        0, 0,
//        SDL_WINDOW_FULLSCREEN | SDL_WINDOW_ALLOW_HIGHDPI | SDL_WINDOW_BORDERLESS | SDL_WINDOW_METAL | SDL_WINDOW_SHOWN
//    );
//    
//    SDL_DisplayMode mode;
//    SDL_GetCurrentDisplayMode(0, &mode);
//    camera.width = mode.w;
//    camera.height = mode.h;
//    std::cout << camera.width << " " << camera.height << std::endl;
//    x = mode.w;
//    y = mode.h;
//    
//    SDL_Window *window = EngineUtils::window;
//#else
//    SDL_Window *window = Helper::SDL_CreateWindow(title, SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED, x, y, SDL_WINDOW_SHOWN | SDL_WINDOW_BORDERLESS | SDL_WINDOW_METAL);
//#endif
    
    // Normal Windows
    EngineUtils::window = Helper::SDL_CreateWindow(title, SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED, x, y, SDL_WINDOW_SHOWN | SDL_WINDOW_BORDERLESS | SDL_WINDOW_METAL);
    SDL_GetWindowSize(EngineUtils::window, &EngineUtils::window_size.x, &EngineUtils::window_size.y);
    SDL_Window* window = EngineUtils::window;
    
    Rendering::renderer = SDL_CreateRenderer(window, -1, SDL_RENDERER_ACCELERATED);
    SDL_Renderer*& renderer = Rendering::renderer;

    Rendering::LoadAllImages(renderer);
    std::vector<std::string> intro_text;
    
    sceneDB.InitialLoadScene();
    
    bool running = true;
    while (running){
        SDL_Event next_event;
        
        Input::StartOfFrame();
        while (Helper::SDL_PollEvent(&next_event)) {
            Input::ProcessEvent(next_event);
            if (next_event.type == SDL_QUIT) {
                running = false;
            }
        }
        
        Update();
        FlushEventBus();
        Physics::Step();
        
        SDL_SetRenderDrawColor(renderer, r, g, b, a);
        SDL_RenderClear(renderer);
        
        SDL_RenderSetScale(renderer, camera.zoom_factor, camera.zoom_factor);

        if (!draw_image_vector.empty()){
            std::stable_sort(draw_image_vector.begin(), draw_image_vector.end(),
                             [](const DrawImageVariables& a, const DrawImageVariables& b) {
                if (a.sorting_order != b.sorting_order) return a.sorting_order < b.sorting_order;
                return a.id < b.id;
            });
        }
        if (!ParticleSystem::particles_to_render.empty()){
            std::stable_sort(ParticleSystem::particles_to_render.begin(), ParticleSystem::particles_to_render.end(),
                [](const DrawParticleVariables& a, const DrawParticleVariables& b) {
                    if (a.sorting_order != b.sorting_order) return a.sorting_order < b.sorting_order;
                    return a.id < b.id;
                });
        }
        
        int img_i = 0, par_i = 0;
        while (img_i < (int)draw_image_vector.size() || par_i < (int)ParticleSystem::particles_to_render.size()) {
            bool use_image;
            if (img_i >= (int)draw_image_vector.size()) use_image = false;
            else if (par_i >= (int)ParticleSystem::particles_to_render.size()) use_image = true;
            else {
                auto& img = draw_image_vector[img_i];
                auto& par = ParticleSystem::particles_to_render[par_i];
                if (img.sorting_order != par.sorting_order)
                    use_image = img.sorting_order < par.sorting_order;
                else
                    use_image = img.id < par.id;
            }

            if (use_image) {
                auto& d = draw_image_vector[img_i++];
                Rendering::DrawImage(d.image_name, d.x, d.y, d.rotation_degrees, d.scale_x, d.scale_y,
                                     d.pivot_x, d.pivot_y, d.r, d.g, d.b, d.a, d.sorting_order, renderer, camera);
            } else {
                auto& dpv = ParticleSystem::particles_to_render[par_i++];
                
                if (dpv.particle_texture == nullptr) continue;
                
                float screen_x = (dpv.x - camera.position.x) * 100.0f + (static_cast<float>(camera.width) / 2.0f) / camera.zoom_factor;
                float screen_y = (dpv.y - camera.position.y) * 100.0f + (static_cast<float>(camera.height) / 2.0f) / camera.zoom_factor;
                
                SDL_FRect rect = {
                    screen_x - (dpv.w * 0.5f),
                    screen_y - (dpv.h * 0.5f),
                    dpv.w, dpv.h
                };
                SDL_SetTextureColorMod(dpv.particle_texture, dpv.r, dpv.g, dpv.b);
                SDL_SetTextureAlphaMod(dpv.particle_texture, dpv.a);
                SDL_FPoint pivot = { dpv.w * 0.5f, dpv.h * 0.5f };
                Helper::SDL_RenderCopyEx(-1, "", renderer, dpv.particle_texture, nullptr, &rect, dpv.rotation, &pivot, SDL_FLIP_NONE);
            }
        }

        draw_image_vector.clear();
        ParticleSystem::particles_to_render.clear();
        
        SDL_RenderSetScale(renderer, 1.0f, 1.0f);
        std::stable_sort(draw_imageUI_vector.begin(), draw_imageUI_vector.end(),
            [](const DrawImageUIVariables& a, const DrawImageUIVariables& b) {
                if (a.rendering_order != b.rendering_order)
                    return a.rendering_order < b.rendering_order;
                return a.id < b.id;
            });

        for (auto& d : draw_imageUI_vector) {
            Rendering::DrawImageUI(d.image_name, d.x, d.y, d.r, d.g, d.b, d.a, d.scale_x, d.scale_y, renderer);
        }
        draw_imageUI_vector.clear();
        
        while (!draw_text_queue.empty()) {
            DrawTextVariables& drawTextVariables = draw_text_queue.front();
            TextDB::DrawText(drawTextVariables.str_content, drawTextVariables.x, drawTextVariables.y, drawTextVariables.font_name, drawTextVariables.font_size, drawTextVariables.r, drawTextVariables.g, drawTextVariables.b, drawTextVariables.a, renderer);
            draw_text_queue.pop();
        }
        
        SDL_SetRenderDrawBlendMode(renderer, SDL_BLENDMODE_BLEND);
        for (auto& p : draw_pixel_vector) {
            SDL_SetRenderDrawColor(renderer, p.r, p.g, p.b, p.a);
            SDL_RenderDrawPoint(renderer, p.x, p.y);
        }
        draw_pixel_vector.clear();
        SDL_SetRenderDrawBlendMode(renderer, SDL_BLENDMODE_NONE);
        
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

            luabridge::LuaRef onDestroy = (*instance_table)["OnDestroy"];
            try {
                if (onDestroy.isFunction())
                    onDestroy(*instance_table);
            } catch (luabridge::LuaException const& e) {
                EngineUtils::ReportError(actor->name, e);
            }
        }
        
        for (Actor* actor_destroy : sceneDB.scene_actors_to_be_destroyed) {
            for (auto& [key, instance_table] : actor_destroy->instance_tables) {
                try {
                    Rigidbody* rb = (*instance_table).cast<Rigidbody*>();
                    if (rb && rb->body) {
                        Physics::world->DestroyBody(rb->body);
                        rb->body = nullptr;
                    }
                } catch (...) {}
                
                luabridge::LuaRef onDestroy = (*instance_table)["OnDestroy"];
                try {
                    if (onDestroy.isFunction()) {
                        onDestroy(*instance_table);
                    }
                } catch (luabridge::LuaException const& e) {
                    EngineUtils::ReportError(actor_destroy->name, e);
                }
            }
        }
        
        Helper::SDL_RenderPresent(renderer);
        Input::LateUpdate();
        
        SceneDB::HandleAddedAndRemovedComponents();
        
        if (!next_scene_name.empty()) {
            std::string scene_to_load = next_scene_name;
            next_scene_name.clear();
            sceneDB.LoadScene(scene_to_load);
        }
        VariablesToReset();
    }
    
    SDL_DestroyRenderer(renderer);
    SDL_DestroyWindow(window);
    if (TextDB::font)
        TextDB::CloseFont();
    SDL_Quit();
}

void Engine::SetFontFilePath(rapidjson::Document& game_config_doc) {
    font_filepath = "resources_game/fonts/" + std::string(game_config_doc["font"].GetString()) + ".ttf";
}

void Engine::GameLoop() {
    SDLRendering();
}

void Engine::Update() {
    sceneDB.UpdateActors();
}

void Engine::QuitProgram() {
    exit(0);
}

void Engine::SleepForMilliSeconds(int ms) {
    std::this_thread::sleep_for(std::chrono::milliseconds(ms));
}

void Engine::OpenWebURL(std::string url) {
    std::string command = "";
#if defined(__IPHONEOS__)
    SDL_OpenURL(url.c_str());
#elif defined(WIN32) || defined(_WIN32) || defined(__WIN32__) || defined(__NT__)
    command = "start " + url;
    std::system(command.c_str());
#elif __APPLE__
    command = "open " + url;
    std::system(command.c_str());
#elif __linux__
    command = "xdg-open " + url;
    std::system(command.c_str());
#else
    #error "Unknown operating system, cannot open URL"
#endif
}

void Engine::VariablesToReset() {
    Actor::components_to_add.clear();
    Actor::components_to_remove.clear();
    
    for (Actor*& actor : sceneDB.scene_actors_to_be_added) {
        sceneDB.scene_actors.push_back(actor);
        CallOnStart(actor);
    }
    sceneDB.scene_actors_to_be_added.clear();
    
    for (Actor* actor_destroy : sceneDB.scene_actors_to_be_destroyed) {
        sceneDB.scene_actors.erase(
            std::remove_if(sceneDB.scene_actors.begin(), sceneDB.scene_actors.end(),
                [actor_destroy](Actor* actor) {
                    return actor->id == actor_destroy->id;
                }),
            sceneDB.scene_actors.end()
        );
    }
    sceneDB.scene_actors_to_be_destroyed.clear();
}

void Engine::CallOnStart(Actor*& actor) {
    for (auto& [key, instance_table] : actor->instance_tables) {
        ComponentDB::CallOnStart(instance_table, actor);
    }
}

void Engine::TextDrawFuncForLua(std::string str_content, float x, float y, std::string font_name, float font_size, float r, float g, float b, float a) {
    DrawTextVariables drawTextVariables(str_content, x, y, font_name, font_size, r, g, b, a);
    draw_text_queue.push(drawTextVariables);
}

void Engine::PlayAudio(int channel, std::string clip_name, bool does_loop) {
    if (AudioDB::audio_chunks.find(clip_name) == AudioDB::audio_chunks.end())
    {
        std::string file_path = AudioDB::CheckAudioFileExists(clip_name);
        Mix_Chunk* chunk = AudioHelper::Mix_LoadWAV(file_path.c_str());
        AudioDB::audio_chunks[clip_name] = chunk;
    }
    int loop = does_loop ? -1 : 0;
    AudioDB::PlayAudio(AudioDB::audio_chunks[clip_name], channel, loop);
}

void Engine::HaltAudio(int channel) {
    AudioDB::StopAudio(channel);
}

void Engine::SetVolume(int channel, int volume) {
    AudioDB::AdjustVolume(channel, volume);
}

void Engine::ImageDrawUIFuncForLua(std::string image_name, float x, float y) {
    DrawImageUIVariables image(image_name, (int)x, (int)y);
    draw_imageUI_vector.push_back(image);
}

void Engine::ImageDrawExUIFuncForLua(std::string image_name, float x, float y, float r, float g, float b, float a, float sorting_order, float scale_x, float scale_y) {
    DrawImageUIVariables image(image_name, (int)x, (int)y, (int)r, (int)g, (int)b, (int)a, (int)sorting_order, scale_x, scale_y);
    draw_imageUI_vector.push_back(image);
}

void Engine::ImageDrawFuncForLua(std::string image_name, float x, float y) {
    DrawImageVariables image(image_name, x, y);
    draw_image_vector.push_back(image);
}

void Engine::ImageDrawExFuncForLua(std::string image_name, float x, float y, float rotation_degrees, float scale_x, float scale_y, float pivot_x, float pivot_y, float r, float g, float b, float a, float sorting_order) {
    DrawImageVariables image(image_name, x, y, (int)rotation_degrees, scale_x, scale_y, pivot_x, pivot_y, (int)r, (int)g, (int)b, (int)a, (int)sorting_order);
    draw_image_vector.push_back(image);
}

void Engine::ImageDrawPixelFuncForLua(float x, float y, float r, float g, float b, float a) {
    draw_pixel_vector.push_back(DrawPixelVariables((int)x, (int)y, (int)r, (int)g, (int)b, (int)a));
}

void Engine::SetCameraPosition(float x, float y){
    camera.position.x = x;
    camera.position.y = y;
}

float Engine::GetCameraPositionX() {
    return camera.position.x;
}

float Engine::GetCameraPositionY() {
    return camera.position.y;
}

void Engine::SetCameraZoomFactor(float set_zoom_factor) {
    camera.zoom_factor = set_zoom_factor;
}

float Engine::GetCameraZoomFactor() {
    return camera.zoom_factor;
}

std::string Engine::GetCurrentSceneName() {
    return SceneDB::scene_name;
}

void Engine::DontDestroyActor(Actor* actor) {
    actor->dontDestroy = true;
}

void Engine::EventBusPublish(std::string event_type, luabridge::LuaRef event_object) {
    auto it = event_bus_subs.find(event_type);
    if (it == event_bus_subs.end()) return;
    for (auto& [component_ref, function] : it->second) {
        if (!component_ref.isNil()) {
            function(component_ref, event_object);
        }
    }
}

void Engine::EventBusSubscribe(std::string event_type, luabridge::LuaRef component, luabridge::LuaRef function) {
    pending_subs.push_back({event_type, {component, function}});
}

void Engine::EventBusUnSubscribe(std::string event_type, luabridge::LuaRef component, luabridge::LuaRef function) {
    pending_unsubs.push_back({event_type, {component, function}});
}

void Engine::FlushEventBus() {
    for (auto& [event_type, pair] : pending_subs)
        event_bus_subs[event_type].push_back(pair);
    pending_subs.clear();

    for (auto& [event_type, pair] : pending_unsubs) {
        auto it = event_bus_subs.find(event_type);
        if (it == event_bus_subs.end()) continue;
        auto& vec = it->second;
        for (auto it2 = vec.begin(); it2 != vec.end(); ) {
            if (it2->first == pair.first && it2->second == pair.second)
                it2 = vec.erase(it2);
            else
                ++it2;
        }
    }
    pending_unsubs.clear();
}

void Engine::SetCameraRotation(float degrees) {
    camera.rotation = degrees;
}

