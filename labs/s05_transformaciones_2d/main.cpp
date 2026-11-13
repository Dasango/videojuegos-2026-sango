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

    float length() const { return std::sqrt(x * x + y * y); }

    Vec2 normalized() const {
        float len = length();
        if (len > 0.0001f) {
            return {x / len, y / len};
        }
        return {0.0f, 0.0f};
    }

    float dot(const Vec2& o) const { return x * o.x + y * o.y; }
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
    // Giro continuo y cambio de escala, ambos con el tiempo dt.
    const float ROT_SPEED = 180.0f;
    const float SCALE_SPEED = 1.0f;
    const float SCALE_MIN = 0.3f;
    const float SCALE_MAX = 3.0f;

    SDL_Window* window = SDL_CreateWindow(
        "Lab 4: Transformaciones 2D",
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
        surface = SDL_LoadBMP("labs/s05_transformaciones_2d/player_sprite.bmp");
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
    float scale = 1.0f;
    float angle = 0.0f;
    bool rotating = false;

    Uint32 previousTicks = SDL_GetTicks();
    float animTime = 0.0f;
    int frame = 0;

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

        // Siempre normalizado: la diagonal tambien va a 250 px/s.
        Vec2 moveDir = Vec2(dirX, dirY).normalized();
        Vec2 delta = moveDir * (SPEED * dt);
        posX += delta.x;
        posY += delta.y;

        // J activa el giro. K lo detiene y pone el angulo en 0 al instante.
        if (state[SDL_SCANCODE_J]) {
            rotating = true;
        }
        if (rotating) {
            angle += ROT_SPEED * dt;
            if (angle >= 360.0f) {
                angle -= 360.0f;
            }
        }
        if (state[SDL_SCANCODE_K]) {
            rotating = false;
            angle = 0.0f;
        }

        // Mantener L agranda. Mantener I achica. Limite entre 0.3 y 3.0.
        if (state[SDL_SCANCODE_L]) {
            scale += SCALE_SPEED * dt;
        }
        if (state[SDL_SCANCODE_I]) {
            scale -= SCALE_SPEED * dt;
        }
        if (scale < SCALE_MIN) scale = SCALE_MIN;
        if (scale > SCALE_MAX) scale = SCALE_MAX;

        // Ancho y alto dibujados = 64 * escala. El borde usa ese tamano.
        // Despues de cambiar la escala se acomoda de nuevo para no salirse.
        int drawW = static_cast<int>(static_cast<float>(SPRITE_SIZE) * scale);
        int drawH = drawW;
        float maxX = static_cast<float>(SCREEN_WIDTH - drawW);
        float maxY = static_cast<float>(SCREEN_HEIGHT - drawH);
        if (posX < 0.0f) posX = 0.0f;
        if (posX > maxX) posX = maxX;
        if (posY < 0.0f) posY = 0.0f;
        if (posY > maxY) posY = maxY;

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

        SDL_SetRenderDrawColor(renderer, 30, 30, 45, 255);
        SDL_RenderClear(renderer);

        SDL_Rect srcRect = {col * SPRITE_SIZE, row * SPRITE_SIZE, SPRITE_SIZE, SPRITE_SIZE};
        SDL_Rect dstRect = {static_cast<int>(posX), static_cast<int>(posY), drawW, drawH};

        if (spriteTexture) {
            // center en NULL: gira sobre el centro del rectangulo destino.
            // SDL_RenderCopyEx aplica escala, rotacion y traslacion (T·R·S) al dibujar.
            SDL_RenderCopyEx(renderer, spriteTexture, &srcRect, &dstRect, angle, nullptr,
                             SDL_FLIP_NONE);
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
