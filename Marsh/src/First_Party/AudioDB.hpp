//
//  AudioDB.hpp
//  game_engine
//
//  Created by Tyler Covacha on 2/9/26.
//

#ifndef AudioDB_hpp
#define AudioDB_hpp

#include <stdio.h>
#include "AudioHelper.h"
#include <string>
#include "EngineUtils.h"
#include <iostream>
#include "Helper.h"
#include <unordered_map>

class AudioDB {
 public:
    static inline std::unordered_map<std::string, Mix_Chunk*> audio_chunks;
    
    static void PlayIntroAudio(std::string audio_file_path) {
        Mix_Chunk* chunk = AudioHelper::Mix_LoadWAV(audio_file_path.c_str());
        AudioHelper::Mix_PlayChannel(0, chunk, -1);
    }
    static void PlayGameplayAudio(std::string audio_file_path) {
        Mix_Chunk* chunk = AudioHelper::Mix_LoadWAV(audio_file_path.c_str());
        AudioHelper::Mix_PlayChannel(0, chunk, -1);
    }
    static Mix_Chunk* MakeAudioChunk(std::string audio_file_path) {
        return AudioHelper::Mix_LoadWAV(audio_file_path.c_str());
    }
    static void PlaySoundSFXAudio(Mix_Chunk* chunk, int channel) {
        AudioHelper::Mix_PlayChannel(channel, chunk, 0);
    }
    static void PlayAudio(Mix_Chunk* chunk, int channel, int loop) {
        AudioHelper::Mix_PlayChannel(channel, chunk, loop);
    }
    static void StopAudio(int channel) {
        AudioHelper::Mix_HaltChannel(channel);
    }
    static void AdjustVolume(int channel, int volume) {
        AudioHelper::Mix_Volume(channel, volume);
    }
    static int CalculateChannel() {
        return Helper::GetFrameNumber() % 48 + 2;
    }
    static void PlayEndingAudio(std::string audio_file_path) {
        Mix_Chunk* chunk = AudioHelper::Mix_LoadWAV(audio_file_path.c_str());
        AudioHelper::Mix_PlayChannel(0, chunk, 0);
    }
    
    static void StopIntroAudio() {
        AudioHelper::Mix_HaltChannel(0);
    }
    
    static std::string CheckAudioFileExists(std::string audio_name) {
        std::string wav_path = EngineUtils::GetResourcePath("resources_game/audio/" + audio_name + ".wav");  // ADD
        std::string ogg_path = EngineUtils::GetResourcePath("resources_game/audio/" + audio_name + ".ogg");  // ADD
        
        if (EngineUtils::CheckDirectoryExists(wav_path)) {  // CHANGE
            return wav_path;  // CHANGE - return full path instead of just filename
        }
        else if (EngineUtils::CheckDirectoryExists(ogg_path)) {  // CHANGE
            return ogg_path;  // CHANGE - return full path instead of just filename
        }
        else {
            std::cout << "error: failed to play audio clip " << audio_name;
            exit(0);
        }
    }
    static std::string CheckEndingAudioFileExists(std::string audio_name) {
        std::string wav_path = EngineUtils::GetResourcePath("resources_game/audio/" + audio_name + ".wav");  // ADD
        std::string ogg_path = EngineUtils::GetResourcePath("resources_game/audio/" + audio_name + ".ogg");  // ADD
        
        if (EngineUtils::CheckDirectoryExists(wav_path)) {  // CHANGE
            return wav_path;  // CHANGE
        }
        else if (EngineUtils::CheckDirectoryExists(ogg_path)) {  // CHANGE
            return ogg_path;  // CHANGE
        }
        return "";
    }
    static void IncreaseAudioChannels() {
        AudioHelper::Mix_AllocateChannels(50);
    }
};

#endif /* AudioDB_hpp */
