// Version web (WebAssembly) del bucle principal.
// Reutiliza toda la logica de Game/Juego sin tocarla; solo cambia el
// bucle bloqueante de SDL por emscripten_set_main_loop_arg (rAF del navegador)
// y no usa SDL_Delay (el navegador controla el ritmo de cuadros).
#include <SDL.h>

#include <emscripten.h>
#include <emscripten/html5.h>

#include <cstdio>

#include "Game/Juego.h"

namespace {

// Paso fijo. El dibujo usa el sobrante del acumulador (alpha).
const double MS_PER_UPDATE = 16.66;

struct App {
    SDL_Window* ventana = nullptr;
    SDL_Renderer* renderer = nullptr;
    Juego juego;
    bool corre = true;
    Uint32 anterior = 0;
    double acumulador = 0.0;
};

void liberar(App& app) {
    app.juego.soltar();
    if (app.renderer) SDL_DestroyRenderer(app.renderer);
    if (app.ventana) SDL_DestroyWindow(app.ventana);
    app.renderer = nullptr;
    app.ventana = nullptr;
    SDL_Quit();
}

void frame(void* arg) {
    App& app = *static_cast<App*>(arg);

    SDL_Event e;
    while (SDL_PollEvent(&e)) {
        if (e.type == SDL_QUIT) app.corre = false;
        if (e.type == SDL_KEYDOWN && e.key.repeat == 0) {
            app.juego.tecla(e.key.keysym.sym, app.corre);
        }
    }

    if (!app.corre) {
        emscripten_cancel_main_loop();
        liberar(app);
        return;
    }

    Uint32 ahora = SDL_GetTicks();
    double frameMs = static_cast<double>(ahora - app.anterior);
    app.anterior = ahora;
    if (frameMs > 250.0) frameMs = 250.0;
    app.juego.anotarMs(static_cast<float>(frameMs));

    // La pausa tambien consume el tiempo, pero su actualizar no mueve nada.
    app.acumulador += frameMs;
    while (app.acumulador >= MS_PER_UPDATE) {
        app.juego.actualizar(static_cast<float>(MS_PER_UPDATE / 1000.0));
        app.acumulador -= MS_PER_UPDATE;
    }

    float alpha = static_cast<float>(app.acumulador / MS_PER_UPDATE);
    app.juego.dibujar(alpha);
    SDL_RenderPresent(app.renderer);
}

}  // namespace

int main(int argc, char* argv[]) {
    (void)argc;
    (void)argv;

    if (SDL_Init(SDL_INIT_VIDEO | SDL_INIT_EVENTS) != 0) {
        std::fprintf(stderr, "SDL_Init fallo: %s\n", SDL_GetError());
        return 1;
    }
    SDL_SetHint(SDL_HINT_RENDER_SCALE_QUALITY, "0");

    static App app;  // estatico: emscripten_set_main_loop_arg no copia el puntero.

    app.ventana = SDL_CreateWindow("Evaluacion Sumativa I", SDL_WINDOWPOS_CENTERED,
                                   SDL_WINDOWPOS_CENTERED, kVentanaW, kVentanaH, SDL_WINDOW_SHOWN);
    if (app.ventana) {
        app.renderer = SDL_CreateRenderer(app.ventana, -1, SDL_RENDERER_ACCELERATED);
        if (!app.renderer) app.renderer = SDL_CreateRenderer(app.ventana, -1, SDL_RENDERER_SOFTWARE);
    }
    if (!app.ventana || !app.renderer) {
        std::fprintf(stderr, "No se pudo abrir la ventana: %s\n", SDL_GetError());
        SDL_Quit();
        return 1;
    }
    SDL_SetRenderDrawBlendMode(app.renderer, SDL_BLENDMODE_BLEND);

    if (!app.juego.iniciar(app.renderer)) {
        liberar(app);
        return 1;
    }

    app.anterior = SDL_GetTicks();

    // fps = 0: un cuadro por requestAnimationFrame del navegador.
    emscripten_set_main_loop_arg(frame, &app, 0, 1);
    return 0;
}
