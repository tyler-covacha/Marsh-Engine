#ifndef INPUT_H
#define INPUT_H

#include "SDL2/SDL.h"
#include <unordered_map>
#include <vector>
#include <string>
#include <iostream>
#include "EngineUtils.h"
#include "glm/glm.hpp"

enum INPUT_STATE { INPUT_STATE_UP, INPUT_STATE_JUST_BECAME_DOWN, INPUT_STATE_DOWN, INPUT_STATE_JUST_BECAME_UP };

class Input
{
public:
    static void Init();
    static void ProcessEvent(const SDL_Event & e);
    static void LateUpdate();
    static void StartOfFrame();

    static bool GetKey(std::string _keycode);
    static bool GetKeyDown(std::string _keycode);
    static bool GetKeyUp(std::string _keycode);
    static glm::vec2 GetMousePosition();
    static bool GetMouseButton(int button);
    static bool GetMouseButtonDown(int button);
    static bool GetMouseButtonUp(int button);
    static float GetMouseScrollDelta();
    static bool GetFingerDown();
    static bool GetFingerUp();
    static glm::vec2 GetFingerPosition();
    static void HideCursor();
    static void ShowCursor();
    
    // New gesture functions
    static bool GetTap();
    static bool GetSwipe(std::string direction); // "up", "down", "left", "right"
    static bool GetDoubleTap();
    static bool GetLongPress();
    static float GetPinchScale(); // Returns 1.0 for no pinch, >1.0 for pinch out, <1.0 for pinch in
    static bool IsPinching();
    static float GetRotationAngle(); // Returns rotation in degrees this frame
    static bool IsRotating();

private:
    // Existing private members
    static inline std::unordered_map<SDL_Scancode, INPUT_STATE> keyboard_states;
    static inline std::vector<SDL_Scancode> just_became_down_scancodes;
    static inline std::vector<SDL_Scancode> just_became_up_scancodes;
    static inline glm::vec2 mouse_position;
    static inline std::unordered_map<int, INPUT_STATE> mouse_button_states;
    static inline std::vector<int> just_became_down_buttons;
    static inline std::vector<int> just_became_up_buttons;
    static inline glm::vec2 finger_position;
    static inline INPUT_STATE finger_state = INPUT_STATE_UP;
    static const std::unordered_map<std::string, SDL_Scancode> __keycode_to_scancode;
    static inline float mouse_scroll_this_frame = 0;
    
    // Tap detection
    static inline bool tap_detected = false;
    static constexpr Uint32 TAP_MAX_TIME = 200; // milliseconds
    static constexpr float TAP_MAX_MOVEMENT = 10.0f;
    
    // Swipe detection
    static inline glm::vec2 swipe_start_position;
    static inline Uint32 swipe_start_time = 0;
    static inline std::string detected_swipe_direction = "";
    static inline bool swipe_consumed = false;
    static constexpr float SWIPE_MIN_DISTANCE = 50.0f;
    static constexpr Uint32 SWIPE_MAX_TIME = 500; // milliseconds
    
    // Double tap detection
    static inline Uint32 last_tap_time = 0;
    static inline glm::vec2 last_tap_position;
    static inline bool double_tap_detected = false;
    static constexpr Uint32 DOUBLE_TAP_MAX_TIME = 300; // milliseconds
    static constexpr float DOUBLE_TAP_MAX_DISTANCE = 30.0f;
    
    // Long press detection
    static inline Uint32 press_start_time = 0;
    static inline glm::vec2 press_start_position;
    static inline bool long_press_detected = false;
    static inline bool long_press_consumed = false;
    static constexpr Uint32 LONG_PRESS_TIME = 500; // milliseconds
    static constexpr float LONG_PRESS_MAX_MOVEMENT = 10.0f;
    
    // Pinch detection
    static inline bool is_pinching = false;
    static inline float initial_pinch_distance = 0.0f;
    static inline float current_pinch_distance = 0.0f;
    static inline float pinch_scale = 1.0f;
    static inline std::unordered_map<SDL_FingerID, glm::vec2> active_fingers;
    
    // Rotation detection
    static inline bool is_rotating = false;
    static inline float rotation_angle = 0.0f;
    static inline float initial_rotation_angle = 0.0f;
    static inline float current_rotation_angle = 0.0f;
};

#endif
