#pragma once

#include <SDL.h>

#include <vector>

#include "Engine/Aabb.h"
#include "Engine/Vector2D.h"
#include "Game/Mapa.h"

struct Premio {
    int col = 0;
    int fila = 0;
    bool vivo = true;
    int variante = 0;
};

constexpr int kPremioAncho = 16;
constexpr int kPremiosVariantes = 4;
constexpr float kPremioEscala = 2.0f;  // premios mas grandes; pueden salirse de la celda

// Maneja solidos (arboles), agua, cajas coleccionables, choques y vista debug de hitboxes.
class Obstaculos {
public:
    void clasificarDesdeMapa(const Mapa& mapa);
    void reiniciar();

    bool bloqueado(const Mapa& mapa, int col, int fila) const;
    bool esAgua(const Mapa& mapa, int col, int fila) const;
    bool pisoSeguro(const Mapa& mapa, const Vector2D& pos) const;

    int premioDeAterrizaje(const Mapa& mapa, const Vector2D& pos, const Vector2D& dir) const;
    Vector2D centroPremio(const Mapa& mapa, int indice) const;
    bool tocaPremio(const Mapa& mapa, int indice, const Aabb& cuerpo) const;
    void recogerPremio(int indice);
    int recogerPremiosEn(Mapa& mapa, const Aabb& cuerpo);

    // Celda con premio sin recoger: bloquea el movimiento pero no mata.
    bool premioEn(const Mapa& mapa, int col, int fila) const;

    // Dibuja los premios sin recoger encima del mapa.
    void dibujarPremios(SDL_Renderer* r, SDL_Texture* sprite, const Mapa& mapa, int camY,
                        int margenX, int altoVista) const;

    void dibujarDebug(SDL_Renderer* r, const Mapa& mapa, int camY, int margenX,
                      int altoVista) const;

private:
    bool solido_[kFilas][kColumnas]{};
    bool agua_[kFilas][kColumnas]{};
    std::vector<Premio> premios_;
};
