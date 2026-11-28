#include "Game/Juego.h"

#include <cstdio>
#include <fstream>
#include <string>

#include "Engine/Matrix3x3.h"
#include "Game/Burbuja.h"

namespace {

constexpr int kVidasInicio = 3;

bool existeArchivo(const std::string& ruta) {
    std::ifstream f(ruta.c_str());
    return static_cast<bool>(f);
}

}  // namespace

std::string Juego::buscarAsset(const char* nombre) const {
    char* base = SDL_GetBasePath();
    if (base) {
        std::string junto = std::string(base) + "assets/" + nombre;
        SDL_free(base);
        if (existeArchivo(junto)) return junto;
    }
    std::string corto = std::string("assets/") + nombre;
    if (existeArchivo(corto)) return corto;
    return std::string("labs/s06_sumativa_i/assets/") + nombre;
}

bool Juego::iniciar(SDL_Renderer* renderer) {
    renderer_ = renderer;
    std::string spriteRuta = buscarAsset("player_sprite.bmp");
    SDL_Surface* superficie = SDL_LoadBMP(spriteRuta.c_str());
    if (!superficie) {
        std::fprintf(stderr, "No se pudo abrir la botella: %s\n", spriteRuta.c_str());
        return false;
    }
    sprite_ = SDL_CreateTextureFromSurface(renderer_, superficie);
    SDL_FreeSurface(superficie);
    if (!sprite_) {
        std::fprintf(stderr, "No se pudo crear la textura de la botella.\n");
        return false;
    }
    SDL_SetTextureBlendMode(sprite_, SDL_BLENDMODE_BLEND);

    // Premios: hoja de sprites con 4 variantes de 16 px (alto libre). BMP 32 bits con alfa.
    std::string premiosRuta = buscarAsset("premios.bmp");
    if (existeArchivo(premiosRuta)) {
        SDL_Surface* premiosSup = SDL_LoadBMP(premiosRuta.c_str());
        if (premiosSup) {
            premios_ = SDL_CreateTextureFromSurface(renderer_, premiosSup);
            SDL_FreeSurface(premiosSup);
            if (premios_) SDL_SetTextureBlendMode(premios_, SDL_BLENDMODE_BLEND);
        }
    }
    if (!premios_) {
        std::fprintf(stderr, "Aviso: no se pudo cargar %s; los premios siguen recogibles.\n",
                     premiosRuta.c_str());
    }

    if (!mapa_.cargar(renderer_, buscarAsset("mapa.txt"), buscarAsset("mapa.bmp"))) return false;
    obstaculos_.clasificarDesdeMapa(mapa_);

    ordenUI_.reiniciarAMenu();
    reiniciarPartida();
    return true;
}

void Juego::soltar() {
    mapa_.soltar();
    if (sprite_) {
        SDL_DestroyTexture(sprite_);
        sprite_ = nullptr;
    }
    if (premios_) {
        SDL_DestroyTexture(premios_);
        premios_ = nullptr;
    }
    renderer_ = nullptr;
}

void Juego::reiniciarPartida() {
    mapa_.reiniciarVisual(renderer_);
    obstaculos_.reiniciar();
    vidas_ = kVidasInicio;
    puntos_ = 0;
    // Columna 6, fila 62. Abajo del mapa, sobre pasto.
    personaje_.reiniciar(mapa_.centroCelda(6, 62));
    pedirSalto_ = false;
}

void Juego::tecla(SDL_Keycode key, bool& salir) {
    if (key == SDLK_1) modoFps_ = 1;
    if (key == SDLK_2) modoFps_ = 2;
    if (key == SDLK_3) modoFps_ = 3;
    if (key == SDLK_h || key == SDLK_F1) debugHitbox_ = !debugHitbox_;

    AccionUI accion = ordenUI_.procesarTecla(key);
    if (accion == AccionUI::IniciarPartida || accion == AccionUI::ReintentarPartida) {
        reiniciarPartida();
    } else if (accion == AccionUI::Saltar) {
        pedirSalto_ = true;
    } else if (accion == AccionUI::Salir) {
        salir = false;
    }
}

void Juego::actualizar(float dt) {
    personaje_.copiarPrevio();

    if (ordenUI_.enMenu()) {
        tiempoMenu_ += dt;
        pedirSalto_ = false;
        return;
    }
    if (!ordenUI_.enJuego()) {
        pedirSalto_ = false;
        return;
    }

    const Uint8* teclas = SDL_GetKeyboardState(nullptr);
    ResultadoPersonaje res =
        personaje_.actualizar(dt, teclas, pedirSalto_, renderer_, mapa_, obstaculos_);
    pedirSalto_ = false;

    puntos_ += res.puntosGanados;
    if (res.perdioVida) {
        vidas_ -= 1;
        if (vidas_ <= 0) {
            ordenUI_.abrirDerrota();
            return;
        }
    }
    if (res.gano) {
        ordenUI_.abrirVictoria();
    }
}

void Juego::anotarMs(float frameMs) {
    fpsMs_ += frameMs;
    fpsFrames_ += 1;
    if (fpsMs_ >= 400.0f) {
        if (fpsMs_ > 0.0f) {
            fpsNumero_ = static_cast<int>((static_cast<float>(fpsFrames_) * 1000.0f) / fpsMs_);
        }
        fpsMs_ = 0.0f;
        fpsFrames_ = 0;
    }
}

void Juego::dibujarMundo(float alpha) {
    Vector2D pos = personaje_.posInterpolada(alpha);
    float cam = pos.y - static_cast<float>(kVentanaH) * 0.62f;
    if (cam < 0.0f) cam = 0.0f;
    float camMax = static_cast<float>(kMapaAlto - kVentanaH);
    if (cam > camMax) cam = camMax;
    camY_ = static_cast<int>(cam);

    if (debugHitbox_) {
        obstaculos_.dibujarDebug(renderer_, mapa_, camY_, kMargenX, kVentanaH);
        personaje_.dibujarDebug(renderer_, alpha, camY_, kMargenX);
        return;
    }

    mapa_.dibujar(renderer_, camY_, kMargenX, kVentanaH);
    // Los premios se dibujan encima del mapa.
    obstaculos_.dibujarPremios(renderer_, premios_, mapa_, camY_, kMargenX, kVentanaH);
    Matrix3x3 matrizPadre;
    if (personaje_.dibujar(renderer_, sprite_, alpha, camY_, kMargenX, matrizPadre)) {
        dibujarBurbuja(renderer_, matrizPadre, personaje_.destinoBurbuja(alpha),
                       personaje_.estaSaltando(), kMargenX, camY_);
    }
}

void Juego::dibujar(float alpha) {
    SDL_SetRenderDrawColor(renderer_, 12, 28, 16, 255);
    SDL_RenderClear(renderer_);

    if (ordenUI_.enMenu()) {
        ui_.dibujarMenu(renderer_, sprite_, tiempoMenu_, ordenUI_.opcionMenu());
        personaje_.limpiarSinInterpolar();
        return;
    }

    dibujarMundo(alpha);
    ui_.dibujarHud(renderer_, vidas_, puntos_, modoFps_, fpsNumero_, debugHitbox_);
    ui_.dibujarOverlay(renderer_, ordenUI_.actual());
    personaje_.limpiarSinInterpolar();
}
