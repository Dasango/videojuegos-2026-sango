#include "Game/Personaje.h"

#include <algorithm>
#include <cmath>
#include <cstdio>

namespace {

constexpr float kVelocidad = 250.0f;
constexpr float g = 980.0f;
constexpr float kTiempoSalto = 0.70f;
constexpr float kVz0 = g * kTiempoSalto * 0.5f;
constexpr float kZMax = (kVz0 * kVz0) / (2.0f * g);
constexpr float kPi = 3.14159265f;
constexpr float kTiempoFrame = 1.0f / 12.0f;
constexpr float kTiempoFrameGiro = 1.0f / 32.0f;
constexpr int kFilaSalto = 4;
constexpr int kFilaGiro = 2;
constexpr int kFilaMuerte = 5;
constexpr int kSprite = 64;
constexpr int kPuntosCaja = 10;
constexpr float kCajaAncho = 30.0f;
constexpr float kCajaAlto = 12.0f;
constexpr float kVelGiro = 3.5f;
constexpr float kSaltoMinCeldas = 1.0f;
constexpr float kSaltoMaxCeldas = 4.0f;
constexpr float kTiempoCargaImpulso = 0.24f;
constexpr float kTiempoBajadaImpulso = 0.06f;
constexpr float kAnguloSprite = kPi * 0.5f;  // el sprite viene vertical; lo acostamos

// Esquinas de la caja orientada (eje largo = eje de la botella, perpendicular al avance).
void esquinas(const Vector2D& p, float a, Vector2D out[4]) {
    const float s = std::sin(a);
    const float c = std::cos(a);
    const float hl = kCajaAncho * 0.5f;
    const float hw = kCajaAlto * 0.5f;
    const float sl[4] = {-1.0f, 1.0f, 1.0f, -1.0f};
    const float sf[4] = {-1.0f, -1.0f, 1.0f, 1.0f};
    for (int i = 0; i < 4; ++i) {
        out[i] =
            Vector2D(p.x + c * hl * sl[i] + s * hw * sf[i], p.y + s * hl * sl[i] - c * hw * sf[i]);
    }
}

float envolver(float a) {
    while (a > kPi) a -= 2.0f * kPi;
    while (a < -kPi) a += 2.0f * kPi;
    return a;
}

float mezclar(float a, float b, float t) { return a + (b - a) * t; }

float mezclarAngulo(float a, float b, float t) { return a + envolver(b - a) * t; }

void elipse(SDL_Renderer* r, int cx, int cy, int rx, int ry, Uint8 ca) {
    if (rx < 1 || ry < 1) return;
    SDL_SetRenderDrawBlendMode(r, SDL_BLENDMODE_BLEND);
    SDL_SetRenderDrawColor(r, 0, 0, 0, ca);
    for (int dy = -ry; dy <= ry; ++dy) {
        float t = 1.0f - (static_cast<float>(dy * dy) / static_cast<float>(ry * ry));
        if (t < 0.0f) continue;
        int hw = static_cast<int>(static_cast<float>(rx) * std::sqrt(t));
        SDL_RenderDrawLine(r, cx - hw, cy + dy, cx + hw, cy + dy);
    }
}

}  // namespace

void Personaje::reiniciar(const Vector2D& posInicial) {
    *this = Personaje{};
    pos_ = posInicial;
    seguro_ = posInicial;
    angulo_ = 0.0f;  // mira al norte; la botella se dibuja acostada
    isGrounded_ = true;
    escala_ = 1.0f;
    destinoPrevisto_ = posInicial + Vector2D(0.0f, -static_cast<float>(kTile));
    destinoSalto_ = destinoPrevisto_;
    copiarPrevio();
    sinInterpolar_ = true;
}

void Personaje::copiarPrevio() {
    posPrev_ = pos_;
    anguloPrev_ = angulo_;
    zPrev_ = z_;
    escalaPrev_ = escala_;
    destinoPrevistoPrev_ = destinoPrevisto_;
}

void Personaje::limpiarSinInterpolar() { sinInterpolar_ = false; }

Aabb Personaje::cuerpoEn(const Vector2D& pos) const {
    return Aabb::desdeCentro(pos, kCajaAncho, kCajaAlto);
}

void Personaje::resolverCajas(const Mapa& mapa, const Obstaculos& obstaculos) {
    // Solo las cajas. El arbol y el agua se resuelven aparte, matando.
    int col = static_cast<int>(pos_.x / static_cast<float>(kTile));
    int fila = static_cast<int>(pos_.y / static_cast<float>(kTile));
    for (int pasada = 0; pasada < 4; ++pasada) {
        for (int fy = fila - 2; fy <= fila + 2; ++fy) {
            for (int fx = col - 2; fx <= col + 2; ++fx) {
                if (!obstaculos.premioEn(mapa, fx, fy)) continue;
                Vector2D empuje;
                if (cuerpoEn(pos_).mtv(mapa.cajaCelda(fx, fy), empuje)) {
                    pos_ = pos_ + empuje;
                }
            }
        }
    }
}

void Personaje::moverEnSuelo(float dt, const Vector2D& dir, const Mapa& mapa,
                             const Obstaculos& obstaculos) {
    // Eje a eje y luego el MTV, para deslizar por el borde de la caja.
    pos_.x += dir.x * kVelocidad * dt;
    resolverCajas(mapa, obstaculos);
    pos_.y += dir.y * kVelocidad * dt;
    resolverCajas(mapa, obstaculos);

    if (pos_.x < 8.0f) pos_.x = 8.0f;
    if (pos_.x > static_cast<float>(kMapaAncho) - 8.0f) {
        pos_.x = static_cast<float>(kMapaAncho) - 8.0f;
    }
    if (pos_.y < 8.0f) pos_.y = 8.0f;
    if (pos_.y > static_cast<float>(kMapaAlto) - 8.0f) {
        pos_.y = static_cast<float>(kMapaAlto) - 8.0f;
    }
}

Vector2D Personaje::calcularDestinoSalto() const {
    const Vector2D frente(std::sin(angulo_), -std::cos(angulo_));
    const Vector2D dir =
        (impulso_ > 0.001f) ? (frente * static_cast<float>(sentidoSalto_)) : frente;
    const float dist = (kSaltoMinCeldas + (kSaltoMaxCeldas - kSaltoMinCeldas) * impulso_) *
                       static_cast<float>(kTile);

    Vector2D cand = pos_ + dir * dist;
    if (cand.x < 8.0f) cand.x = 8.0f;
    if (cand.x > static_cast<float>(kMapaAncho) - 8.0f) {
        cand.x = static_cast<float>(kMapaAncho) - 8.0f;
    }
    if (cand.y < 8.0f) cand.y = 8.0f;
    if (cand.y > static_cast<float>(kMapaAlto) - 8.0f) {
        cand.y = static_cast<float>(kMapaAlto) - 8.0f;
    }
    return cand;
}

bool Personaje::estaEnPeligro(const Mapa& mapa, const Obstaculos& obstaculos) const {
    Vector2D esq[4];
    esquinas(pos_, angulo_, esq);
    const int maxCol = kMapaAncho / kTile - 1;
    const int maxFila = kMapaAlto / kTile - 1;
    for (const Vector2D& e : esq) {
        int col = static_cast<int>(e.x / static_cast<float>(kTile));
        int fila = static_cast<int>(e.y / static_cast<float>(kTile));
        if (col < 0) col = 0;
        if (col > maxCol) col = maxCol;
        if (fila < 0) fila = 0;
        if (fila > maxFila) fila = maxFila;
        if (obstaculos.esAgua(mapa, col, fila) || obstaculos.bloqueado(mapa, col, fila))
            return true;
    }
    return false;
}

void Personaje::empezarSalto(const Vector2D& destino) {
    saltando_ = true;
    isGrounded_ = false;
    destinoSalto_ = destino;
    destinoPrevisto_ = destino;
    destinoPrevistoPrev_ = destino;
    velSalto_ = (destino - pos_) * (1.0f / kTiempoSalto);

    tiempoSalto_ = 0.0f;
    vz_ = kVz0;
    z_ = 0.0f;
    frameSalto_ = 0;
    tiempoSaltoFrame_ = 0.0f;
}

void Personaje::empezarMuerte(bool forzar) {
    if (muriendo_ || (!forzar && invuln_ > 0.0f)) return;
    invuln_ = 0.0f;
    muriendo_ = true;
    saltando_ = false;
    isGrounded_ = true;
    z_ = 0.0f;
    vz_ = 0.0f;
    escala_ = 1.0f;
    frameMuerte_ = 0;
    tiempoMuerte_ = 0.0f;
}

void Personaje::terminarMuerte() {
    muriendo_ = false;
    pos_ = seguro_;
    z_ = 0.0f;
    vz_ = 0.0f;
    escala_ = 1.0f;
    saltando_ = false;
    isGrounded_ = true;
    invuln_ = 1.0f;
    sinInterpolar_ = true;
    copiarPrevio();
}

void Personaje::actualizarAnimacion(float dt, const Uint8* teclas) {
    // Al estar acostada (+90 deg), avanzar en fila mueve la etiqueta hacia abajo;
    // W retrocede la fila para rodar hacia adelante y S la avanza para rodar hacia atras.
    if (!saltando_ && !muriendo_) {
        int sentido = 0;
        if (teclas[SDL_SCANCODE_W] && !teclas[SDL_SCANCODE_S]) sentido = -1;
        if (teclas[SDL_SCANCODE_S] && !teclas[SDL_SCANCODE_W]) sentido = 1;
        if (sentido != 0) {
            tiempoGiro_ += dt;
            while (tiempoGiro_ >= kTiempoFrameGiro) {
                tiempoGiro_ -= kTiempoFrameGiro;
                frameGiro_ += sentido;
                if (frameGiro_ >= 9) frameGiro_ = 0;
                if (frameGiro_ < 0) frameGiro_ = 8;
            }
        }
    } else if (saltando_) {
        tiempoSaltoFrame_ += dt;
        while (tiempoSaltoFrame_ >= kTiempoFrame) {
            tiempoSaltoFrame_ -= kTiempoFrame;
            frameSalto_ += 1;
            if (frameSalto_ >= 9) frameSalto_ = 0;
        }
    }
}

ResultadoPersonaje Personaje::actualizar(float dt, const Uint8* teclas, bool pedirSalto,
                                         SDL_Renderer*, Mapa& mapa, Obstaculos& obstaculos) {
    ResultadoPersonaje res;

    // A/D solo giran sobre el sitio. El pivote es el centro, que es donde ya giraba,
    // asi la botella no se corre. T(pivote) * R * T(-pivote).
    if (!saltando_ && !muriendo_) {
        float giro = 0.0f;
        if (teclas[SDL_SCANCODE_D]) giro += 1.0f;
        if (teclas[SDL_SCANCODE_A]) giro -= 1.0f;
        float paso = giro * kVelGiro * dt;
        if (paso != 0.0f) {
            Vector2D pivote = pos_;
            Matrix3x3 giroM = Matrix3x3::traslacion(pivote.x, pivote.y) *
                              Matrix3x3::rotacion(paso) *
                              Matrix3x3::traslacion(-pivote.x, -pivote.y);
            pos_ = giroM.transformar(pos_);
            angulo_ = envolver(angulo_ + paso);
        }
    }

    // W/S avanzan/retroceden en la direccion a la que mira el personaje.
    float avance = 0.0f;
    if (teclas[SDL_SCANCODE_W]) avance += 1.0f;
    if (teclas[SDL_SCANCODE_S]) avance -= 1.0f;

    if (!saltando_ && !muriendo_) {
        if (avance != 0.0f) {
            int nuevoSentido = (avance > 0.0f) ? 1 : -1;
            if (nuevoSentido != sentidoSalto_) {
                impulso_ = 0.0f;
                sentidoSalto_ = nuevoSentido;
            }
            impulso_ = std::min(1.0f, impulso_ + dt / kTiempoCargaImpulso);
        } else {
            impulso_ = std::max(0.0f, impulso_ - dt / kTiempoBajadaImpulso);
            if (impulso_ <= 0.001f) {
                impulso_ = 0.0f;
                sentidoSalto_ = 1;
            }
        }
    }

    const Vector2D frente(std::sin(angulo_), -std::cos(angulo_));
    Vector2D crudo = frente * avance;
    float mag = crudo.length();
    // Siempre se normaliza. Con W o S la longitud ya es 1; en diagonal no hay extra.
    Vector2D dir = crudo.normalized();
    const Vector2D norte(0.0f, -1.0f);
    float dotNorte = dir.dot(norte);

    relojPrint_ += dt;
    if (relojPrint_ >= 0.10f) {
        relojPrint_ = 0.0f;
        std::printf("Mag: %6.4f | Norm: (%7.4f, %7.4f) | Dot Norte: %7.4f | Modo: Normalizado\n",
                    mag, dir.x, dir.y, dotNorte);
        std::fflush(stdout);
    }

    if (invuln_ > 0.0f) invuln_ -= dt;

    if (muriendo_) {
        tiempoMuerte_ += dt;
        while (tiempoMuerte_ >= kTiempoFrame && frameMuerte_ < 8) {
            tiempoMuerte_ -= kTiempoFrame;
            frameMuerte_ += 1;
        }
        if (frameMuerte_ >= 8 && tiempoMuerte_ >= kTiempoFrame) {
            terminarMuerte();
            res.perdioVida = true;
        }
        return res;
    }

    if (!saltando_ && pedirSalto && isGrounded_) {
        empezarSalto(calcularDestinoSalto());
    }

    if (saltando_) {
        tiempoSalto_ += dt;
        pos_ = pos_ + velSalto_ * dt;
        // Euler semi-implicito: primero la velocidad, despues la altura.
        vz_ = vz_ - g * dt;
        z_ = z_ + vz_ * dt;
        if (z_ < 0.0f || tiempoSalto_ >= kTiempoSalto) {
            z_ = 0.0f;
            vz_ = 0.0f;
            saltando_ = false;
            isGrounded_ = true;
            pos_ = destinoSalto_;

            int recogidos = obstaculos.recogerPremiosEn(mapa, cuerpoEn(pos_));
            res.puntosGanados += recogidos * kPuntosCaja;

            if (recogidos == 0 || estaEnPeligro(mapa, obstaculos)) {
                empezarMuerte(true);
            } else if (obstaculos.pisoSeguro(mapa, pos_)) {
                seguro_ = pos_;
            }
        }
    } else {
        isGrounded_ = true;
        z_ = 0.0f;
        moverEnSuelo(dt, dir, mapa, obstaculos);
        if (estaEnPeligro(mapa, obstaculos)) {
            empezarMuerte();
        } else if (obstaculos.pisoSeguro(mapa, pos_)) {
            seguro_ = pos_;
        }
    }

    if (saltando_) {
        destinoPrevisto_ = destinoSalto_;
        destinoPrevistoPrev_ = destinoSalto_;
    } else {
        destinoPrevisto_ = calcularDestinoSalto();
    }

    escala_ = 1.0f + z_ / kZMax;
    if (!saltando_) escala_ = 1.0f;

    actualizarAnimacion(dt, teclas);

    if (isGrounded_ && !muriendo_ && mapa.filaDe(pos_.y) <= 1) {
        res.gano = true;
    }
    return res;
}

Vector2D Personaje::posInterpolada(float alpha) const {
    float t = sinInterpolar_ ? 1.0f : alpha;
    return {mezclar(posPrev_.x, pos_.x, t), mezclar(posPrev_.y, pos_.y, t)};
}

Vector2D Personaje::destinoBurbuja(float alpha) const {
    if (saltando_) return destinoSalto_;
    float t = sinInterpolar_ ? 1.0f : alpha;
    return {mezclar(destinoPrevistoPrev_.x, destinoPrevisto_.x, t),
            mezclar(destinoPrevistoPrev_.y, destinoPrevisto_.y, t)};
}

void Personaje::dibujarSombra(SDL_Renderer* r, float sx, float sy, float z) const {
    float t = z / kZMax;
    if (t < 0.0f) t = 0.0f;
    if (t > 1.0f) t = 1.0f;
    Uint8 alfa = static_cast<Uint8>(120.0f - t * 70.0f);
    elipse(r, static_cast<int>(sx), static_cast<int>(sy), 16, 6, alfa);
}

void Personaje::dibujarBotella(SDL_Renderer* r, SDL_Texture* sprite, float sx, float sy,
                               float angulo, float escala, int fila, int col) const {
    int lado = static_cast<int>(static_cast<float>(kSprite) * escala);
    if (lado < 1) lado = 1;
    SDL_Rect src{col * kSprite, fila * kSprite, kSprite, kSprite};
    SDL_Rect dst{static_cast<int>(sx) - lado / 2, static_cast<int>(sy) - lado / 2, lado, lado};
    SDL_Point centro{lado / 2, lado / 2};
    double grados = static_cast<double>(angulo) * (180.0 / static_cast<double>(kPi));
    SDL_RenderCopyEx(r, sprite, &src, &dst, grados, &centro, SDL_FLIP_NONE);
}

bool Personaje::dibujar(SDL_Renderer* r, SDL_Texture* sprite, float alpha, int camY, int margenX,
                        Matrix3x3& matrizPadreOut) const {
    float t = sinInterpolar_ ? 1.0f : alpha;
    Vector2D pos = posInterpolada(alpha);
    float z = mezclar(zPrev_, z_, t);
    float angulo = mezclarAngulo(anguloPrev_, angulo_, t);
    float escala = mezclar(escalaPrev_, escala_, t);
    if (escala < 1.0f) escala = 1.0f;

    float sx = pos.x + static_cast<float>(margenX);
    float sySuelo = pos.y - static_cast<float>(camY);
    float sy = sySuelo - z;
    dibujarSombra(r, sx, sySuelo, z);

    bool parpadeo = invuln_ > 0.0f && (static_cast<int>(invuln_ * 14.0f) % 2) == 0;

    int fila = kFilaGiro;
    int col = frameGiro_;
    float anguloSprite = angulo + kAnguloSprite;
    if (saltando_) {
        fila = kFilaSalto;
        col = frameSalto_;
    } else if (muriendo_) {
        fila = kFilaMuerte;
        col = frameMuerte_;
        anguloSprite = kAnguloSprite;
    }

    matrizPadreOut = Matrix3x3::traslacion(pos.x, pos.y - z) * Matrix3x3::rotacion(anguloSprite) *
                     Matrix3x3::escala(escala, escala);
    if (!parpadeo) {
        float escDibujo = matrizPadreOut.ejeX().length();
        dibujarBotella(r, sprite, sx, sy, anguloSprite, escDibujo, fila, col);
    }
    return !muriendo_;
}

void Personaje::dibujarDebug(SDL_Renderer* r, float alpha, int camY, int margenX) const {
    float t = sinInterpolar_ ? 1.0f : alpha;
    Vector2D esq[4];
    esquinas(posInterpolada(alpha), mezclarAngulo(anguloPrev_, angulo_, t), esq);
    SDL_SetRenderDrawColor(r, 80, 255, 80, 255);
    for (int i = 0; i < 4; ++i) {
        const Vector2D& a = esq[i];
        const Vector2D& b = esq[(i + 1) % 4];
        SDL_RenderDrawLine(r, margenX + static_cast<int>(a.x), static_cast<int>(a.y) - camY,
                           margenX + static_cast<int>(b.x), static_cast<int>(b.y) - camY);
    }
}
