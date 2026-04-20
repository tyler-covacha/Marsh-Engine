//
//  main.m
//  Marsh
//
//  Created by Tyler Covacha on 4/10/26.
//
#define SDL_MAIN_HANDLED
#import <UIKit/UIKit.h>
#import <SDL2/SDL.h>

int main(int argc, char *argv[]) {
    @autoreleasepool {
        return SDL_UIKitRunApp(argc, argv, SDL_main);
    }
}
