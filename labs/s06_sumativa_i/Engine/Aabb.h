#pragma once

// Caja alineada a los ejes.
#include "Engine/Vector2D.h"

struct Aabb {
    float x = 0.0f;
    float y = 0.0f;
    float w = 0.0f;
    float h = 0.0f;

    static Aabb desdeCentro(const Vector2D& c, float ancho, float alto) {
        Aabb caja;
        caja.w = ancho;
        caja.h = alto;
        caja.x = c.x - ancho * 0.5f;
        caja.y = c.y - alto * 0.5f;
        return caja;
    }

    bool toca(const Aabb& o) const {
        if (x + w <= o.x || o.x + o.w <= x) return false;
        if (y + h <= o.y || o.y + o.h <= y) return false;
        return true;
    }

    // Empuje minimo para salir de la otra caja. El eje mas corto es el que se usa.
    bool mtv(const Aabb& o, Vector2D& empuje) const {
        float derecha = (x + w < o.x + o.w) ? (x + w) : (o.x + o.w);
        float izquierda = (x > o.x) ? x : o.x;
        float abajo = (y + h < o.y + o.h) ? (y + h) : (o.y + o.h);
        float arriba = (y > o.y) ? y : o.y;
        float solapaX = derecha - izquierda;
        float solapaY = abajo - arriba;
        if (solapaX <= 0.0f || solapaY <= 0.0f) return false;

        if (solapaX < solapaY) {
            float centro = x + w * 0.5f;
            float centroOtro = o.x + o.w * 0.5f;
            empuje = {(centro < centroOtro) ? -solapaX : solapaX, 0.0f};
        } else {
            float centro = y + h * 0.5f;
            float centroOtro = o.y + o.h * 0.5f;
            empuje = {0.0f, (centro < centroOtro) ? -solapaY : solapaY};
        }
        return true;
    }
};
