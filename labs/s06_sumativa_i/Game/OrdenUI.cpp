#include "Game/OrdenUI.h"

OrdenUI::OrdenUI() { reiniciarAMenu(); }

void OrdenUI::reiniciarAMenu() {
    pila_.clear();
    pila_.push_back(Pantalla::Menu);
}

void OrdenUI::abrirJuego() { pila_.push_back(Pantalla::Juego); }

void OrdenUI::abrirVictoria() { pila_.push_back(Pantalla::Victoria); }

void OrdenUI::abrirDerrota() { pila_.push_back(Pantalla::Derrota); }

Pantalla OrdenUI::actual() const {
    if (pila_.empty()) return Pantalla::Menu;
    return pila_.back();
}

AccionUI OrdenUI::procesarTecla(SDL_Keycode key) {
    Pantalla ahora = actual();

    if (ahora == Pantalla::Menu) {
        if (key == SDLK_w || key == SDLK_UP || key == SDLK_s || key == SDLK_DOWN) {
            opcionMenu_ = 1 - opcionMenu_;
        }
        if (key == SDLK_RETURN || key == SDLK_SPACE) {
            if (opcionMenu_ == 0) {
                abrirJuego();
                return AccionUI::IniciarPartida;
            }
            return AccionUI::Salir;
        }
        if (key == SDLK_ESCAPE) return AccionUI::Salir;
        return AccionUI::Ninguna;
    }

    if (ahora == Pantalla::Juego) {
        if (key == SDLK_p || key == SDLK_ESCAPE) pila_.push_back(Pantalla::Pausa);
        if (key == SDLK_SPACE) return AccionUI::Saltar;
        return AccionUI::Ninguna;
    }

    if (ahora == Pantalla::Pausa) {
        if (key == SDLK_RETURN || key == SDLK_p) {
            if (!pila_.empty()) pila_.pop_back();
        }
        if (key == SDLK_ESCAPE) reiniciarAMenu();
        return AccionUI::Ninguna;
    }

    if (ahora == Pantalla::Victoria || ahora == Pantalla::Derrota) {
        if (key == SDLK_RETURN || key == SDLK_SPACE) {
            reiniciarAMenu();
            abrirJuego();
            return AccionUI::ReintentarPartida;
        }
        if (key == SDLK_ESCAPE) reiniciarAMenu();
    }

    return AccionUI::Ninguna;
}
