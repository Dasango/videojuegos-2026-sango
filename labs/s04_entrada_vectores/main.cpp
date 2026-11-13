#include <SDL.h>
#include <cmath>
#include <cstdio>

struct Vec2 {
    float x = 0.0f;
    float y = 0.0f;

    Vec2() = default;
    Vec2(float x_, float y_) : x(x_), y(y_) {}

    Vec2 operator+(const Vec2& o) const { return {x + o.x, y + o.y}; }
    Vec2 operator-(const Vec2& o) const { return {x - o.x, y - o.y}; }
    Vec2 operator*(float s) const { return {x * s, y * s}; }

    float length() const {
        return std::sqrt(x * x + y * y);
    }

    Vec2 normalized() const {
        float len = length();
        if (len > 0.0001f) {
            return {x / len, y / len};
        }
        return {0.0f, 0.0f};
    }

    float dot(const Vec2& o) const {
        return x * o.x + y * o.y;
    }
};

int main(int, char**) {
    if (SDL_Init(SDL_INIT_VIDEO) != 0) {
        std::fprintf(stderr, "Error al inicializar SDL2: %s\n", SDL_GetError());
        return 1;
    }

    const int SCREEN_WIDTH = 800;
    const int SCREEN_HEIGHT = 600;
    const int SPRITE_SIZE = 64;
    const float SPEED = 250.0f;
    // La hoja tiene 9 columnas. Fila 0 = quieto, fila 2 = caminando.
    const int SHEET_COLS = 9;
    const float FRAME_TIME = 1.0f / 12.0f;

    SDL_Window* window = SDL_CreateWindow(
        "Lab 3: Entrada, Normalizacion y Restriccion de Bordes",
        SDL_WINDOWPOS_CENTERED,
        SDL_WINDOWPOS_CENTERED,
        SCREEN_WIDTH,
        SCREEN_HEIGHT,
        SDL_WINDOW_SHOWN
    );

    if (!window) {
        std::fprintf(stderr, "Error al crear ventana: %s\n", SDL_GetError());
        SDL_Quit();
        return 1;
    }

    SDL_Renderer* renderer = SDL_CreateRenderer(window, -1, SDL_RENDERER_ACCELERATED);
    if (!renderer) {
        std::fprintf(stderr, "Error al crear renderer: %s\n", SDL_GetError());
        SDL_DestroyWindow(window);
        SDL_Quit();
        return 1;
    }

    // BMP de 32 bits con transparencia, copiado junto al ejecutable.
    SDL_Surface* surface = SDL_LoadBMP("player_sprite.bmp");
    if (!surface) {
        surface = SDL_LoadBMP("labs/s04_entrada_vectores/player_sprite.bmp");
    }

    SDL_Texture* spriteTexture = nullptr;
    if (surface) {
        spriteTexture = SDL_CreateTextureFromSurface(renderer, surface);
        SDL_FreeSurface(surface);
        if (spriteTexture) {
            SDL_SetTextureBlendMode(spriteTexture, SDL_BLENDMODE_BLEND);
        }
    }

    float posX = (SCREEN_WIDTH - SPRITE_SIZE) / 2.0f;
    float posY = (SCREEN_HEIGHT - SPRITE_SIZE) / 2.0f;

    Uint32 previousTicks = SDL_GetTicks();
    Uint32 lastPrintTicks = 0;
    float animTime = 0.0f;
    int frame = 0;

    const Vec2 north(0.0f, -1.0f);

    bool running = true;
    while (running) {
        SDL_Event event;
        while (SDL_PollEvent(&event)) {
            if (event.type == SDL_QUIT) {
                running = false;
            }
            if (event.type == SDL_KEYDOWN && event.key.keysym.sym == SDLK_ESCAPE) {
                running = false;
            }
        }

        Uint32 currentTicks = SDL_GetTicks();
        float dt = (currentTicks - previousTicks) / 1000.0f;
        previousTicks = currentTicks;
        if (dt > 0.1f) {
            dt = 0.1f;
        }

        const Uint8* state = SDL_GetKeyboardState(nullptr);
        float dirX = 0.0f;
        float dirY = 0.0f;

        if (state[SDL_SCANCODE_W]) dirY -= 1.0f;
        if (state[SDL_SCANCODE_S]) dirY += 1.0f;
        if (state[SDL_SCANCODE_A]) dirX -= 1.0f;
        if (state[SDL_SCANCODE_D]) dirX += 1.0f;

        bool ctrlPressed = (state[SDL_SCANCODE_LCTRL] != 0) || (state[SDL_SCANCODE_RCTRL] != 0);

        Vec2 vRaw(dirX, dirY);
        float mag = vRaw.length();
        Vec2 vNorm = vRaw.normalized();
        float dotNorth = vNorm.dot(north);

        Vec2 moveDir = ctrlPressed ? vRaw : vNorm;

        posX += moveDir.x * SPEED * dt;
        posY += moveDir.y * SPEED * dt;

        if (posX < 0.0f) posX = 0.0f;
        if (posX > static_cast<float>(SCREEN_WIDTH - SPRITE_SIZE)) posX = static_cast<float>(SCREEN_WIDTH - SPRITE_SIZE);
        if (posY < 0.0f) posY = 0.0f;
        if (posY > static_cast<float>(SCREEN_HEIGHT - SPRITE_SIZE)) posY = static_cast<float>(SCREEN_HEIGHT - SPRITE_SIZE);

        // Quieto: fila 0 de izquierda a derecha.
        // Derecha o vertical: fila 2, columnas 0 a 8.
        // Izquierda: la misma fila, columnas 8 a 0. No se voltea el dibujo.
        bool moving = (dirX != 0.0f) || (dirY != 0.0f);
        animTime += dt;
        while (animTime >= FRAME_TIME) {
            animTime -= FRAME_TIME;
            frame += 1;
            if (frame >= SHEET_COLS) {
                frame = 0;
            }
        }
        int row = moving ? 2 : 0;
        int col = frame;
        if (moving && dirX < 0.0f) {
            col = (SHEET_COLS - 1) - frame;
        }

        if (currentTicks - lastPrintTicks >= 100) {
            lastPrintTicks = currentTicks;
            std::printf("Mag: %6.4f | Norm: (%7.4f, %7.4f) | Dot Norte: %7.4f | Modo: %s\n",
                        mag,
                        vNorm.x,
                        vNorm.y,
                        dotNorth,
                        ctrlPressed ? "Sin normalizar (Ctrl)" : "Normalizado");
        }

        SDL_SetRenderDrawColor(renderer, 30, 30, 45, 255);
        SDL_RenderClear(renderer);

        SDL_Rect srcRect = {
            col * SPRITE_SIZE,
            row * SPRITE_SIZE,
            SPRITE_SIZE,
            SPRITE_SIZE
        };
        SDL_Rect dstRect = {
            static_cast<int>(posX),
            static_cast<int>(posY),
            SPRITE_SIZE,
            SPRITE_SIZE
        };

        if (spriteTexture) {
            SDL_RenderCopy(renderer, spriteTexture, &srcRect, &dstRect);
        } else {
            SDL_SetRenderDrawColor(renderer, 50, 150, 250, 255);
            SDL_RenderFillRect(renderer, &dstRect);
        }

        SDL_RenderPresent(renderer);
    }

    if (spriteTexture) {
        SDL_DestroyTexture(spriteTexture);
        spriteTexture = nullptr;
    }

    SDL_DestroyRenderer(renderer);
    SDL_DestroyWindow(window);
    SDL_Quit();

    return 0;
}
