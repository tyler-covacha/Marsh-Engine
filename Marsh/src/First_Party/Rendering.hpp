//
//  Rendering.hpp
//  game_engine
//
//  Created by Tyler Covacha on 2/10/26.
//

#ifndef Rendering_hpp
#define Rendering_hpp

#include <stdio.h>
#include <string>
#include <vector>
#include "rapidjson/filereadstream.h"
#include "rapidjson/document.h"
#include "EngineUtils.h"
#include <iostream>
#include <stdint.h>
#include "SDL2_image/SDL_image.h"
#include "SDL2/SDL.h"
#include "TextDB.hpp"
#include "glm/glm.hpp"
#include "Helper.h"
#include <cmath>

struct Camera {
 public:
    glm::vec2 position;
    int width;
    int height;
    float rotation;
    float zoom_factor;
    
    Camera() {
        position.x = 0;
        position.y = 0;
        width = 640;  // pixels
        height = 360; // pixels
        zoom_factor = 1.0f;
        rotation = 0.0f;
    }
};

class Rendering {
 public:
    static inline std::unordered_map<std::string, SDL_Texture*> images;
    static inline std::unordered_map<std::string, SDL_Texture*> texture_bank;
    static inline SDL_Renderer* renderer;
    
    static void SetRenderingConfig(std::string& RENDERING_CONFIG_FILEPATH, rapidjson::Document& rendering_config_doc, int& x, int& y, Uint8& r, Uint8& g, Uint8& b, glm::vec2& camera_offset, float& zoom_factor, float& cam_ease_factor) {
        EngineUtils::ReadJsonFile(RENDERING_CONFIG_FILEPATH, rendering_config_doc);
        if (rendering_config_doc.HasMember("x_resolution")) x = rendering_config_doc["x_resolution"].GetInt();
        if (rendering_config_doc.HasMember("y_resolution")) y = rendering_config_doc["y_resolution"].GetInt();
        if (rendering_config_doc.HasMember("clear_color_r")) r = rendering_config_doc["clear_color_r"].GetInt();
        if (rendering_config_doc.HasMember("clear_color_g")) g = rendering_config_doc["clear_color_g"].GetInt();
        if (rendering_config_doc.HasMember("clear_color_b")) b = rendering_config_doc["clear_color_b"].GetInt();
        if (rendering_config_doc.HasMember("cam_offset_x")) camera_offset.x = rendering_config_doc["cam_offset_x"].GetFloat();
        if (rendering_config_doc.HasMember("cam_offset_y")) camera_offset.y = rendering_config_doc["cam_offset_y"].GetFloat();
        if (rendering_config_doc.HasMember("zoom_factor")) zoom_factor = rendering_config_doc["zoom_factor"].GetFloat();
        if (rendering_config_doc.HasMember("cam_ease_factor")) cam_ease_factor = rendering_config_doc["cam_ease_factor"].GetFloat();
//        if (rendering_config_doc.HasMember("x_scale_actor_flipping_on_movement")) x_scale_actor_flipping_on_movement = rendering_config_doc["x_scale_actor_flipping_on_movement"].GetBool();
    }
    
    static void LoadAllImages(SDL_Renderer* renderer) {
        std::string images_dir = EngineUtils::GetResourcePath("resources_game/images/");
        
        if (!EngineUtils::CheckDirectoryExists(images_dir)) return;
        
        for (const auto& entry : std::filesystem::directory_iterator(images_dir)) {
            std::string filename = entry.path().stem().string();  // "player"
            std::string filepath = entry.path().string();
            
            SDL_Texture* image_texture = IMG_LoadTexture(renderer, filepath.c_str());
            if (image_texture)
                images[filename] = image_texture;
        }
    }
    
    static void DrawImageUI(std::string image_name, int x, int y, int r, int g, int b, int a, float scale_x, float scale_y, SDL_Renderer* renderer) {
        
//        std::string image_filepath = "resources_game/images/" + image_name + ".png";
        SDL_Texture* image_texture = images[image_name];
        SDL_SetTextureColorMod(image_texture, r, g, b);
        SDL_SetTextureAlphaMod(image_texture, a);

        float w, h;
        Helper::SDL_QueryTexture(image_texture, &w, &h);
        float scaled_w = w * glm::abs(scale_x);
        float scaled_h = h * glm::abs(scale_y);
        SDL_FRect image_rect = {(float)x, (float)y, scaled_w, scaled_h};
            
//        Helper::SDL_RenderCopy(renderer, image_texture, NULL, &image_rect);
        Helper::SDL_RenderCopyEx(-1, "", renderer, image_texture, NULL, &image_rect, 0, NULL, SDL_FLIP_NONE);
    }
    
    static void DrawImage(std::string image_name, float x, float y, float rotation_degrees, float scale_x, float scale_y, float pivot_x, float pivot_y, int r, int g, int b, int a, int sorting_order, SDL_Renderer* renderer, Camera& camera) {
        SDL_Texture* texture = images[image_name];

        float w, h;
        Helper::SDL_QueryTexture(texture, &w, &h);

        float scaled_w = w * glm::abs(scale_x);
        float scaled_h = h * glm::abs(scale_y);

        float screen_x = (x - camera.position.x) * 100.0f + (camera.width / 2.0f) / camera.zoom_factor;
        float screen_y = (y - camera.position.y) * 100.0f + (camera.height / 2.0f) / camera.zoom_factor;

        // Apply camera rotation to position
        if (camera.rotation != 0.0f) {
            float center_x = (camera.width / 2.0f) / camera.zoom_factor;
            float center_y = (camera.height / 2.0f) / camera.zoom_factor;
            
            float dx = screen_x - center_x;
            float dy = screen_y - center_y;
            
            float rad = camera.rotation * (M_PI / 180.0f);
            float cos_angle = cos(rad);
            float sin_angle = sin(rad);
            
            screen_x = center_x + (dx * cos_angle - dy * sin_angle);
            screen_y = center_y + (dx * sin_angle + dy * cos_angle);
        }

        SDL_FPoint pivot = {
            pivot_x * scaled_w,
            pivot_y * scaled_h
        };

        SDL_FRect rect = {
            screen_x - pivot.x,
            screen_y - pivot.y,
            scaled_w,
            scaled_h
        };

        // Determine flip based on negative scale
        SDL_RendererFlip flip = SDL_FLIP_NONE;
        if (scale_x < 0 && scale_y < 0)
            flip = (SDL_RendererFlip)(SDL_FLIP_HORIZONTAL | SDL_FLIP_VERTICAL);
        else if (scale_x < 0)
            flip = SDL_FLIP_HORIZONTAL;
        else if (scale_y < 0)
            flip = SDL_FLIP_VERTICAL;

        SDL_SetTextureColorMod(texture, r, g, b);
        SDL_SetTextureAlphaMod(texture, a);

        // Add camera rotation to sprite rotation
        float total_rotation = rotation_degrees + camera.rotation;
        
        Helper::SDL_RenderCopyEx(-1, "", renderer, texture, NULL, &rect, total_rotation, &pivot, flip);
    }
    
    static void CreateDefaultParticleTextureWithName(const std::string& name) {
        if (texture_bank.find(name) != texture_bank.end())
            return;
        
        SDL_Surface* surface = SDL_CreateRGBSurfaceWithFormat(0, 8, 8, 32, SDL_PIXELFORMAT_RGBA8888);
        
        Uint32 white_color = SDL_MapRGBA(surface->format, 255, 255, 255, 255);
        SDL_FillRect(surface, NULL, white_color);
        
        SDL_Texture* texture = SDL_CreateTextureFromSurface(renderer, surface);
        
        SDL_FreeSurface(surface);
        texture_bank[name] = texture;
    }
};

#endif /* Rendering_hpp */
