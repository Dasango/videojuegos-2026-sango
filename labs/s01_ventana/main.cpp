// Semana 1 - Entorno funcionando y bucle de juego minimo.
//
// Un juego es un bucle: procesar entrada -> actualizar -> dibujar.
// Aqui escribimos ese bucle "a mano" usando solo SDL2.
#include <SDL.h>

#include <cstdio>

int main(int, char**) {
    if (SDL_Init(SDL_INIT_VIDEO) != 0) {
        std::fprintf(stderr, "SDL_Init fallo: %s\n", SDL_GetError());
        return 1;
    }

    SDL_Window* window = SDL_CreateWindow("Semana 1 - Mi primera ventana", SDL_WINDOWPOS_CENTERED,
                                          SDL_WINDOWPOS_CENTERED, 800, 600, SDL_WINDOW_SHOWN);
    SDL_Renderer* renderer = SDL_CreateRenderer(window, -1, SDL_RENDERER_ACCELERATED);
    if (!window || !renderer) {
        std::fprintf(stderr, "No se pudo crear ventana/renderer: %s\n", SDL_GetError());
        SDL_Quit();
        return 1;
    }

    bool running = true;
    while (running) {
        // 1) ENTRADA
        SDL_Event e;
        while (SDL_PollEvent(&e)) {
            if (e.type == SDL_QUIT) running = false;
            if (e.type == SDL_KEYDOWN && e.key.keysym.sym == SDLK_ESCAPE) running = false;
        }

        // 2) ACTUALIZAR
        // TODO (ejercicio 2): cambiar el color de fondo con el paso del tiempo.

        // 3) DIBUJAR
        SDL_SetRenderDrawColor(renderer, 30, 30, 46, 255);
        SDL_RenderClear(renderer);
        // TODO (ejercicio 1): dibujar un rectangulo con SDL_RenderFillRect.
        SDL_RenderPresent(renderer);
    }

    SDL_DestroyRenderer(renderer);
    SDL_DestroyWindow(window);
    SDL_Quit();
    return 0;
}
