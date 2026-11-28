#include "Game/Mapa.h"

#include <SDL.h>

#include <cstdio>
#include <fstream>
#include <sstream>
#include <string>

namespace {

SDL_Surface* duplicar(SDL_Surface* src) {
    SDL_Surface* copia =
        SDL_CreateRGBSurfaceWithFormat(0, src->w, src->h, 32, SDL_PIXELFORMAT_ARGB8888);
    if (!copia) return nullptr;
    SDL_SetSurfaceBlendMode(src, SDL_BLENDMODE_NONE);
    SDL_SetSurfaceBlendMode(copia, SDL_BLENDMODE_NONE);
    SDL_BlitSurface(src, nullptr, copia, nullptr);
    return copia;
}

}  // namespace

Mapa::~Mapa() { soltar(); }

void Mapa::soltar() {
    if (textura_) {
        SDL_DestroyTexture(textura_);
        textura_ = nullptr;
    }
    if (superficie_) {
        SDL_FreeSurface(superficie_);
        superficie_ = nullptr;
    }
    if (base_) {
        SDL_FreeSurface(base_);
        base_ = nullptr;
    }
}

bool Mapa::cargar(SDL_Renderer* r, const std::string& txt, const std::string& bmp) {
    soltar();

    std::ifstream archivo(txt.c_str());
    if (!archivo) {
        std::fprintf(stderr, "No se pudo abrir el mapa: %s\n", txt.c_str());
        return false;
    }

    int fila = 0;
    std::string linea;
    while (std::getline(archivo, linea)) {
        if (!linea.empty() && linea.back() == '\r') linea.pop_back();
        if (linea.empty()) continue;
        if (fila >= kFilas) {
            std::fprintf(stderr, "El mapa tiene filas de mas.\n");
            return false;
        }
        std::stringstream ss(linea);
        std::string pieza;
        int col = 0;
        while (std::getline(ss, pieza, ',')) {
            if (col >= kColumnas) {
                std::fprintf(stderr, "La fila %d tiene columnas de mas.\n", fila);
                return false;
            }
            valor_[fila][col] = std::stoi(pieza);
            ++col;
        }
        if (col != kColumnas) {
            std::fprintf(stderr, "La fila %d no tiene 16 columnas.\n", fila);
            return false;
        }
        ++fila;
    }
    if (fila != kFilas) {
        std::fprintf(stderr, "El mapa no tiene 64 filas.\n");
        return false;
    }

    SDL_Surface* cargada = SDL_LoadBMP(bmp.c_str());
    if (!cargada) {
        std::fprintf(stderr, "No se pudo abrir el dibujo: %s\n", bmp.c_str());
        return false;
    }
    if (cargada->format->format != SDL_PIXELFORMAT_ARGB8888) {
        SDL_Surface* convertida = SDL_ConvertSurfaceFormat(cargada, SDL_PIXELFORMAT_ARGB8888, 0);
        SDL_FreeSurface(cargada);
        cargada = convertida;
    }
    if (!cargada || cargada->w != kMapaAncho || cargada->h != kMapaAlto) {
        std::fprintf(stderr, "El dibujo del mapa no mide 512x2048.\n");
        if (cargada) SDL_FreeSurface(cargada);
        return false;
    }

    base_ = cargada;
    superficie_ = duplicar(base_);
    if (!superficie_) {
        std::fprintf(stderr, "No se pudo copiar el mapa.\n");
        return false;
    }

    refrescarTextura(r);
    return textura_ != nullptr;
}

void Mapa::refrescarTextura(SDL_Renderer* r) {
    if (textura_) {
        SDL_DestroyTexture(textura_);
        textura_ = nullptr;
    }
    textura_ = SDL_CreateTextureFromSurface(r, superficie_);
    if (textura_) SDL_SetTextureBlendMode(textura_, SDL_BLENDMODE_BLEND);
}

void Mapa::reiniciarVisual(SDL_Renderer* r) {
    if (!base_ || !superficie_) return;
    SDL_SetSurfaceBlendMode(base_, SDL_BLENDMODE_NONE);
    SDL_SetSurfaceBlendMode(superficie_, SDL_BLENDMODE_NONE);
    SDL_BlitSurface(base_, nullptr, superficie_, nullptr);
    refrescarTextura(r);
}

void Mapa::dibujar(SDL_Renderer* r, int camY, int margenX, int altoVista) const {
    if (!textura_) return;
    if (camY < 0) camY = 0;
    if (camY > kMapaAlto - altoVista) camY = kMapaAlto - altoVista;
    SDL_Rect src{0, camY, kMapaAncho, altoVista};
    SDL_Rect dst{margenX, 0, kMapaAncho, altoVista};
    SDL_RenderCopy(r, textura_, &src, &dst);
}

bool Mapa::dentro(int col, int fila) const {
    return col >= 0 && col < kColumnas && fila >= 0 && fila < kFilas;
}

int Mapa::valorEn(int col, int fila) const {
    if (!dentro(col, fila)) return 1;
    return valor_[fila][col];
}

int Mapa::filaDe(float y) const {
    int fila = static_cast<int>(y / static_cast<float>(kTile));
    if (fila < 0) fila = 0;
    if (fila >= kFilas) fila = kFilas - 1;
    return fila;
}

Vector2D Mapa::centroCelda(int col, int fila) const {
    // Dividir por unitScale es lo mismo que multiplicar por el tile.
    float x = (static_cast<float>(col) + 0.5f) / kUnitScale;
    float y = (static_cast<float>(fila) + 0.5f) / kUnitScale;
    return {x, y};
}

Aabb Mapa::cajaCelda(int col, int fila) const {
    Aabb caja;
    caja.x = static_cast<float>(col * kTile);
    caja.y = static_cast<float>(fila * kTile);
    caja.w = static_cast<float>(kTile);
    caja.h = static_cast<float>(kTile);
    return caja;
}
