#ifndef Physics_hpp
#define Physics_hpp

#include <stdio.h>
#include "box2d/box2d.h"
#include "lua/lua.hpp"
#include "LuaBridge/LuaBridge.h"
#include "glm/glm.hpp"

class Actor;

struct Collision {
    Actor* other = nullptr;
    b2Vec2 point = b2Vec2(0, 0);
    b2Vec2 relative_velocity = b2Vec2(0, 0);
    b2Vec2 normal = b2Vec2(0, 0);
};

class ContactListener : public b2ContactListener {
public:
    void BeginContact(b2Contact* contact) override;
    void EndContact(b2Contact* contact) override;
    void CallCollisionFunction(Actor* actor, const std::string& func_name, Collision& collision);
};

class Physics {
public:
    static inline b2World* world = nullptr;
    static inline ContactListener contact_listener;

    static void InsertVector2(lua_State* lua_state);
    static void InsertRigidbody(lua_State* lua_state);
    
    static luabridge::LuaRef Raycast(b2Vec2 pos, b2Vec2 dir, float dist);
    static luabridge::LuaRef RaycastAll(b2Vec2 pos, b2Vec2 dir, float dist);

    static inline void Init() {
        b2Vec2 gravity = b2Vec2(0.0f, 9.8f);
        world = new b2World(gravity);
        world->SetContactListener(&contact_listener);
    }

    static inline void Step() {
        if (world)
            world->Step(1.0f / 60.0f, 8, 3);
    }
};

class Rigidbody {
public:
    std::string key = "";
    bool enabled = true;

    float x = 0.0f;
    float y = 0.0f;
    std::string body_type = "dynamic";
    bool precise = true;
    float gravity_scale = 1.0f;
    float density = 1.0f;
    float angular_friction = 0.3f;
    float rotation = 0.0f;
    bool has_collider = true;
    bool has_trigger = true;
    std::string collider_type = "box";
    float width = 1.0f;
    float height = 1.0f;
    float radius = 0.5f;
    float friction = 0.3f;
    float bounciness = 0.3f;
    
    std::string trigger_type = "box";
    float trigger_width = 1.0f;
    float trigger_height = 1.0f;
    float trigger_radius = 0.5f;
    
    Actor* actor = nullptr;

    b2Body* body = nullptr;

    void OnStart() {
        if (!Physics::world)
            Physics::Init();

        b2BodyDef bodyDef;
        bodyDef.position.Set(x, y);
        bodyDef.bullet = precise;
        bodyDef.gravityScale = gravity_scale;
        bodyDef.angularDamping = angular_friction;
        bodyDef.angle = (rotation * (b2_pi / 180.0f));

        if (body_type == "dynamic")        bodyDef.type = b2_dynamicBody;
        else if (body_type == "static")    bodyDef.type = b2_staticBody;
        else if (body_type == "kinematic") bodyDef.type = b2_kinematicBody;

        body = Physics::world->CreateBody(&bodyDef);

        // phantom body if no collider and no trigger
        if (!has_collider && !has_trigger) {
            b2PolygonShape phantom_shape;
            phantom_shape.SetAsBox(width * 0.5f, height * 0.5f);
            b2FixtureDef phantom_fixture_def;
            phantom_fixture_def.shape = &phantom_shape;
            phantom_fixture_def.density = density;
            phantom_fixture_def.isSensor = true;
            phantom_fixture_def.userData.pointer = reinterpret_cast<uintptr_t>(actor);
            phantom_fixture_def.filter.categoryBits = 0x0000; // collides with nothing
            phantom_fixture_def.filter.maskBits = 0x0000;
            body->CreateFixture(&phantom_fixture_def);
            return;
        }

        // collider fixture first
        if (has_collider) {
            b2FixtureDef colliderDef;
            colliderDef.userData.pointer = reinterpret_cast<uintptr_t>(actor);
            colliderDef.density = density;
            colliderDef.friction = friction;
            colliderDef.restitution = bounciness;
            colliderDef.isSensor = false;
            colliderDef.filter.categoryBits = 0x0001; // collider category
            colliderDef.filter.maskBits = 0x0001;     // only collides with other colliders

            if (collider_type == "box") {
                b2PolygonShape box_shape;
                box_shape.SetAsBox(width * 0.5f, height * 0.5f);
                colliderDef.shape = &box_shape;
                body->CreateFixture(&colliderDef);
            }
            else if (collider_type == "circle") {
                b2CircleShape circle_shape;
                circle_shape.m_radius = radius;
                colliderDef.shape = &circle_shape;
                body->CreateFixture(&colliderDef);
            }
        }

        // trigger fixture second
        if (has_trigger) {
            b2FixtureDef triggerDef;
            triggerDef.userData.pointer = reinterpret_cast<uintptr_t>(actor);
            triggerDef.density = density;
            triggerDef.isSensor = true;
            triggerDef.filter.categoryBits = 0x0002; // trigger category
            triggerDef.filter.maskBits = 0x0002;     // only hits other triggers

            if (trigger_type == "box") {
                b2PolygonShape box_shape;
                box_shape.SetAsBox(trigger_width * 0.5f, trigger_height * 0.5f);
                triggerDef.shape = &box_shape;
                body->CreateFixture(&triggerDef);
            }
            else if (trigger_type == "circle") {
                b2CircleShape circle_shape;
                circle_shape.m_radius = trigger_radius;
                triggerDef.shape = &circle_shape;
                body->CreateFixture(&triggerDef);
            }
        }
    }
    
    void OnDestroy() {
        if (Physics::world && body) {
            Physics::world->DestroyBody(body);
            body = nullptr;
        }
    }

    inline b2Vec2 GetPosition() {
        if (!body) return b2Vec2(x, y);
        return body->GetPosition();
    }
    inline float GetRotation() {
        if (!body) return rotation;
        return body->GetAngle() * (180.0f / b2_pi);
    }
    inline void AddForce(b2Vec2 force_vector) { body->ApplyForceToCenter(force_vector, true); }
    inline void SetVelocity(b2Vec2 velocity_vector) { body->SetLinearVelocity(velocity_vector); }
    inline void SetPosition(b2Vec2 position_vector) {
        if (!body) {
            x = position_vector.x;
            y = position_vector.y;
            return;
        }
        body->SetTransform(position_vector, body->GetAngle());
    }
    inline void SetRotation(float degrees_clockwise) {
        if (!body) {
            rotation = degrees_clockwise;
            return;
        }
        body->SetTransform(GetPosition(), degrees_clockwise * (b2_pi / 180.0f));
    }
    inline void SetAngularVelocity(float degrees_clockwise) { body->SetAngularVelocity(degrees_clockwise * (b2_pi / 180.0f)); }
    inline void SetGravityScale(float scale) { body->SetGravityScale(scale); }
    inline b2Vec2 GetVelocity() { return body->GetLinearVelocity(); }
    inline float GetAngularVelocity() { return body->GetAngularVelocity() * (180.0f / b2_pi); }
    inline float GetGravityScale() { return body->GetGravityScale(); }

    inline void SetUpDirection(b2Vec2 direction) {
        direction.Normalize();
        float angle_rad = glm::atan(direction.x, -direction.y);
        body->SetTransform(GetPosition(), angle_rad);
    }

    inline void SetRightDirection(b2Vec2 direction) {
        direction.Normalize();
        float angle_rad = glm::atan(direction.x, -direction.y);
        body->SetTransform(GetPosition(), angle_rad - (b2_pi / 2.0f));
    }

    inline b2Vec2 GetUpDirection() {
        float angle = body->GetAngle();
        b2Vec2 result = b2Vec2(-glm::cos(angle + (b2_pi / 2)), -glm::sin(angle + (b2_pi / 2)));
        result.Normalize();
        return result;
    }

    inline b2Vec2 GetRightDirection() {
        float angle = body->GetAngle();
        b2Vec2 result = b2Vec2(glm::cos(angle), glm::sin(angle));
        result.Normalize();
        return result;
    }
};

struct HitResult {
    Actor* actor = nullptr;
    b2Vec2 point = b2Vec2(0, 0);
    b2Vec2 normal = b2Vec2(0, 0);
    bool is_trigger = false;
};

class RaycastCallback : public b2RayCastCallback {
public:
    std::vector<HitResult> hits;

    float ReportFixture(b2Fixture* fixture, const b2Vec2& point, const b2Vec2& normal, float fraction) override {
        Actor* actor = reinterpret_cast<Actor*>(fixture->GetUserData().pointer);
        
        if (!actor) return -1; // phantom fixture, skip
        
        // skip phantom fixtures (category 0)
        if (fixture->GetFilterData().categoryBits == 0x0000) return -1;

        HitResult hit;
        hit.actor = actor;
        hit.point = point;
        hit.normal = normal;
        hit.is_trigger = fixture->IsSensor();
        hits.push_back(hit);

        return 1; // continue to find all fixtures
    }
};



#endif /* Physics_hpp */
