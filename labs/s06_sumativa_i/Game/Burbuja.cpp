#include "Game/Burbuja.h"

namespace {

void disco(SDL_Renderer* r, int cx, int cy, int radio, Uint8 cr, Uint8 cg, Uint8 cb, Uint8 ca) {
    if (radio < 1) return;
    SDL_SetRenderDrawBlendMode(r, SDL_BLENDMODE_BLEND);
    SDL_SetRenderDrawColor(r, cr, cg, cb, ca);
    for (int dy = -radio; dy <= radio; ++dy) {
        int resto = radio * radio - dy * dy;
        int hw = 0;
        while ((hw + 1) * (hw + 1) <= resto) ++hw;
        SDL_RenderDrawLine(r, cx - hw, cy + dy, cx + hw, cy + dy);
    }
}

void anillo(SDL_Renderer* r, int cx, int cy, int radio, Uint8 cr, Uint8 cg, Uint8 cb, Uint8 ca) {
    if (radio < 1) return;
    SDL_SetRenderDrawBlendMode(r, SDL_BLENDMODE_BLEND);
    SDL_SetRenderDrawColor(r, cr, cg, cb, ca);
    for (int dy = -radio; dy <= radio; ++dy) {
        for (int dx = -radio; dx <= radio; ++dx) {
            int d2 = dx * dx + dy * dy;
            if (d2 <= radio * radio && d2 >= (radio - 1) * (radio - 1)) {
                SDL_RenderDrawPoint(r, cx + dx, cy + dy);
            }
        }
    }
}

void reticula(SDL_Renderer* r, int cx, int cy, int radioExterior, int largoMarca, bool enAire) {
    SDL_SetRenderDrawBlendMode(r, SDL_BLENDMODE_BLEND);
    Uint8 alpha = enAire ? 230 : 150;

    anillo(r, cx, cy, radioExterior, 255, 255, 255, alpha);

    int inicio = radioExterior - 2;
    int fin = radioExterior + largoMarca;

    SDL_SetRenderDrawColor(r, 20, 40, 60, alpha);
    for (int offset = -1; offset <= 1; ++offset) {
        SDL_RenderDrawLine(r, cx + offset, cy - fin, cx + offset, cy - inicio);
        SDL_RenderDrawLine(r, cx + offset, cy + inicio, cx + offset, cy + fin);
        SDL_RenderDrawLine(r, cx - fin, cy + offset, cx - inicio, cy + offset);
        SDL_RenderDrawLine(r, cx + inicio, cy + offset, cx + fin, cy + offset);
    }

    SDL_SetRenderDrawColor(r, 170, 230, 255, 255);
    SDL_RenderDrawLine(r, cx, cy - fin + 1, cx, cy - inicio);
    SDL_RenderDrawLine(r, cx, cy + inicio, cx, cy + fin - 1);
    SDL_RenderDrawLine(r, cx - fin + 1, cy, cx - inicio, cy);
    SDL_RenderDrawLine(r, cx + inicio, cy, cx + fin - 1, cy);

    disco(r, cx, cy, 3, 20, 40, 60, 255);
    disco(r, cx, cy, 2, 255, 220, 100, 255);
}

}  // namespace

void dibujarBurbuja(SDL_Renderer* r, const Matrix3x3& padreBotella, const Vector2D& destino,
                    bool enAire, int margenX, int camY) {
    Vector2D centro = destino;
    if (!enAire) {
        // En el suelo hereda el marco de la botella: M_hijo_mundo = M_padre_mundo * M_hijo_local.
        Vector2D origen(padreBotella.m[2], padreBotella.m[5]);
        Vector2D ejeX(padreBotella.m[0], padreBotella.m[3]);
        Vector2D ejeY(padreBotella.m[1], padreBotella.m[4]);
        float len2 = ejeX.dot(ejeX);
        if (len2 > 0.0001f) {
            Vector2D delta = destino - origen;
            float lx = delta.dot(ejeX) / len2;
            float ly = delta.dot(ejeY) / len2;
            Matrix3x3 hijoLocal = Matrix3x3::traslacion(lx, ly);
            Matrix3x3 hijoMundo = padreBotella * hijoLocal;
            centro = hijoMundo.transformar({0.0f, 0.0f});
        }
    }

    int cx = static_cast<int>(centro.x + static_cast<float>(margenX));
    int cy = static_cast<int>(centro.y - static_cast<float>(camY));

    int radio = enAire ? 10 : 8;
    int largoMarca = enAire ? 5 : 3;

    reticula(r, cx, cy, radio, largoMarca, enAire);
}
