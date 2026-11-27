#include "Game/UI.h"

#include <cstdio>

#include "Engine/Fuente.h"

namespace {

constexpr int kSprite = 64;
constexpr int kFilaGiro = 4;

}  // namespace

void UI::dibujarMenu(SDL_Renderer* r, SDL_Texture* spriteBotella, float tiempoMenu,
                     int opcionMenu) const {
    SDL_SetRenderDrawColor(r, 18, 70, 32, 255);
    SDL_RenderClear(r);
    dibujarTexto(r, (kVentanaW - anchoTexto("LA BOTELLA", 4)) / 2, 70, "LA BOTELLA", 4, 255, 255,
                 255);
    dibujarTexto(r, (kVentanaW - anchoTexto("LLEGA ARRIBA", 2)) / 2, 130, "LLEGA ARRIBA", 2, 220,
                 240, 180);

    if (spriteBotella) {
        int frame = static_cast<int>(tiempoMenu * 8.0f) % 9;
        if (frame < 0) frame = 0;
        int lado = kSprite * 2;
        int sx = kVentanaW / 2;
        int sy = 280;
        SDL_Rect src{frame * kSprite, kFilaGiro * kSprite, kSprite, kSprite};
        SDL_Rect dst{sx - kSprite, sy - lado, lado, lado};
        SDL_RenderCopy(r, spriteBotella, &src, &dst);
    }

    int y0 = 360;
    dibujarTexto(r, 390, y0, "INICIAR", 3, 255, 255, 255);
    dibujarTexto(r, 390, y0 + 40, "SALIR", 3, 255, 255, 255);
    SDL_Rect marca{360, y0 + opcionMenu * 40 + 4, 16, 16};
    SDL_SetRenderDrawColor(r, 255, 220, 80, 255);
    SDL_RenderFillRect(r, &marca);

    const char* ayuda = "WASD MUEVE  ESPACIO SALTA  P PAUSA";
    dibujarTexto(r, (kVentanaW - anchoTexto(ayuda, 2)) / 2, 520, ayuda, 2, 230, 230, 230);
}

void UI::dibujarHud(SDL_Renderer* r, int vidas, int puntos, int modoFps, int fpsNumero,
                    bool debugHitbox) const {
    char txtVidas[32];
    char txtPuntos[32];
    char txtModo[32];
    char txtHz[32];
    std::snprintf(txtVidas, sizeof(txtVidas), "VIDAS %d", vidas);
    std::snprintf(txtPuntos, sizeof(txtPuntos), "PUNTOS %d", puntos);
    std::snprintf(txtModo, sizeof(txtModo), "MODO %d", modoFps);
    std::snprintf(txtHz, sizeof(txtHz), "%d HZ", fpsNumero);

    dibujarTexto(r, 16, 24, txtVidas, 2, 255, 255, 255);
    dibujarTexto(r, 16, 48, txtPuntos, 2, 255, 230, 120);
    dibujarTexto(r, 16, 78, txtModo, 2, 180, 220, 255);
    dibujarTexto(r, 16, 102, txtHz, 2, 180, 220, 255);

    if (debugHitbox) {
        dibujarTexto(r, 16, 132, "HITBOX H", 2, 120, 255, 120);
    }
}

void UI::dibujarAviso(SDL_Renderer* r, const char* a, const char* b, const char* c) const {
    int an = anchoTexto(a, 4);
    int x = (kVentanaW - an) / 2;
    int y = 220;
    SDL_Rect caja{x - 24, y - 20, an + 48, 150};
    SDL_SetRenderDrawBlendMode(r, SDL_BLENDMODE_BLEND);
    SDL_SetRenderDrawColor(r, 0, 0, 0, 170);
    SDL_RenderFillRect(r, &caja);
    dibujarTexto(r, x, y, a, 4, 255, 255, 255);
    if (b) {
        dibujarTexto(r, (kVentanaW - anchoTexto(b, 2)) / 2, y + 50, b, 2, 255, 230, 140);
    }
    if (c) {
        dibujarTexto(r, (kVentanaW - anchoTexto(c, 2)) / 2, y + 80, c, 2, 220, 220, 220);
    }
}

void UI::dibujarOverlay(SDL_Renderer* r, Pantalla pantalla) const {
    if (pantalla == Pantalla::Pausa) {
        SDL_SetRenderDrawBlendMode(r, SDL_BLENDMODE_BLEND);
        SDL_SetRenderDrawColor(r, 0, 0, 0, 90);
        SDL_Rect todo{0, 0, kVentanaW, kVentanaH};
        SDL_RenderFillRect(r, &todo);
        dibujarAviso(r, "PAUSA", "ENTER SIGUE", "ESC MENU");
    } else if (pantalla == Pantalla::Victoria) {
        dibujarAviso(r, "VICTORIA", "ENTER JUEGA", "ESC MENU");
    } else if (pantalla == Pantalla::Derrota) {
        dibujarAviso(r, "FIN DEL JUEGO", "ENTER JUEGA", "ESC MENU");
    }
}
