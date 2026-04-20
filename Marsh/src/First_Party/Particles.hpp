#ifndef Particles_hpp
#define Particles_hpp

#include <stdio.h>
#include "glm/glm.hpp"
#include <vector>
#include "Rendering.hpp"
#include "lua/lua.hpp"
#include "LuaBridge/LuaBridge.h"
#include "Actor.hpp"
#include <queue>

class Rendering;

struct DrawParticleVariables {
    SDL_Texture* particle_texture = nullptr;
    float x, y;
    float w, h;
    float rotation = 0.0f;
    int r = 255, g = 255, b = 255, a = 255;
    int sorting_order = 9999;
    int id = 0;
};

class ParticleSystem {
public:
    glm::vec2 pos = glm::vec2(0.0f, 0.0f);

    float getX() const { return pos.x; }
    void setX(float value) { pos.x = value; }
    float getY() const { return pos.y; }
    void setY(float value) { pos.y = value; }

    Actor* actor = nullptr;
    std::string key = "";
    bool enabled = true;

    int frames_between_bursts = 1;
    int local_frame_number = 0;
    int burst_quantity = 1;
    int sorting_order = 9999;

    float emit_angle_min = 0.0f;
    float emit_angle_max = 360.0f;
    float emit_radius_min = 0.0f;
    float emit_radius_max = 0.5f;

    float start_scale_min = 1.0f;
    float start_scale_max = 1.0f;

    float rotation_min = 0.0f;
    float rotation_max = 0.0f;

    int start_color_r = 255;
    int start_color_g = 255;
    int start_color_b = 255;
    int start_color_a = 255;

    int duration_frames = 300;
    float start_speed_min = 0.0f, start_speed_max = 0.0f;
    float rotation_speed_min = 0.0f, rotation_speed_max = 0.0f;
    float gravity_scale_x = 0.0f, gravity_scale_y = 0.0f;
    float drag_factor = 1.0f;
    float angular_drag_factor = 1.0f;
    float end_scale = -1.0f;
    int end_color_r = -1;
    int end_color_g = -1;
    int end_color_b = -1;
    int end_color_a = -1;

    std::string image = "";

    // Suite #3: Stop/Play state
    bool playing = true;

    RandomEngine* emit_angle_distribution = nullptr;
    RandomEngine* emit_radius_distribution = nullptr;
    RandomEngine* scale_distribution = nullptr;
    RandomEngine* rotation_distribution = nullptr;
    RandomEngine* speed_distribution = nullptr;
    RandomEngine* rotation_speed_distribution = nullptr;

    // Per-particle data (SoA layout for cache performance - Suite #4)
    std::vector<uint8_t>          p_active;
    std::vector<int>           p_frames_alive;
    std::vector<float>         p_px, p_py;
    std::vector<float>         p_vx, p_vy;
    std::vector<float>         p_rotation;
    std::vector<float>         p_rotation_speed;
    std::vector<float>         p_scale;
    std::vector<float>         p_start_scale;
    std::vector<int>           p_start_r, p_start_g, p_start_b, p_start_a;
    std::vector<SDL_Texture*>  p_texture;
    std::vector<int>           free_list;
    std::vector<float>         p_w;
    std::vector<float>         p_h;
    

    static inline int particle_draw_id = 0;
    static inline std::vector<DrawParticleVariables> particles_to_render;

    static void InsertParticleSystem(lua_State* lua_state);

    void OnStart() {
        duration_frames       = duration_frames < 1 ? 1 : duration_frames;
        frames_between_bursts = frames_between_bursts < 1 ? 1 : frames_between_bursts;
        burst_quantity        = burst_quantity < 1 ? 1 : burst_quantity;

        emit_angle_distribution     = new RandomEngine(emit_angle_min, emit_angle_max, 298);
        emit_radius_distribution    = new RandomEngine(emit_radius_min, emit_radius_max, 404);
        rotation_distribution       = new RandomEngine(rotation_min, rotation_max, 440);
        scale_distribution          = new RandomEngine(start_scale_min, start_scale_max, 494);
        speed_distribution          = new RandomEngine(start_speed_min, start_speed_max, 498);
        rotation_speed_distribution = new RandomEngine(rotation_speed_min, rotation_speed_max, 305);

        Rendering::CreateDefaultParticleTextureWithName("default");

        local_frame_number = 0;
    }

    void OnUpdate() {
        // Suite #3: respect playing flag for emission
        if (playing && local_frame_number % frames_between_bursts == 0)
            BurstParticles(burst_quantity);

        ++local_frame_number;
    }

    void OnLateUpdate() {
        UpdateAndRenderParticles();
    }

    // Suite #3: Stop/Play/Burst
    void Stop()  { playing = false; }
    void Play()  { playing = true; }
    void Burst() { BurstParticles(burst_quantity); }

    void BurstParticles(int num_particles) {
        SDL_Texture* tex = nullptr;
        if (!image.empty() && Rendering::images.find(image) != Rendering::images.end())
            tex = Rendering::images[image];
        else
            tex = Rendering::texture_bank["default"];

        for (int i = 0; i < num_particles; ++i) {
            float angle_radians = glm::radians(emit_angle_distribution->Sample());
            float radius        = emit_radius_distribution->Sample();
            float speed         = speed_distribution->Sample();
            float rot           = rotation_distribution->Sample();
            float sc            = scale_distribution->Sample();
            float rot_speed     = rotation_speed_distribution->Sample();

            float new_px = pos.x + glm::cos(angle_radians) * radius;
            float new_py = pos.y + glm::sin(angle_radians) * radius;
            float new_vx = glm::cos(angle_radians) * speed;
            float new_vy = glm::sin(angle_radians) * speed;

            if (!free_list.empty()) {
                int idx = free_list.back();
                free_list.pop_back();
                p_active[idx]         = true;
                p_frames_alive[idx]   = 0;
                p_px[idx]             = new_px;
                p_py[idx]             = new_py;
                p_vx[idx]             = new_vx;
                p_vy[idx]             = new_vy;
                p_rotation[idx]       = rot;
                p_rotation_speed[idx] = rot_speed;
                p_scale[idx]          = sc;
                p_start_scale[idx]    = sc;
                p_start_r[idx]        = start_color_r;
                p_start_g[idx]        = start_color_g;
                p_start_b[idx]        = start_color_b;
                p_start_a[idx]        = start_color_a;
                p_texture[idx]        = tex;
            } else {
                p_active.push_back(true);
                p_frames_alive.push_back(0);
                p_px.push_back(new_px);
                p_py.push_back(new_py);
                p_vx.push_back(new_vx);
                p_vy.push_back(new_vy);
                p_rotation.push_back(rot);
                p_rotation_speed.push_back(rot_speed);
                p_scale.push_back(sc);
                p_start_scale.push_back(sc);
                p_start_r.push_back(start_color_r);
                p_start_g.push_back(start_color_g);
                p_start_b.push_back(start_color_b);
                p_start_a.push_back(start_color_a);
                p_texture.push_back(tex);
                float tex_w = 0.0f;
                float tex_h = 0.0f;
                Helper::SDL_QueryTexture(tex, &tex_w, &tex_h);
                ///TEMPORARY
                p_w.push_back(tex_w);
                p_h.push_back(tex_h);
            }
        }
    }

    void UpdateAndRenderParticles() {
        // Suite #4: reserve render slots upfront to avoid repeated reallocation
        int active_count = 0;
        for (int i = 0; i < (int)p_active.size(); ++i)
            if (p_active[i]) ++active_count;
        particles_to_render.reserve(particles_to_render.size() + active_count);

        for (int i = 0; i < (int)p_active.size(); ++i) {
            if (!p_active[i]) continue;
            
            // ADD THIS
            if (p_texture[i] == nullptr) {
                std::cout << "NULL TEXTURE at particle " << i << std::endl;
                continue;
            }

            if (p_frames_alive[i] >= duration_frames) {
                p_active[i] = false;
                free_list.push_back(i);
                continue;
            }

            float lifetime_progress = static_cast<float>(p_frames_alive[i])
                                    / static_cast<float>(duration_frames);

            // 1. Gravity
            p_vx[i] += gravity_scale_x;
            p_vy[i] += gravity_scale_y;

            // 2. Drag
            p_vx[i] *= drag_factor;
            p_vy[i] *= drag_factor;

            // 3. Angular drag
            p_rotation_speed[i] *= angular_drag_factor;

            // 4. Position
            p_px[i] += p_vx[i];
            p_py[i] += p_vy[i];

            // 5. Rotation
            p_rotation[i] += p_rotation_speed[i];

            // 6. Scale lerp
            float current_scale = p_start_scale[i];
            if (end_scale >= 0.0f)
                current_scale = p_start_scale[i] + (end_scale - p_start_scale[i]) * lifetime_progress;

            // 7. Color lerp from per-particle start color
            int cur_r = p_start_r[i];
            int cur_g = p_start_g[i];
            int cur_b = p_start_b[i];
            int cur_a = p_start_a[i];
            if (end_color_r >= 0) cur_r = static_cast<int>(p_start_r[i] + (end_color_r - p_start_r[i]) * lifetime_progress);
            if (end_color_g >= 0) cur_g = static_cast<int>(p_start_g[i] + (end_color_g - p_start_g[i]) * lifetime_progress);
            if (end_color_b >= 0) cur_b = static_cast<int>(p_start_b[i] + (end_color_b - p_start_b[i]) * lifetime_progress);
            if (end_color_a >= 0) cur_a = static_cast<int>(p_start_a[i] + (end_color_a - p_start_a[i]) * lifetime_progress);

            p_frames_alive[i]++;

            float w = p_w[i] * glm::abs(current_scale);
            float h = p_h[i] * glm::abs(current_scale);
            
//            Helper::SDL_QueryTexture(p_texture[i], &w, &h);

            DrawParticleVariables dpv;
            dpv.particle_texture = p_texture[i];
            dpv.x             = p_px[i];
            dpv.y             = p_py[i];
            dpv.w             = w;
            dpv.h             = h;
            dpv.rotation      = p_rotation[i];
            dpv.r             = cur_r;
            dpv.g             = cur_g;
            dpv.b             = cur_b;
            dpv.a             = cur_a;
            dpv.sorting_order = sorting_order;
            dpv.id            = particle_draw_id++;

            particles_to_render.push_back(dpv);
        }
    }
};

#endif /* Particles_hpp */
