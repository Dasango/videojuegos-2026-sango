#pragma once
// Semana 4: vectores 2D. Completar los TODO.
#include <cmath>

namespace mini {

struct Vec2 {
    float x = 0.0f;
    float y = 0.0f;

    Vec2() = default;
    Vec2(float x_, float y_) : x(x_), y(y_) {}

    Vec2 operator+(const Vec2& o) const { return {x + o.x, y + o.y}; }
    Vec2 operator-(const Vec2& o) const { return {x - o.x, y - o.y}; }
    Vec2 operator*(float s) const { return {x * s, y * s}; }

    // TODO: magnitud (length)
    // TODO: normalizacion (normalized) -> cuidado con el vector cero
    // TODO: producto punto (dot)
};

}  // namespace mini
