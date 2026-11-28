#pragma once

// Misma idea que la tarea 3: longitud, normalizacion y producto punto.
#include <cmath>

struct Vector2D {
    float x = 0.0f;
    float y = 0.0f;

    Vector2D() = default;
    Vector2D(float x_, float y_) : x(x_), y(y_) {}

    Vector2D operator+(const Vector2D& o) const { return {x + o.x, y + o.y}; }
    Vector2D operator-(const Vector2D& o) const { return {x - o.x, y - o.y}; }
    Vector2D operator*(float s) const { return {x * s, y * s}; }

    float length() const { return std::sqrt(x * x + y * y); }

    // El vector cero se queda en cero. No se divide.
    Vector2D normalized() const {
        float largo = length();
        if (largo > 0.0001f) {
            return {x / largo, y / largo};
        }
        return {0.0f, 0.0f};
    }

    float dot(const Vector2D& o) const { return x * o.x + y * o.y; }
};
