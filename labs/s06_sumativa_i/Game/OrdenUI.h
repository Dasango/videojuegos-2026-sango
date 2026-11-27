#pragma once

#include <SDL.h>

#include <vector>

enum class Pantalla { Menu, Juego, Pausa, Victoria, Derrota };

enum class AccionUI { Ninguna, IniciarPartida, ReintentarPartida, Saltar, Salir };

// Controla el orden y las transiciones entre pantallas usando una pila.
class OrdenUI {
public:
    OrdenUI();

    void reiniciarAMenu();
    void abrirJuego();
    void abrirVictoria();
    void abrirDerrota();

    Pantalla actual() const;
    bool enMenu() const { return actual() == Pantalla::Menu; }
    bool enJuego() const { return actual() == Pantalla::Juego; }
    int opcionMenu() const { return opcionMenu_; }

    AccionUI procesarTecla(SDL_Keycode key);

private:
    std::vector<Pantalla> pila_;
    int opcionMenu_ = 0;
};
