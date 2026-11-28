#include <SDL.h>

#include <cstdio>

#include "Game/Juego.h"

// Paso fijo. El dibujo usa el sobrante del acumulador (alpha).
const double MS_PER_UPDATE = 16.66;

int main(int argc, char* argv[]) {
    (void)argc;
    (void)argv;

    if (SDL_Init(SDL_INIT_VIDEO | SDL_INIT_EVENTS) != 0) {
        std::fprintf(stderr, "SDL_Init fallo: %s\n", SDL_GetError());
        return 1;
    }
    SDL_SetHint(SDL_HINT_RENDER_SCALE_QUALITY, "0");

    int wx = SDL_WINDOWPOS_CENTERED;
    int wy = SDL_WINDOWPOS_CENTERED;
    if (const char* ex = SDL_getenv("SUMATIVA_X")) wx = SDL_atoi(ex);
    if (const char* ey = SDL_getenv("SUMATIVA_Y")) wy = SDL_atoi(ey);

    SDL_Window* ventana =
        SDL_CreateWindow("Evaluacion Sumativa I", wx, wy, kVentanaW, kVentanaH, SDL_WINDOW_SHOWN);
    SDL_Renderer* renderer = nullptr;
    if (ventana) {
        renderer = SDL_CreateRenderer(ventana, -1, SDL_RENDERER_ACCELERATED);
        if (!renderer) renderer = SDL_CreateRenderer(ventana, -1, SDL_RENDERER_SOFTWARE);
    }
    if (!ventana || !renderer) {
        std::fprintf(stderr, "No se pudo abrir la ventana: %s\n", SDL_GetError());
        SDL_Quit();
        return 1;
    }
    SDL_SetRenderDrawBlendMode(renderer, SDL_BLENDMODE_BLEND);

    Juego juego;
    if (!juego.iniciar(renderer)) {
        SDL_DestroyRenderer(renderer);
        SDL_DestroyWindow(ventana);
        SDL_Quit();
        return 1;
    }

    bool corre = true;
    Uint32 anterior = SDL_GetTicks();
    double acumulador = 0.0;

    while (corre) {
        SDL_Event e;
        while (SDL_PollEvent(&e)) {
            if (e.type == SDL_QUIT) corre = false;
            if (e.type == SDL_KEYDOWN && e.key.repeat == 0) {
                juego.tecla(e.key.keysym.sym, corre);
            }
        }

        Uint32 ahora = SDL_GetTicks();
        double frameMs = static_cast<double>(ahora - anterior);
        anterior = ahora;
        if (frameMs > 250.0) frameMs = 250.0;
        juego.anotarMs(static_cast<float>(frameMs));

        // La pausa tambien consume el tiempo, pero su actualizar no mueve nada.
        // Asi, al volver, el personaje no sale disparado.
        acumulador += frameMs;
        while (acumulador >= MS_PER_UPDATE) {
            juego.actualizar(static_cast<float>(MS_PER_UPDATE / 1000.0));
            acumulador -= MS_PER_UPDATE;
        }

        float alpha = static_cast<float>(acumulador / MS_PER_UPDATE);
        juego.dibujar(alpha);
        SDL_RenderPresent(renderer);

        // Teclas 1, 2 y 3. El paso fijo no cambia: solo se espera mas o menos.
        if (juego.modoFps() == 2) SDL_Delay(16);
        if (juego.modoFps() == 3) SDL_Delay(33);
    }

    juego.soltar();
    SDL_DestroyRenderer(renderer);
    SDL_DestroyWindow(ventana);
    SDL_Quit();
    return 0;
}
