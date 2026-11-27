#pragma once

// Matriz 3x3 en coordenadas homogeneas.
// Sirve para mover, girar y escalar. El punto es una columna (x, y, 1).
#include <cmath>

#include "Engine/Vector2D.h"

struct Matrix3x3 {
    // Filas: m0 m1 m2 / m3 m4 m5 / m6 m7 m8
    float m[9];

    Matrix3x3() {
        m[0] = 1.0f;
        m[1] = 0.0f;
        m[2] = 0.0f;
        m[3] = 0.0f;
        m[4] = 1.0f;
        m[5] = 0.0f;
        m[6] = 0.0f;
        m[7] = 0.0f;
        m[8] = 1.0f;
    }

    static Matrix3x3 traslacion(float tx, float ty) {
        Matrix3x3 r;
        r.m[2] = tx;
        r.m[5] = ty;
        return r;
    }

    // Angulo en radianes. En pantalla (Y hacia abajo) un angulo positivo gira
    // como las agujas del reloj. SDL usa el mismo sentido.
    static Matrix3x3 rotacion(float radianes) {
        float c = std::cos(radianes);
        float s = std::sin(radianes);
        Matrix3x3 r;
        r.m[0] = c;
        r.m[1] = -s;
        r.m[3] = s;
        r.m[4] = c;
        return r;
    }

    static Matrix3x3 escala(float sx, float sy) {
        Matrix3x3 r;
        r.m[0] = sx;
        r.m[4] = sy;
        return r;
    }

    // this * otra. Con puntos columna, primero se aplica "otra".
    Matrix3x3 operator*(const Matrix3x3& o) const {
        Matrix3x3 c;
        for (int fila = 0; fila < 3; ++fila) {
            for (int col = 0; col < 3; ++col) {
                c.m[fila * 3 + col] = m[fila * 3 + 0] * o.m[0 * 3 + col] +
                                      m[fila * 3 + 1] * o.m[1 * 3 + col] +
                                      m[fila * 3 + 2] * o.m[2 * 3 + col];
            }
        }
        return c;
    }

    Vector2D transformar(const Vector2D& p) const {
        float nx = m[0] * p.x + m[1] * p.y + m[2];
        float ny = m[3] * p.x + m[4] * p.y + m[5];
        return {nx, ny};
    }

    // Primera columna. Su longitud es la escala en X.
    Vector2D ejeX() const { return {m[0], m[3]}; }
};
