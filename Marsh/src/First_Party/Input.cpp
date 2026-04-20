//
//  Input.cpp
//  game_engine
//
//  Created by Tyler Covacha on 2/10/26.
//

#include "Input.hpp"

const std::unordered_map<std::string, SDL_Scancode> Input::__keycode_to_scancode = {
    // Directional (arrow) Keys
    {"up", SDL_SCANCODE_UP},
    {"down", SDL_SCANCODE_DOWN},
    {"right", SDL_SCANCODE_RIGHT},
    {"left", SDL_SCANCODE_LEFT},

    // Misc Keys
    {"escape", SDL_SCANCODE_ESCAPE},

    // Modifier Keys
    {"lshift", SDL_SCANCODE_LSHIFT},
    {"rshift", SDL_SCANCODE_RSHIFT},
    {"lctrl", SDL_SCANCODE_LCTRL},
    {"rctrl", SDL_SCANCODE_RCTRL},
    {"lalt", SDL_SCANCODE_LALT},
    {"ralt", SDL_SCANCODE_RALT},

    // Editing Keys
    {"tab", SDL_SCANCODE_TAB},
    {"return", SDL_SCANCODE_RETURN},
    {"enter", SDL_SCANCODE_RETURN},
    {"backspace", SDL_SCANCODE_BACKSPACE},
    {"delete", SDL_SCANCODE_DELETE},
    {"insert", SDL_SCANCODE_INSERT},

    // Character Keys
    {"space", SDL_SCANCODE_SPACE},
    {"a", SDL_SCANCODE_A},
    {"b", SDL_SCANCODE_B},
    {"c", SDL_SCANCODE_C},
    {"d", SDL_SCANCODE_D},
    {"e", SDL_SCANCODE_E},
    {"f", SDL_SCANCODE_F},
    {"g", SDL_SCANCODE_G},
    {"h", SDL_SCANCODE_H},
    {"i", SDL_SCANCODE_I},
    {"j", SDL_SCANCODE_J},
    {"k", SDL_SCANCODE_K},
    {"l", SDL_SCANCODE_L},
    {"m", SDL_SCANCODE_M},
    {"n", SDL_SCANCODE_N},
    {"o", SDL_SCANCODE_O},
    {"p", SDL_SCANCODE_P},
    {"q", SDL_SCANCODE_Q},
    {"r", SDL_SCANCODE_R},
    {"s", SDL_SCANCODE_S},
    {"t", SDL_SCANCODE_T},
    {"u", SDL_SCANCODE_U},
    {"v", SDL_SCANCODE_V},
    {"w", SDL_SCANCODE_W},
    {"x", SDL_SCANCODE_X},
    {"y", SDL_SCANCODE_Y},
    {"z", SDL_SCANCODE_Z},
    {"0", SDL_SCANCODE_0},
    {"1", SDL_SCANCODE_1},
    {"2", SDL_SCANCODE_2},
    {"3", SDL_SCANCODE_3},
    {"4", SDL_SCANCODE_4},
    {"5", SDL_SCANCODE_5},
    {"6", SDL_SCANCODE_6},
    {"7", SDL_SCANCODE_7},
    {"8", SDL_SCANCODE_8},
    {"9", SDL_SCANCODE_9},
    {"/", SDL_SCANCODE_SLASH},
    {";", SDL_SCANCODE_SEMICOLON},
    {"=", SDL_SCANCODE_EQUALS},
    {"-", SDL_SCANCODE_MINUS},
    {".", SDL_SCANCODE_PERIOD},
    {",", SDL_SCANCODE_COMMA},
    {"[", SDL_SCANCODE_LEFTBRACKET},
    {"]", SDL_SCANCODE_RIGHTBRACKET},
    {"\\", SDL_SCANCODE_BACKSLASH},
    {"'", SDL_SCANCODE_APOSTROPHE}
};

void Input::Init() {
    for (int code = SDL_SCANCODE_UNKNOWN; code < SDL_NUM_SCANCODES; code++) {
        keyboard_states[static_cast<SDL_Scancode>(code)] = INPUT_STATE_UP;
    }
    
    mouse_button_states[1] = INPUT_STATE_UP;
    mouse_button_states[2] = INPUT_STATE_UP;
    mouse_button_states[3] = INPUT_STATE_UP;
    
    // Initialize gesture variables
    swipe_start_time = 0;
    last_tap_time = 0;
    press_start_time = 0;
    is_rotating = false;
    rotation_angle = 0.0f;
    active_fingers.clear();
}

void Input::StartOfFrame() {
    mouse_scroll_this_frame = 0;
    
    // Reset per-frame gesture flags
    detected_swipe_direction = "";
    swipe_consumed = false;
    double_tap_detected = false;
    tap_detected = false;
    rotation_angle = 0.0f;
    
    // Check for long press timeout
    if (finger_state == INPUT_STATE_DOWN && !long_press_detected) {
        Uint32 current_time = SDL_GetTicks();
        if (current_time - press_start_time >= LONG_PRESS_TIME) {
            float distance = glm::distance(finger_position, press_start_position);
            if (distance < LONG_PRESS_MAX_MOVEMENT) {
                long_press_detected = true;
                long_press_consumed = false;
            }
        }
    }
}

void Input::ProcessEvent(const SDL_Event& e) {
    if (e.type == SDL_MOUSEWHEEL) {
        mouse_scroll_this_frame = e.wheel.preciseY;
    }
    
    if (e.type == SDL_KEYDOWN) {
        keyboard_states[e.key.keysym.scancode] = INPUT_STATE_JUST_BECAME_DOWN;
        just_became_down_scancodes.push_back(e.key.keysym.scancode);
    }
    else if (e.type == SDL_KEYUP) {
        keyboard_states[e.key.keysym.scancode] = INPUT_STATE_JUST_BECAME_UP;
        just_became_up_scancodes.push_back(e.key.keysym.scancode);
    }
    
    if (e.type == SDL_MOUSEBUTTONDOWN) {
        mouse_button_states[e.button.button] = INPUT_STATE_JUST_BECAME_DOWN;
        just_became_down_buttons.push_back(e.button.button);
    }
    else if (e.type == SDL_MOUSEBUTTONUP) {
        mouse_button_states[e.button.button] = INPUT_STATE_JUST_BECAME_UP;
        just_became_up_buttons.push_back(e.button.button);
    }
    
    if (e.type == SDL_MOUSEMOTION) {
        mouse_position.x = e.motion.x;
        mouse_position.y = e.motion.y;
    }
    
    if (e.type == SDL_FINGERDOWN) {
        glm::vec2 touch_pos(e.tfinger.x * EngineUtils::window_size.x,
                           e.tfinger.y * EngineUtils::window_size.y);
        
        active_fingers[e.tfinger.fingerId] = touch_pos;
        
        // First finger down
        if (active_fingers.size() == 1) {
            finger_state = INPUT_STATE_JUST_BECAME_DOWN;
            finger_position = touch_pos;
            
            // Start swipe tracking
            swipe_start_position = touch_pos;
            swipe_start_time = SDL_GetTicks();
            
            // Start long press tracking
            press_start_position = touch_pos;
            press_start_time = SDL_GetTicks();
            long_press_detected = false;
            
            // Check for double tap
            Uint32 current_time = SDL_GetTicks();
            if (current_time - last_tap_time <= DOUBLE_TAP_MAX_TIME) {
                float distance = glm::distance(touch_pos, last_tap_position);
                if (distance <= DOUBLE_TAP_MAX_DISTANCE) {
                    double_tap_detected = true;
                }
            }
            last_tap_time = current_time;
            last_tap_position = touch_pos;
        }
        // Second finger down - start pinch and rotation
        else if (active_fingers.size() == 2) {
                auto it = active_fingers.begin();
                glm::vec2 finger1 = it->second;
                ++it;
                glm::vec2 finger2 = it->second;
                
                // Pinch setup
                initial_pinch_distance = glm::distance(finger1, finger2);
                current_pinch_distance = initial_pinch_distance;
                is_pinching = true;
                pinch_scale = 1.0f;
                
                // Rotation setup
                glm::vec2 delta = finger2 - finger1;
                initial_rotation_angle = atan2(delta.y, delta.x);
                current_rotation_angle = initial_rotation_angle;
                is_rotating = true;
                rotation_angle = 0.0f;
            }
    }
    else if (e.type == SDL_FINGERUP) {
        glm::vec2 touch_pos(e.tfinger.x * EngineUtils::window_size.x,
                           e.tfinger.y * EngineUtils::window_size.y);
        
        // Detect tap on finger up (before swipe detection)
        if (active_fingers.size() == 1) {
            Uint32 current_time = SDL_GetTicks();
            Uint32 time_elapsed = current_time - swipe_start_time;
            float distance = glm::distance(touch_pos, swipe_start_position);
            
            // Check if it's a tap (quick and minimal movement)
            if (time_elapsed <= TAP_MAX_TIME && distance <= TAP_MAX_MOVEMENT) {
                tap_detected = true;
            }
            // Otherwise check for swipe
            else if (time_elapsed <= SWIPE_MAX_TIME && !swipe_consumed) {
                float swipe_distance = glm::length(touch_pos - swipe_start_position);
                
                if (swipe_distance >= SWIPE_MIN_DISTANCE) {
                    glm::vec2 delta = touch_pos - swipe_start_position;
                    // Determine swipe direction
                    if (abs(delta.x) > abs(delta.y)) {
                        detected_swipe_direction = (delta.x > 0) ? "right" : "left";
                    } else {
                        detected_swipe_direction = (delta.y > 0) ? "down" : "up";
                    }
                }
            }
        }
        
        active_fingers.erase(e.tfinger.fingerId);
            
        // All fingers up
        if (active_fingers.empty()) {
            finger_state = INPUT_STATE_JUST_BECAME_UP;
            is_pinching = false;
            pinch_scale = 1.0f;
            is_rotating = false;
            rotation_angle = 0.0f;
        }
        // One finger remains after pinch/rotate
        else if (active_fingers.size() == 1) {
            is_pinching = false;
            pinch_scale = 1.0f;
            is_rotating = false;
            rotation_angle = 0.0f;
        }
    }
    
    if (e.type == SDL_FINGERMOTION) {
        glm::vec2 touch_pos(e.tfinger.x * EngineUtils::window_size.x,
                           e.tfinger.y * EngineUtils::window_size.y);
        
        active_fingers[e.tfinger.fingerId] = touch_pos;
        
        if (active_fingers.size() == 1) {
            finger_position = touch_pos;
        }
        // Update pinch scale AND rotation
        else if (active_fingers.size() == 2) {
            auto it = active_fingers.begin();
            glm::vec2 finger1 = it->second;
            ++it;
            glm::vec2 finger2 = it->second;
            
            if (is_pinching) {
                current_pinch_distance = glm::distance(finger1, finger2);
                pinch_scale = current_pinch_distance / initial_pinch_distance;
            }
            
            if (is_rotating) {
                glm::vec2 delta = finger2 - finger1;
                current_rotation_angle = atan2(delta.y, delta.x);
                
                // Calculate angle difference in radians, then convert to degrees
                float angle_diff = current_rotation_angle - initial_rotation_angle;
                
                // Normalize to -PI to PI range
                while (angle_diff > M_PI) angle_diff -= 2.0f * M_PI;
                while (angle_diff < -M_PI) angle_diff += 2.0f * M_PI;
                
                // Convert to degrees
                rotation_angle = angle_diff * (180.0f / M_PI);
            }
        }
    }
}

void Input::LateUpdate() {
    for (const SDL_Scancode& code : just_became_down_scancodes) {
        keyboard_states[code] = INPUT_STATE_DOWN;
    }
    just_became_down_scancodes.clear();
    
    for (const SDL_Scancode& code : just_became_up_scancodes) {
        keyboard_states[code] = INPUT_STATE_UP;
    }
    just_became_up_scancodes.clear();
    
    for (const int button : just_became_down_buttons) {
        mouse_button_states[button] = INPUT_STATE_DOWN;
    }
    just_became_down_buttons.clear();
    
    for (const int button : just_became_up_buttons) {
        mouse_button_states[button] = INPUT_STATE_UP;
    }
    just_became_up_buttons.clear();
    
    if (finger_state == INPUT_STATE_JUST_BECAME_UP) {
        finger_state = INPUT_STATE_UP;
        long_press_consumed = false;
    }
    else if (finger_state == INPUT_STATE_JUST_BECAME_DOWN) {
        finger_state = INPUT_STATE_DOWN;
    }
}

bool Input::GetKey(std::string _keycode) {
    if (__keycode_to_scancode.find(_keycode) == __keycode_to_scancode.end()) return false;
    SDL_Scancode keycode = __keycode_to_scancode.at(_keycode);
    return keyboard_states[keycode] == INPUT_STATE_DOWN || keyboard_states[keycode] == INPUT_STATE_JUST_BECAME_DOWN;
}

bool Input::GetKeyUp(std::string _keycode) {
    if (__keycode_to_scancode.find(_keycode) == __keycode_to_scancode.end()) return false;
    SDL_Scancode keycode = __keycode_to_scancode.at(_keycode);
    return keyboard_states[keycode] == INPUT_STATE_JUST_BECAME_UP;
}

bool Input::GetKeyDown(std::string _keycode) {
    if (__keycode_to_scancode.find(_keycode) == __keycode_to_scancode.end()) return false;
    SDL_Scancode keycode = __keycode_to_scancode.at(_keycode);
    return keyboard_states[keycode] == INPUT_STATE_JUST_BECAME_DOWN;
}

bool Input::GetMouseButton(int button) {
    return mouse_button_states[button] == INPUT_STATE_DOWN || mouse_button_states[button] == INPUT_STATE_JUST_BECAME_DOWN;
}

bool Input::GetMouseButtonDown(int button) {
    return mouse_button_states[button] == INPUT_STATE_JUST_BECAME_DOWN;
}

bool Input::GetMouseButtonUp(int button) {
    return mouse_button_states[button] == INPUT_STATE_JUST_BECAME_UP;
}

bool Input::GetFingerDown() {
    return finger_state == INPUT_STATE_JUST_BECAME_DOWN || finger_state == INPUT_STATE_DOWN;
}

bool Input::GetFingerUp() {
    return finger_state == INPUT_STATE_JUST_BECAME_UP || finger_state == INPUT_STATE_UP;
}

glm::vec2 Input::GetFingerPosition() {
    return finger_position;
}

glm::vec2 Input::GetMousePosition() {
    return mouse_position;
}

float Input::GetMouseScrollDelta() {
    return mouse_scroll_this_frame;
}

void Input::HideCursor() {
    SDL_ShowCursor(SDL_DISABLE);
}

void Input::ShowCursor() {
    SDL_ShowCursor(SDL_ENABLE);
}

// New gesture functions
bool Input::GetTap() {
    return tap_detected;
}

bool Input::GetSwipe(std::string direction) {
    if (detected_swipe_direction == direction && !swipe_consumed) {
        swipe_consumed = true;
        return true;
    }
    return false;
}

bool Input::GetDoubleTap() {
    return double_tap_detected;
}

bool Input::GetLongPress() {
    if (long_press_detected && !long_press_consumed) {
        long_press_consumed = true;
        return true;
    }
    return false;
}

float Input::GetPinchScale() {
    return pinch_scale;
}

bool Input::IsPinching() {
    return is_pinching;
}

float Input::GetRotationAngle() {
    return rotation_angle;
}

bool Input::IsRotating() {
    return is_rotating;
}
