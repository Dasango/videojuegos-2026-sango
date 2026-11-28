#pragma once

#include <SDL.h>

#include <string>

#include "Engine/Aabb.h"
#include "Engine/Vector2D.h"

// Mapa de 16 x 64 celdas. Cada celda mide 32 px.
// unitScale = 1 / 32 pasa de celdas a pixeles.
constexpr int kColumnas = 16;
constexpr int kFilas = 64;
constexpr int kTile = 32;
constexpr float kUnitScale = 1.0f / 32.0f;
constexpr int kMapaAncho = kColumnas * kTile;
constexpr int kMapaAlto = kFilas * kTile;

// Carga el nivel (mapa.txt y mapa.bmp), convierte coordenadas y dibuja el fondo.
class Mapa {
public:
    Mapa() = default;
    ~Mapa();
    Mapa(const Mapa&) = delete;
    Mapa& operator=(const Mapa&) = delete;

    bool cargar(SDL_Renderer* r, const std::string& txt, const std::string& bmp);
    void soltar();
    void reiniciarVisual(SDL_Renderer* r);
    void dibujar(SDL_Renderer* r, int camY, int margenX, int altoVista) const;

    bool dentro(int col, int fila) const;
    int valorEn(int col, int fila) const;
    int filaDe(float y) const;
    Vector2D centroCelda(int col, int fila) const;
    Aabb cajaCelda(int col, int fila) const;

    SDL_Surface* superficie() const { return superficie_; }

private:
    void refrescarTextura(SDL_Renderer* r);

    int valor_[kFilas][kColumnas]{};
    SDL_Surface* base_ = nullptr;
    SDL_Surface* superficie_ = nullptr;
    SDL_Texture* textura_ = nullptr;
};
