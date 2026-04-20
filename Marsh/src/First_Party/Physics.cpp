//
//  Physics.cpp
//  game_engine
//
//  Created by Tyler Covacha on 3/25/26.
//

#include "Physics.hpp"
#include "Actor.hpp"

void Physics::InsertVector2(lua_State* lua_state) {
    luabridge::getGlobalNamespace(lua_state)
        .beginClass<b2Vec2>("Vector2")
        .addConstructor<void(*)(float,float)>()
        .addProperty("x", &b2Vec2::x)
        .addProperty("y", &b2Vec2::y)
        .addFunction("Normalize", &b2Vec2::Normalize)
        .addFunction("Length", &b2Vec2::Length)
        .addFunction("__add", &b2Vec2::operator_add)
        .addFunction("__sub", &b2Vec2::operator_sub)
        .addFunction("__mul", &b2Vec2::operator_mul)
        .addStaticFunction("Distance", b2Distance)
        .addStaticFunction("Dot", static_cast<float (*)(const b2Vec2&, const b2Vec2&)>(&b2Dot))
        .endClass();
}

void Physics::InsertRigidbody(lua_State* lua_state) {
    luabridge::getGlobalNamespace(lua_state)
        .beginClass<Rigidbody>("Rigidbody")
        .addFunction("OnStart", &Rigidbody::OnStart)
        .addFunction("OnDestroy", &Rigidbody::OnDestroy)
        .addProperty("actor", &Rigidbody::actor)
        .addProperty("x", &Rigidbody::x)
        .addProperty("y", &Rigidbody::y)
        .addProperty("key", &Rigidbody::key)
        .addProperty("enabled", &Rigidbody::enabled)
        .addProperty("body_type", &Rigidbody::body_type)
        .addProperty("precise", &Rigidbody::precise)
        .addProperty("gravity_scale", &Rigidbody::gravity_scale)
        .addProperty("density", &Rigidbody::density)
        .addProperty("angular_friction", &Rigidbody::angular_friction)
        .addProperty("rotation", &Rigidbody::rotation)
        .addProperty("has_collider", &Rigidbody::has_collider)
        .addProperty("has_trigger", &Rigidbody::has_trigger)
        .addProperty("collider_type", &Rigidbody::collider_type)
        .addProperty("width", &Rigidbody::width)
        .addProperty("height", &Rigidbody::height)
        .addProperty("radius", &Rigidbody::radius)
        .addProperty("friction", &Rigidbody::friction)
        .addProperty("bounciness", &Rigidbody::bounciness)
        .addFunction("GetPosition", &Rigidbody::GetPosition)
        .addFunction("GetRotation", &Rigidbody::GetRotation)
        .addFunction("AddForce", &Rigidbody::AddForce)
        .addFunction("SetVelocity", &Rigidbody::SetVelocity)
        .addFunction("SetPosition", &Rigidbody::SetPosition)
        .addFunction("SetRotation", &Rigidbody::SetRotation)
        .addFunction("SetAngularVelocity", &Rigidbody::SetAngularVelocity)
        .addFunction("SetGravityScale", &Rigidbody::SetGravityScale)
        .addFunction("SetUpDirection", &Rigidbody::SetUpDirection)
        .addFunction("SetRightDirection", &Rigidbody::SetRightDirection)
        .addFunction("GetVelocity", &Rigidbody::GetVelocity)
        .addFunction("GetAngularVelocity", &Rigidbody::GetAngularVelocity)
        .addFunction("GetGravityScale", &Rigidbody::GetGravityScale)
        .addFunction("GetUpDirection", &Rigidbody::GetUpDirection)
        .addFunction("GetRightDirection", &Rigidbody::GetRightDirection)
        .addProperty("trigger_type", &Rigidbody::trigger_type)
        .addProperty("trigger_width", &Rigidbody::trigger_width)
        .addProperty("trigger_height", &Rigidbody::trigger_height)
        .addProperty("trigger_radius", &Rigidbody::trigger_radius)
        .endClass()
        .beginClass<Collision>("Collision")
        .addProperty("other", &Collision::other)
        .addProperty("point", &Collision::point)
        .addProperty("relative_velocity", &Collision::relative_velocity)
        .addProperty("normal", &Collision::normal)
        .endClass()
        .beginClass<HitResult>("HitResult")
        .addProperty("actor", &HitResult::actor)
        .addProperty("point", &HitResult::point)
        .addProperty("normal", &HitResult::normal)
        .addProperty("is_trigger", &HitResult::is_trigger)
        .endClass()
        .beginNamespace("Physics")
        .addFunction("Raycast", &Physics::Raycast)
        .addFunction("RaycastAll", &Physics::RaycastAll)
        .endNamespace();
}

void ContactListener::BeginContact(b2Contact* contact) {
    b2Fixture* fixtureA = contact->GetFixtureA();
    b2Fixture* fixtureB = contact->GetFixtureB();

    Actor* actorA = reinterpret_cast<Actor*>(fixtureA->GetUserData().pointer);
    Actor* actorB = reinterpret_cast<Actor*>(fixtureB->GetUserData().pointer);

    if (!actorA || !actorB) return;

    bool sensorA = fixtureA->IsSensor();
    bool sensorB = fixtureB->IsSensor();

    if (!sensorA && !sensorB) {
        b2WorldManifold world_manifold;
        contact->GetWorldManifold(&world_manifold);

        Collision collision_a;
        collision_a.other = actorB;
        collision_a.point = world_manifold.points[0];
        collision_a.relative_velocity = fixtureA->GetBody()->GetLinearVelocity() - fixtureB->GetBody()->GetLinearVelocity();
        collision_a.normal = world_manifold.normal;

        Collision collision_b;
        collision_b.other = actorA;
        collision_b.point = world_manifold.points[0];
        collision_b.relative_velocity = fixtureA->GetBody()->GetLinearVelocity() - fixtureB->GetBody()->GetLinearVelocity(); // same as a
        collision_b.normal = world_manifold.normal;

        CallCollisionFunction(actorA, "OnCollisionEnter", collision_a);
        CallCollisionFunction(actorB, "OnCollisionEnter", collision_b);
    }
    else if (sensorA && sensorB) {
        Collision collision_a;
        collision_a.other = actorB;
        collision_a.point = b2Vec2(-999.0f, -999.0f);
        collision_a.relative_velocity = fixtureA->GetBody()->GetLinearVelocity() - fixtureB->GetBody()->GetLinearVelocity();
        collision_a.normal = b2Vec2(-999.0f, -999.0f);

        Collision collision_b;
        collision_b.other = actorA;
        collision_b.point = b2Vec2(-999.0f, -999.0f);
        collision_b.relative_velocity = fixtureA->GetBody()->GetLinearVelocity() - fixtureB->GetBody()->GetLinearVelocity(); // same as a
        collision_b.normal = b2Vec2(-999.0f, -999.0f);

        CallCollisionFunction(actorA, "OnTriggerEnter", collision_a);
        CallCollisionFunction(actorB, "OnTriggerEnter", collision_b);
    }
}

void ContactListener::EndContact(b2Contact* contact) {
    b2Fixture* fixtureA = contact->GetFixtureA();
    b2Fixture* fixtureB = contact->GetFixtureB();

    Actor* actorA = reinterpret_cast<Actor*>(fixtureA->GetUserData().pointer);
    Actor* actorB = reinterpret_cast<Actor*>(fixtureB->GetUserData().pointer);

    if (!actorA || !actorB) return;

    bool sensorA = fixtureA->IsSensor();
    bool sensorB = fixtureB->IsSensor();

    if (!sensorA && !sensorB) {
        Collision collision_a;
        collision_a.other = actorB;
        collision_a.point = b2Vec2(-999.0f, -999.0f);
        collision_a.relative_velocity = fixtureA->GetBody()->GetLinearVelocity() - fixtureB->GetBody()->GetLinearVelocity();
        collision_a.normal = b2Vec2(-999.0f, -999.0f);

        Collision collision_b;
        collision_b.other = actorA;
        collision_b.point = b2Vec2(-999.0f, -999.0f);
        collision_b.relative_velocity = fixtureA->GetBody()->GetLinearVelocity() - fixtureB->GetBody()->GetLinearVelocity(); // same as a
        collision_b.normal = b2Vec2(-999.0f, -999.0f);

        CallCollisionFunction(actorA, "OnCollisionExit", collision_a);
        CallCollisionFunction(actorB, "OnCollisionExit", collision_b);
    }
    else if (sensorA && sensorB) {
        Collision collision_a;
        collision_a.other = actorB;
        collision_a.point = b2Vec2(-999.0f, -999.0f);
        collision_a.relative_velocity = fixtureA->GetBody()->GetLinearVelocity() - fixtureB->GetBody()->GetLinearVelocity();
        collision_a.normal = b2Vec2(-999.0f, -999.0f);

        Collision collision_b;
        collision_b.other = actorA;
        collision_b.point = b2Vec2(-999.0f, -999.0f);
        collision_b.relative_velocity = fixtureA->GetBody()->GetLinearVelocity() - fixtureB->GetBody()->GetLinearVelocity(); // same as a
        collision_b.normal = b2Vec2(-999.0f, -999.0f);

        CallCollisionFunction(actorA, "OnTriggerExit", collision_a);
        CallCollisionFunction(actorB, "OnTriggerExit", collision_b);
    }
}

void ContactListener::CallCollisionFunction(Actor* actor, const std::string& func_name, Collision& collision) {
    for (auto& [key, instance_table] : actor->instance_tables) {
        luabridge::LuaRef func = (*instance_table)[func_name];
        if (func.isFunction()) {
            try {
                func(*instance_table, collision);
            } catch (luabridge::LuaException& e) {
                EngineUtils::ReportError(actor->name, e);
            }
        }
    }
}

luabridge::LuaRef Physics::Raycast(b2Vec2 pos, b2Vec2 dir, float dist) {
    lua_State* L = ComponentDB::GetLuaState();

    if (!world || dist <= 0.0f)
        return luabridge::LuaRef(L);

    dir.Normalize();
    b2Vec2 end = pos + dist * dir;

    RaycastCallback callback;
    world->RayCast(&callback, pos, end);

    if (callback.hits.empty())
        return luabridge::LuaRef(L);

    HitResult* closest = nullptr;
    float min_dist = FLT_MAX;
    for (auto& hit : callback.hits) {
        float d = (hit.point - pos).Length();
        if (d < min_dist) {
            min_dist = d;
            closest = &hit;
        }
    }

    return luabridge::LuaRef(L, *closest);
}

luabridge::LuaRef Physics::RaycastAll(b2Vec2 pos, b2Vec2 dir, float dist) {
    lua_State* L = ComponentDB::GetLuaState();
    luabridge::LuaRef result = luabridge::newTable(L);

    if (!world || dist <= 0.0f)
        return result;

    dir.Normalize();
    b2Vec2 end = pos + dist * dir;

    RaycastCallback callback;
    world->RayCast(&callback, pos, end);

    std::sort(callback.hits.begin(), callback.hits.end(),
        [&pos](const HitResult& a, const HitResult& b) {
            return (a.point - pos).Length() < (b.point - pos).Length();
        });

    for (int i = 0; i < (int)callback.hits.size(); i++) {
        result[i + 1] = callback.hits[i];
    }

    return result;
}
