#include "physics.h"
#include <algorithm>
#include <cmath>

PhysicsWorld::PhysicsWorld(int w, int h) : worldW(w), worldH(h) {}

void PhysicsWorld::addBody(Body body) {
    bodies.push_back(body);
}

void PhysicsWorld::step(float dt) {
    integrate(dt);
    bodyCollisions();
    for (auto& b : bodies)
        if (!b.isStatic) wallCollisions(b);
}

// Apply gravity and move every body
void PhysicsWorld::integrate(float dt) {
    for (auto& b : bodies) {
        if (b.isStatic) continue;
        b.vel += gravity * dt;
        b.pos += b.vel * dt;
    }
}

// Bounce off the window edges
void PhysicsWorld::wallCollisions(Body& b) {
    if (b.pos.x < 0) {
        b.pos.x = 0;
        b.vel.x = std::abs(b.vel.x) * b.restitution;
    }
    if (b.pos.x + b.w > worldW) {
        b.pos.x = worldW - b.w;
        b.vel.x = -std::abs(b.vel.x) * b.restitution;
    }
    if (b.pos.y < 0) {
        b.pos.y = 0;
        b.vel.y = std::abs(b.vel.y) * b.restitution;
    }
    if (b.pos.y + b.h > worldH) {
        b.pos.y = worldH - b.h;
        b.vel.y = -std::abs(b.vel.y) * b.restitution;
        b.vel.x *= 0.97f;  // floor friction
    }
}

// AABB collision detection + impulse resolution between every pair of bodies
void PhysicsWorld::bodyCollisions() {
    for (int i = 0; i < (int)bodies.size(); i++) {
        for (int j = i + 1; j < (int)bodies.size(); j++) {
            Body& a = bodies[i];
            Body& b = bodies[j];
            if (a.isStatic && b.isStatic) continue;

            // How much do the two boxes overlap on each axis?
            float ox = std::min(a.pos.x + a.w, b.pos.x + b.w) - std::max(a.pos.x, b.pos.x);
            float oy = std::min(a.pos.y + a.h, b.pos.y + b.h) - std::max(a.pos.y, b.pos.y);

            if (ox <= 0 || oy <= 0) continue;  // no overlap = no collision

            float e    = std::min(a.restitution, b.restitution);
            float invM = (a.isStatic ? 0.0f : 1.0f / a.mass)
                       + (b.isStatic ? 0.0f : 1.0f / b.mass);

            // Resolve along the axis of least penetration
            if (ox < oy) {
                // Horizontal collision
                float dir = (a.pos.x < b.pos.x) ? 1.0f : -1.0f;
                if (!a.isStatic) a.pos.x -= dir * ox * (1.0f / a.mass) / invM;
                if (!b.isStatic) b.pos.x += dir * ox * (1.0f / b.mass) / invM;

                float relVx  = a.vel.x - b.vel.x;
                float impulse = -(1.0f + e) * relVx / invM;
                if (!a.isStatic) a.vel.x += impulse / a.mass;
                if (!b.isStatic) b.vel.x -= impulse / b.mass;
            } else {
                // Vertical collision
                float dir = (a.pos.y < b.pos.y) ? 1.0f : -1.0f;
                if (!a.isStatic) a.pos.y -= dir * oy * (1.0f / a.mass) / invM;
                if (!b.isStatic) b.pos.y += dir * oy * (1.0f / b.mass) / invM;

                float relVy  = a.vel.y - b.vel.y;
                float impulse = -(1.0f + e) * relVy / invM;
                if (!a.isStatic) a.vel.y += impulse / a.mass;
                if (!b.isStatic) b.vel.y -= impulse / b.mass;
            }
        }
    }
}