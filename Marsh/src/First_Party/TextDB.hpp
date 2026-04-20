 //
//  TextDB.hpp
//  game_engine
//
//  Created by Tyler Covacha on 2/7/26.
//

#ifndef TextDB_hpp
#define TextDB_hpp

#include <stdio.h>
#include <string>
#include "SDL2/SDL.h"
#include <SDL2_ttf/SDL_ttf.h>
#include <SDL2_image/SDL_image.h>
#include <SDL2_mixer/SDL_mixer.h>
#include <iostream>
#include "EngineUtils.h"

#include "Helper.h"


#endif /* TextDB_hpp */
#pragma once
class TextDB {
public:
    static inline TTF_Font* font = nullptr;
    static inline std::unordered_map<std::string, std::unordered_map<int, TTF_Font*>> fonts;
    
    static void DrawIntroText (SDL_Renderer* renderer, SDL_Surface*& text_surface, SDL_FRect& text_rect, SDL_Texture*& text_texture, const std::string& text_content, const char* font_filepath, int font_size, SDL_Color font_color, int x, int y) {
        if (font == nullptr)
            font = TTF_OpenFont(font_filepath, font_size);
        
        if (text_surface) {
            SDL_FreeSurface(text_surface);
            text_surface = nullptr;
        }
        
        text_surface = TTF_RenderText_Solid(font, text_content.c_str(), font_color);
        
        text_texture = SDL_CreateTextureFromSurface(renderer, text_surface);
        
        text_rect = {25.0f, (float)(y - 50), (float)text_surface->w, (float)text_surface->h};
    }
    
//    static void DrawText (const char* font_filepath, int font_size, SDL_Surface*& text_surface, SDL_FRect& text_rect, SDL_Texture*& text_texture, std::string text_content,SDL_Color font_color, SDL_Renderer*& renderer, int x, int y) {
//        
//        // str_content, x, y, font_name, font_size, r, g, b, a
//        
//        if (font == nullptr)
//            font = TTF_OpenFont(font_filepath, font_size);
//        
//        if (text_surface) {
//            SDL_FreeSurface(text_surface);
//            text_surface = nullptr;
//        }
//        
//        text_surface = TTF_RenderText_Solid(font, text_content.c_str(), font_color);
//        
//        text_texture = SDL_CreateTextureFromSurface(renderer, text_surface);
//        
//        text_rect = {(float)x, (float)y, (float)text_surface->w, (float)text_surface->h};
//    }
    
    static void CloseFont() {
        for (auto& [key, size_map] : fonts) {
            for (auto& [size, font_ptr] : size_map) {
                TTF_CloseFont(font_ptr);
            }
        }
    }
    
    static void DrawText(std::string str_content, int x, int y, std::string font_name, int font_size, int r, int g, int b, int a, SDL_Renderer* renderer) {
        
        if (fonts.find(font_name) == fonts.end()){
            std::string font_filepath = EngineUtils::GetResourcePath("resources_game/fonts/" + font_name + ".ttf");  // CHANGE THIS
            fonts[font_name][font_size] = TTF_OpenFont(font_filepath.c_str(), font_size);
        }
        else if (fonts[font_name].find(font_size) == fonts[font_name].end()) {
            std::string font_filepath = EngineUtils::GetResourcePath("resources_game/fonts/" + font_name + ".ttf");  // CHANGE THIS
            fonts[font_name][font_size] = TTF_OpenFont(font_filepath.c_str(), font_size);
        }
        font = fonts[font_name][font_size];
        
        SDL_Color font_color = {static_cast<Uint8>(r), static_cast<Uint8>(g), static_cast<Uint8>(b), static_cast<Uint8>(a)};

        if (str_content.empty()) str_content = " ";
        SDL_Surface* text_surface = TTF_RenderText_Solid(font, str_content.c_str(), font_color);

        SDL_Texture* text_texture = SDL_CreateTextureFromSurface(renderer, text_surface);

        SDL_FRect text_rect = {(float)x, (float)y, (float)text_surface->w, (float)text_surface->h};
            
        Helper::SDL_RenderCopyEx(-1, "", renderer, text_texture, NULL, &text_rect, 0, NULL, SDL_FLIP_NONE);
    }
};
