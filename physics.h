#pragma once
#include <vector>

// Simple 2D vector with math operators
struct Vec2 {
    float x, y;
    Vec2(float x = 0, float y = 0) : x(x), y(y) {}
    Vec2 operator+(const Vec2& o) const { return {x + o.x, y + o.y}; }
    Vec2 operator-(const Vec2& o) const { return {x - o.x, y - o.y}; }
    Vec2 operator*(float s)       const { return {x * s,   y * s};   }
    Vec2& operator+=(const Vec2& o) { x += o.x; y += o.y; return *this; }
};

// A single physics object (axis-aligned box)
struct Body {
    Vec2  pos;          // top-left corner
    Vec2  vel;          // velocity (pixels/sec)
    float w, h;         // width and height
    float restitution;  // bounciness: 0 = dead stop, 1 = perfect bounce
    float mass;
    bool  isStatic;     // static bodies never move (e.g. platforms)

    Body(float x, float y, float w, float h,
         float restitution = 0.7f, float mass = 1.0f, bool isStatic = false)
        : pos(x, y), vel(0, 0), w(w), h(h),
          restitution(restitution), mass(mass), isStatic(isStatic) {}
};

class PhysicsWorld {
public:
    std::vector<Body> bodies;
    Vec2 gravity{0.0f, 500.0f};  // pixels/sec^2 downward
    int  worldW, worldH;

    PhysicsWorld(int worldW, int worldH);
    void addBody(Body body);
    void step(float dt);

private:
    void integrate(float dt);
    void wallCollisions(Body& b);
    void bodyCollisions();
};