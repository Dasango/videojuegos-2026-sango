#pragma once

#include <SDL.h>

#include "Engine/Aabb.h"
#include "Engine/Matrix3x3.h"
#include "Engine/Vector2D.h"
#include "Game/Mapa.h"
#include "Game/Obstaculos.h"

struct ResultadoPersonaje {
    int puntosGanados = 0;
    bool perdioVida = false;
    bool gano = false;
};

// Controla todo lo del personaje (botella): movimiento, rotacion, salto, muerte y animacion.
class Personaje {
public:
    void reiniciar(const Vector2D& posInicial);
    void copiarPrevio();
    void limpiarSinInterpolar();

    ResultadoPersonaje actualizar(float dt, const Uint8* teclas, bool pedirSalto, SDL_Renderer* r,
                                  Mapa& mapa, Obstaculos& obstaculos);

    Vector2D posInterpolada(float alpha) const;
    Vector2D destinoBurbuja(float alpha) const;
    bool estaSaltando() const { return saltando_; }
    bool estaMuriendo() const { return muriendo_; }
    Aabb cuerpoEn(const Vector2D& pos) const;

    // Dibuja sombra y sprite de la botella. Devuelve true si la burbuja debe dibujarse.
    bool dibujar(SDL_Renderer* r, SDL_Texture* sprite, float alpha, int camY, int margenX,
                 Matrix3x3& matrizPadreOut) const;

    // Dibuja solo la hitbox de la botella sin usar sprites.
    void dibujarDebug(SDL_Renderer* r, float alpha, int camY, int margenX) const;

private:
    void moverEnSuelo(float dt, const Vector2D& dir, const Mapa& mapa,
                      const Obstaculos& obstaculos);
    void resolverCajas(const Mapa& mapa, const Obstaculos& obstaculos);
    Vector2D calcularDestinoSalto() const;
    bool estaEnPeligro(const Mapa& mapa, const Obstaculos& obstaculos) const;
    void empezarSalto(const Vector2D& destino);
    void empezarMuerte(bool forzar = false);
    void terminarMuerte();
    void actualizarAnimacion(float dt, const Uint8* teclas);

    void dibujarSombra(SDL_Renderer* r, float sx, float sy, float z) const;
    void dibujarBotella(SDL_Renderer* r, SDL_Texture* sprite, float sx, float sy, float angulo,
                        float escala, int fila, int col) const;

    Vector2D pos_;
    Vector2D posPrev_;
    float angulo_ = 0.0f;
    float anguloPrev_ = 0.0f;
    float z_ = 0.0f;
    float zPrev_ = 0.0f;
    float escala_ = 1.0f;
    float escalaPrev_ = 1.0f;
    float vz_ = 0.0f;
    bool isGrounded_ = true;
    bool saltando_ = false;
    float tiempoSalto_ = 0.0f;
    Vector2D velSalto_;
    Vector2D destinoSalto_;
    Vector2D destinoPrevisto_;
    Vector2D destinoPrevistoPrev_;
    float impulso_ = 0.0f;
    int sentidoSalto_ = 1;
    int frameGiro_ = 0;
    float tiempoGiro_ = 0.0f;
    int frameSalto_ = 0;
    float tiempoSaltoFrame_ = 0.0f;
    bool muriendo_ = false;
    int frameMuerte_ = 0;
    float tiempoMuerte_ = 0.0f;
    float invuln_ = 0.0f;
    Vector2D seguro_;
    bool sinInterpolar_ = false;
    float relojPrint_ = 0.0f;
};
