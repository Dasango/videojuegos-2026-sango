#include <SDL.h>
#include <cstdio>

int main(int, char**) {
    // 1. Inicializar subsistema de video
    if (SDL_Init(SDL_INIT_VIDEO) != 0) {
        std::fprintf(stderr, "Error al inicializar SDL2: %s\n", SDL_GetError());
        return 1;
    }

    // 2. Crear ventana
    SDL_Window* window = SDL_CreateWindow(
        "Lab 2: Sincronizacion Temporal (Paso Fijo vs Delta Time)",
        SDL_WINDOWPOS_CENTERED,
        SDL_WINDOWPOS_CENTERED,
        800, 600,
        SDL_WINDOW_SHOWN
    );

    if (!window) {
        std::fprintf(stderr, "Error al crear ventana: %s\n", SDL_GetError());
        SDL_Quit();
        return 1;
    }

    // 3. Crear el renderer acelerado
    SDL_Renderer* renderer = SDL_CreateRenderer(window, -1, SDL_RENDERER_ACCELERATED);
    if (!renderer) {
        std::fprintf(stderr, "Error al crear renderer: %s\n", SDL_GetError());
        SDL_DestroyWindow(window);
        SDL_Quit();
        return 1;
    }

    // -------------------------------------------------------------
    // PARTE B: CARGA DE TEXTURA (sprite.bmp)
    // -------------------------------------------------------------
    SDL_Surface* surface = SDL_LoadBMP("sprite.bmp");
    if (!surface) {
        surface = SDL_LoadBMP("labs/s02_tiempo_fijo/sprite.bmp");
    }

    SDL_Texture* spriteTexture = nullptr;
    if (surface) {
        spriteTexture = SDL_CreateTextureFromSurface(renderer, surface);
        // Requisito 5: Liberar la superficie inmediatamente despues de crear la textura
        SDL_FreeSurface(surface);
    } else {
        std::fprintf(stderr, "Advertencia: No se pudo cargar sprite.bmp: %s\n", SDL_GetError());
    }

    // Estado del modo de dibujo: false = Rectangulos (Parte A), true = Sprites (Parte B)
    bool modoSprite = false;

    // Dimension del sprite / rectangulos (64x64 pixeles segun ficha tecnica)
    const int SPRITE_SIZE = 64;

    // Velocidad constante para ambos objetos (pixeles por segundo)
    const float VELOCIDAD = 250.0f;

    // Posiciones iniciales persistentes en coma flotante (fuera del bucle)
    float posX_fijo = 50.0f;
    float posX_variable = 50.0f;

    // Parametros para el Paso Logico Fijo con Acumulador (Lag Loop)
    const float MS_PER_UPDATE = 16.6667f; // 16.6 ms (~60 Hz)
    float lag = 0.0f;

    // Control de retardo para simulacion de caidas de FPS
    int retardoSimuladoMs = 0; // 0 ms = Nativo (1), 33 ms = ~30 FPS (2), 66 ms = ~15 FPS (3)

    // Tiempo anterior para calcular el delta time
    Uint32 previousTicks = SDL_GetTicks();

    bool running = true;
    while (running) {
        // --- 1. EVENTOS (Entrada Discreta) ---
        SDL_Event event;
        while (SDL_PollEvent(&event)) {
            if (event.type == SDL_QUIT) {
                running = false;
            }
            if (event.type == SDL_KEYDOWN) {
                switch (event.key.keysym.sym) {
                    case SDLK_ESCAPE:
                        running = false;
                        break;
                    case SDLK_1:
                        retardoSimuladoMs = 0; // FPS Nativos
                        break;
                    case SDLK_2:
                        retardoSimuladoMs = 33; // ~30 FPS
                        break;
                    case SDLK_3:
                        retardoSimuladoMs = 66; // ~15 FPS
                        break;
                    case SDLK_r:
                        modoSprite = false; // Parte A: Rectangulos
                        break;
                    case SDLK_t:
                        if (spriteTexture) {
                            modoSprite = true; // Parte B: Sprites
                        }
                        break;
                    default:
                        break;
                }
            }
        }

        // --- 2. MEDIR TIEMPO TRANSCURRIDO (DELTA TIME) ---
        Uint32 currentTicks = SDL_GetTicks();
        Uint32 elapsedTicks = currentTicks - previousTicks;
        previousTicks = currentTicks;

        // Limite de seguridad para evitar desbordamiento si se arrastra la ventana
        if (elapsedTicks > 250) {
            elapsedTicks = 250;
        }

        float dt = elapsedTicks / 1000.0f; // Tiempo real en segundos

        // Estado continuo del teclado (para mantener presionadas F y V)
        const Uint8* state = SDL_GetKeyboardState(nullptr);
        bool presionaF = (state[SDL_SCANCODE_F] != 0);
        bool presionaV = (state[SDL_SCANCODE_V] != 0);

        // --- 3. ACTUALIZAR: PASO LOGICO FIJO CON ACUMULADOR (LAG LOOP) -> TECLA 'F' ---
        lag += static_cast<float>(elapsedTicks);
        while (lag >= MS_PER_UPDATE) {
            if (presionaF) {
                posX_fijo += VELOCIDAD * (MS_PER_UPDATE / 1000.0f);
                if (posX_fijo > 730.0f) {
                    posX_fijo = 50.0f;
                }
            }
            lag -= MS_PER_UPDATE;
        }

        // --- 4. ACTUALIZAR: PASO VARIABLE (DELTA TIME) -> TECLA 'V' ---
        if (presionaV) {
            posX_variable += VELOCIDAD * dt;
            if (posX_variable > 730.0f) {
                posX_variable = 50.0f;
            }
        }

        // --- 5. TELEMETRIA EN EL TITULO DE LA VENTANA ---
        char titleBuffer[160];
        float fpsActual = (elapsedTicks > 0) ? (1000.0f / elapsedTicks) : 0.0f;
        std::snprintf(titleBuffer, sizeof(titleBuffer),
                      "FPS: %4.1f | Retardo: %2d ms | Modo: %s | Fijo X: %5.1f | Var X: %5.1f",
                      fpsActual, retardoSimuladoMs,
                      (modoSprite ? "Sprite [T]" : "Rectangulos [R]"),
                      posX_fijo, posX_variable);
        SDL_SetWindowTitle(window, titleBuffer);

        // --- 6. DIBUJAR ---
        SDL_SetRenderDrawColor(renderer, 30, 30, 45, 255); // Fondo
        SDL_RenderClear(renderer);

        // Linea divisoria de carriles
        SDL_SetRenderDrawColor(renderer, 60, 60, 80, 255);
        SDL_RenderDrawLine(renderer, 30, 260, 770, 260);

        // Mapeo de coordenadas logicas flotantes a SDL_Rect entero
        SDL_Rect rectFijo = {
            static_cast<int>(posX_fijo),
            140,
            SPRITE_SIZE,
            SPRITE_SIZE
        };

        SDL_Rect rectVariable = {
            static_cast<int>(posX_variable),
            340,
            SPRITE_SIZE,
            SPRITE_SIZE
        };

        if (!modoSprite) {
            // PARTE A: Formas geometricas primitivas
            // Objeto 1: Paso Fijo (Rojo)
            SDL_SetRenderDrawColor(renderer, 235, 50, 50, 255);
            SDL_RenderFillRect(renderer, &rectFijo);

            // Objeto 2: Tiempo Variable (Azul)
            SDL_SetRenderDrawColor(renderer, 50, 140, 245, 255);
            SDL_RenderFillRect(renderer, &rectVariable);
        } else {
            // PARTE B: Renderizado de Sprites (SDL_RenderCopy)
            SDL_Rect srcRect = { 0, 0, SPRITE_SIZE, SPRITE_SIZE };

            // Dibujar sprite superior (Paso Fijo)
            SDL_RenderCopy(renderer, spriteTexture, &srcRect, &rectFijo);

            // Dibujar sprite inferior (Tiempo Variable)
            SDL_RenderCopy(renderer, spriteTexture, &srcRect, &rectVariable);
        }

        // Presentar cuadro
        SDL_RenderPresent(renderer);

        // --- 7. SIMULACION DE CAIDAS DE FPS CON SDL_Delay() ---
        if (retardoSimuladoMs > 0) {
            SDL_Delay(static_cast<Uint32>(retardoSimuladoMs));
        }
    }

    // -------------------------------------------------------------
    // GESTION LIMPIA DE MEMORIA (Requisito 5)
    // -------------------------------------------------------------
    if (spriteTexture) {
        SDL_DestroyTexture(spriteTexture);
        spriteTexture = nullptr;
    }

    SDL_DestroyRenderer(renderer);
    SDL_DestroyWindow(window);
    SDL_Quit();

    return 0;
}
