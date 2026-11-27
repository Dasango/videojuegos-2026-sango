#pragma once

#include <SDL.h>

#include <string>

#include "Game/Mapa.h"
#include "Game/Obstaculos.h"
#include "Game/OrdenUI.h"
#include "Game/Personaje.h"
#include "Game/UI.h"

constexpr int kMargenX = (kVentanaW - kMapaAncho) / 2;

// Coordina la carga de recursos y conecta mapa, obstaculos, personaje, burbuja y UI.
class Juego {
public:
    bool iniciar(SDL_Renderer* renderer);
    void soltar();
    void tecla(SDL_Keycode key, bool& salir);
    void actualizar(float dt);
    void dibujar(float alpha);
    void anotarMs(float frameMs);
    int modoFps() const { return modoFps_; }

private:
    void reiniciarPartida();
    void dibujarMundo(float alpha);
    std::string buscarAsset(const char* nombre) const;

    SDL_Renderer* renderer_ = nullptr;
    SDL_Texture* sprite_ = nullptr;
    SDL_Texture* premios_ = nullptr;

    Mapa mapa_;
    Obstaculos obstaculos_;
    Personaje personaje_;
    OrdenUI ordenUI_;
    UI ui_;

    int modoFps_ = 1;
    bool debugHitbox_ = false;
    float tiempoMenu_ = 0.0f;
    int vidas_ = 3;
    int puntos_ = 0;
    bool pedirSalto_ = false;
    float fpsMs_ = 0.0f;
    int fpsFrames_ = 0;
    int fpsNumero_ = 0;
    int camY_ = 0;
};
