#pragma once

#include <SDL.h>

#include "Game/OrdenUI.h"

// Ventana un poco mas ancha que el mapa (512 px), para el texto de vidas y puntos.
constexpr int kVentanaW = 960;
constexpr int kVentanaH = 600;

// Dibuja todos los elementos de interfaz: menu, HUD y avisos de pausa/fin.
class UI {
public:
    void dibujarMenu(SDL_Renderer* r, SDL_Texture* spriteBotella, float tiempoMenu,
                     int opcionMenu) const;
    void dibujarHud(SDL_Renderer* r, int vidas, int puntos, int modoFps, int fpsNumero,
                    bool debugHitbox) const;
    void dibujarOverlay(SDL_Renderer* r, Pantalla pantalla) const;

private:
    void dibujarAviso(SDL_Renderer* r, const char* a, const char* b, const char* c) const;
};
