#include "Game/Obstaculos.h"

namespace {

bool pixelAzul(Uint8 r, Uint8 g, Uint8 b) { return b > 140 && b > r + 40 && b > g + 20; }

}  // namespace

void Obstaculos::clasificarDesdeMapa(const Mapa& mapa) {
    premios_.clear();
    SDL_Surface* sup = mapa.superficie();
    if (!sup) return;

    SDL_LockSurface(sup);
    for (int fila = 0; fila < kFilas; ++fila) {
        for (int col = 0; col < kColumnas; ++col) {
            int azules = 0;
            int muestras = 0;
            int x0 = col * kTile + 6;
            int y0 = fila * kTile + 6;
            for (int y = y0; y < y0 + 20; ++y) {
                Uint8* linea = static_cast<Uint8*>(sup->pixels) + y * sup->pitch;
                for (int x = x0; x < x0 + 20; ++x) {
                    Uint32 px = *reinterpret_cast<Uint32*>(linea + x * 4);
                    Uint8 r = 0;
                    Uint8 g = 0;
                    Uint8 b = 0;
                    Uint8 a = 0;
                    SDL_GetRGBA(px, sup->format, &r, &g, &b, &a);
                    if (pixelAzul(r, g, b)) ++azules;
                    ++muestras;
                }
            }
            bool hayAgua = muestras > 0 && (azules * 2 > muestras);
            agua_[fila][col] = false;
            solido_[fila][col] = false;
            int v = mapa.valorEn(col, fila);
            if (v == 2) {
                int variante = (col + fila) % kPremiosVariantes;
                premios_.push_back({col, fila, true, variante});
            } else if (hayAgua) {
                // En el archivo el agua vale 1, igual que el arbol.
                // El azul del dibujo la separa: se puede pisar y quita una vida.
                agua_[fila][col] = true;
            } else if (v == 1) {
                solido_[fila][col] = true;
            }
        }
    }
    SDL_UnlockSurface(sup);
}

void Obstaculos::reiniciar() {
    for (Premio& p : premios_) p.vivo = true;
}

bool Obstaculos::bloqueado(const Mapa& mapa, int col, int fila) const {
    if (!mapa.dentro(col, fila)) return true;
    return solido_[fila][col];
}

bool Obstaculos::esAgua(const Mapa& mapa, int col, int fila) const {
    if (!mapa.dentro(col, fila)) return false;
    return agua_[fila][col];
}

bool Obstaculos::pisoSeguro(const Mapa& mapa, const Vector2D& pos) const {
    // Un poco antes del agua o del arbol, para no reaparecer en el borde.
    const float margen = 12.0f;
    float xs[3] = {pos.x - margen, pos.x, pos.x + margen};
    float ys[3] = {pos.y - margen, pos.y, pos.y + margen};
    for (float y : ys) {
        for (float x : xs) {
            int col = static_cast<int>(x / static_cast<float>(kTile));
            int fila = static_cast<int>(y / static_cast<float>(kTile));
            if (esAgua(mapa, col, fila) || bloqueado(mapa, col, fila)) return false;
        }
    }
    return true;
}

int Obstaculos::premioDeAterrizaje(const Mapa& mapa, const Vector2D& pos,
                                    const Vector2D& dir) const {
    int mejor = -1;
    float mejorDist = 100000.0f;
    Vector2D n = dir.normalized();
    bool hayDir = dir.length() > 0.1f;
    for (int i = 0; i < static_cast<int>(premios_.size()); ++i) {
        if (!premios_[i].vivo) continue;
        Vector2D centro = mapa.centroCelda(premios_[i].col, premios_[i].fila);
        Vector2D hacia = centro - pos;
        float dist = hacia.length();
        // Ya esta encima de la caja, o la caja queda delante y cerca.
        bool encima = dist < 20.0f;
        bool delante = hayDir && dist <= 110.0f && hacia.normalized().dot(n) > 0.40f;
        if (!encima && !delante) continue;
        if (dist < mejorDist) {
            mejorDist = dist;
            mejor = i;
        }
    }
    return mejor;
}

Vector2D Obstaculos::centroPremio(const Mapa& mapa, int indice) const {
    return mapa.centroCelda(premios_[indice].col, premios_[indice].fila);
}

bool Obstaculos::tocaPremio(const Mapa& mapa, int indice, const Aabb& cuerpo) const {
    if (indice < 0 || indice >= static_cast<int>(premios_.size())) return false;
    if (!premios_[indice].vivo) return false;
    return cuerpo.toca(mapa.cajaCelda(premios_[indice].col, premios_[indice].fila));
}

void Obstaculos::recogerPremio(int indice) {
    if (indice < 0 || indice >= static_cast<int>(premios_.size())) return;
    if (!premios_[indice].vivo) return;
    // Solo se marca recogido; el mapa no se modifica.
    premios_[indice].vivo = false;
}

int Obstaculos::recogerPremiosEn(Mapa& mapa, const Aabb& cuerpo) {
    int recogidos = 0;
    for (int i = 0; i < static_cast<int>(premios_.size()); ++i) {
        if (!premios_[i].vivo) continue;
        if (tocaPremio(mapa, i, cuerpo)) {
            recogerPremio(i);
            ++recogidos;
        }
    }
    return recogidos;
}

bool Obstaculos::premioEn(const Mapa& mapa, int col, int fila) const {
    if (!mapa.dentro(col, fila)) return false;
    for (const Premio& p : premios_) {
        if (p.vivo && p.col == col && p.fila == fila) return true;
    }
    return false;
}

void Obstaculos::dibujarPremios(SDL_Renderer* r, SDL_Texture* sprite, const Mapa&, int camY,
                                int margenX, int altoVista) const {
    if (!sprite) return;

    int texW = 0;
    int texH = 0;
    if (SDL_QueryTexture(sprite, nullptr, nullptr, &texW, &texH) != 0) return;
    const int frameW = kPremioAncho;
    const int frameH = texH;
    if (texW < frameW || frameH < 1) return;
    int variantes = texW / frameW;
    if (variantes < 1) variantes = 1;

    int filaIni = camY / kTile;
    int filaFin = (camY + altoVista) / kTile;
    if (filaIni < 0) filaIni = 0;
    if (filaFin >= kFilas) filaFin = kFilas - 1;

    for (const Premio& p : premios_) {
        if (!p.vivo) continue;
        if (p.fila < filaIni - 1 || p.fila > filaFin + 1) continue;
        int variante = p.variante % variantes;
        SDL_Rect src{variante * frameW, 0, frameW, frameH};
        int dstW = static_cast<int>(frameW * kPremioEscala);
        int dstH = static_cast<int>(frameH * kPremioEscala);
        int cx = margenX + p.col * kTile + kTile / 2;
        int cy = p.fila * kTile + kTile / 2 - camY;
        SDL_Rect dst{cx - dstW / 2, cy - dstH / 2, dstW, dstH};
        SDL_RenderCopy(r, sprite, &src, &dst);
    }
}

void Obstaculos::dibujarDebug(SDL_Renderer* r, const Mapa& mapa, int camY, int margenX,
                              int altoVista) const {
    int filaIni = camY / kTile;
    int filaFin = (camY + altoVista) / kTile;
    if (filaIni < 0) filaIni = 0;
    if (filaFin >= kFilas) filaFin = kFilas - 1;

    for (int fila = filaIni; fila <= filaFin; ++fila) {
        for (int col = 0; col < kColumnas; ++col) {
            if (!solido_[fila][col] && !agua_[fila][col]) continue;
            Aabb c = mapa.cajaCelda(col, fila);
            SDL_Rect rect{margenX + static_cast<int>(c.x), static_cast<int>(c.y) - camY,
                          static_cast<int>(c.w), static_cast<int>(c.h)};
            if (solido_[fila][col]) {
                SDL_SetRenderDrawColor(r, 230, 70, 70, 255);
            } else {
                SDL_SetRenderDrawColor(r, 70, 150, 255, 255);
            }
            SDL_RenderDrawRect(r, &rect);
        }
    }

    SDL_SetRenderDrawColor(r, 255, 220, 60, 255);
    for (const Premio& p : premios_) {
        if (!p.vivo) continue;
        Aabb c = mapa.cajaCelda(p.col, p.fila);
        int y = static_cast<int>(c.y) - camY;
        if (y + kTile < 0 || y > altoVista) continue;
        SDL_Rect rect{margenX + static_cast<int>(c.x), y, static_cast<int>(c.w),
                      static_cast<int>(c.h)};
        SDL_RenderDrawRect(r, &rect);
    }
}
