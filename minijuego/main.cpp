#include <SDL.h>
#include <algorithm>
#include <cmath>
#include <cstdio>
#include <string>
#include <vector>

namespace {

constexpr int SCREEN_WIDTH = 480;
constexpr int SCREEN_HEIGHT = 720;
constexpr int MAP_WIDTH = 480;
constexpr int MAP_HEIGHT = 2048;

constexpr int SPRITE_SIZE = 64;
constexpr int SHEET_COLS = 9;
constexpr int BOTTLE_ROW = 2; // Fila 2: animacion de rodar/girar de la botella

// Constantes de fisica de la botella flotante
constexpr float UP_ACCEL = 420.0f;        // Empuje vertical hacia arriba
constexpr float DOWN_ACCEL = 220.0f;      // Empuje hacia abajo
constexpr float TORQUE = 450.0f;          // Torque angular al presionar izquierda/derecha
constexpr float LATERAL_FORCE = 160.0f;   // Impulso lateral al pivotar
constexpr float LINEAR_DAMPING = 2.4f;    // Friccion hidrodinamica lineal
constexpr float ROT_DAMPING = 2.6f;       // Amortiguacion hidrodinamica de rotacion
constexpr float MAX_SPEED = 320.0f;
constexpr float MAX_ANG_VEL = 380.0f;

// Intenta cargar un BMP desde multiples rutas relativas
SDL_Surface* loadSurfaceWithFallbacks(const std::string& filename) {
    const std::vector<std::string> paths = {
        filename,
        "assets/" + filename,
        "minijuego/assets/" + filename,
        "minijuego/" + filename,
        "../minijuego/assets/" + filename,
        "../../minijuego/assets/" + filename,
        "assets/levels/level01/" + filename
    };

    for (const auto& path : paths) {
        SDL_Surface* s = SDL_LoadBMP(path.c_str());
        if (s) {
            return s;
        }
    }
    return nullptr;
}

// Obtiene el color RGB de un pixel en una superficie BMP
void getPixelColor(SDL_Surface* surface, int x, int y, Uint8& r, Uint8& g, Uint8& b) {
    if (!surface || x < 0 || x >= surface->w || y < 0 || y >= surface->h) {
        r = 0; g = 0; b = 0;
        return;
    }

    const int bpp = surface->format->BytesPerPixel;
    const Uint8* p = static_cast<const Uint8*>(surface->pixels) + y * surface->pitch + x * bpp;

    Uint32 pixelVal = 0;
    if (bpp == 1) {
        pixelVal = *p;
    } else if (bpp == 2) {
        pixelVal = *reinterpret_cast<const Uint16*>(p);
    } else if (bpp == 3) {
        if (SDL_BYTEORDER == SDL_BIG_ENDIAN) {
            pixelVal = (p[0] << 16) | (p[1] << 8) | p[2];
        } else {
            pixelVal = p[0] | (p[1] << 8) | (p[2] << 16);
        }
    } else if (bpp == 4) {
        pixelVal = *reinterpret_cast<const Uint32*>(p);
    }

    SDL_GetRGB(pixelVal, surface->format, &r, &g, &b);
}

enum class GameState {
    Playing,
    Won,
    GameOver
};

} // namespace

int main(int, char**) {
    if (SDL_Init(SDL_INIT_VIDEO) != 0) {
        std::fprintf(stderr, "Error al inicializar SDL2: %s\n", SDL_GetError());
        return 1;
    }

    SDL_Window* window = SDL_CreateWindow(
        "La Gran Travesia de la Botella Flotante - Nivel 1",
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

    SDL_Renderer* renderer = SDL_CreateRenderer(
        window, -1, SDL_RENDERER_ACCELERATED | SDL_RENDERER_PRESENTVSYNC
    );
    if (!renderer) {
        renderer = SDL_CreateRenderer(window, -1, 0);
    }
    if (!renderer) {
        std::fprintf(stderr, "Error al crear renderer: %s\n", SDL_GetError());
        SDL_DestroyWindow(window);
        SDL_Quit();
        return 1;
    }

    // 1. Cargar Sprite Sheet de la Botella
    SDL_Texture* bottleTexture = nullptr;
    {
        SDL_Surface* surf = loadSurfaceWithFallbacks("player_sprite.bmp");
        if (surf) {
            bottleTexture = SDL_CreateTextureFromSurface(renderer, surf);
            SDL_FreeSurface(surf);
            if (bottleTexture) {
                SDL_SetTextureBlendMode(bottleTexture, SDL_BLENDMODE_BLEND);
            }
        }
    }

    // 2. Cargar los 4 fotogramas del Fondo de Agua Animada
    SDL_Texture* bgTextures[4] = {nullptr, nullptr, nullptr, nullptr};
    for (int i = 0; i < 4; ++i) {
        std::string fname = "bg_frame_" + std::to_string(i) + ".bmp";
        SDL_Surface* surf = loadSurfaceWithFallbacks(fname);
        if (!surf) {
            surf = loadSurfaceWithFallbacks("level01_rendered.bmp");
        }
        if (surf) {
            bgTextures[i] = SDL_CreateTextureFromSurface(renderer, surf);
            SDL_FreeSurface(surf);
        }
    }

    // 3. Cargar Mapa de Colisiones en memoria CPU
    SDL_Surface* collisionSurface = loadSurfaceWithFallbacks("level01_collision.bmp");

    // Estado del Jugador (Botella)
    const float START_X = (MAP_WIDTH - SPRITE_SIZE) / 2.0f;
    const float START_Y = 1940.0f;
    const float START_ANGLE = 90.0f; // Comienza horizontal en el agua

    float posX = START_X;
    float posY = START_Y;
    float velX = 0.0f;
    float velY = 0.0f;
    float angle = START_ANGLE;
    float angVel = 0.0f;
    float durability = 100.0f;
    float animTimer = 0.0f;
    int animFrame = 0;

    float cameraY = static_cast<float>(MAP_HEIGHT - SCREEN_HEIGHT);
    GameState state = GameState::Playing;

    Uint32 previousTicks = SDL_GetTicks();
    bool running = true;

    while (running) {
        SDL_Event event;
        while (SDL_PollEvent(&event)) {
            if (event.type == SDL_QUIT) {
                running = false;
            }
            if (event.type == SDL_KEYDOWN) {
                if (event.key.keysym.sym == SDLK_ESCAPE) {
                    running = false;
                }
                if (event.key.keysym.sym == SDLK_r) {
                    // Reiniciar partida
                    posX = START_X;
                    posY = START_Y;
                    velX = 0.0f;
                    velY = 0.0f;
                    angle = START_ANGLE;
                    angVel = 0.0f;
                    durability = 100.0f;
                    state = GameState::Playing;
                }
            }
        }

        Uint32 currentTicks = SDL_GetTicks();
        float dt = (currentTicks - previousTicks) / 1000.0f;
        previousTicks = currentTicks;
        if (dt > 0.05f) {
            dt = 0.05f; // Clampeo para evitar saltos en caidas de frames
        }

        const Uint8* keys = SDL_GetKeyboardState(nullptr);

        if (state == GameState::Playing) {
            // Controles de la botella:
            // W / Arriba: avanza hacia arriba por el canal fluvial
            if (keys[SDL_SCANCODE_W] || keys[SDL_SCANCODE_UP]) {
                velY -= UP_ACCEL * dt;
                // Leve impulso frontal acorde a la orientacion actual de la botella
                float rad = angle * 0.0174532925f;
                velX += std::sin(rad) * 60.0f * dt;
                velY -= std::cos(rad) * 60.0f * dt;
            }
            if (keys[SDL_SCANCODE_S] || keys[SDL_SCANCODE_DOWN]) {
                velY += DOWN_ACCEL * dt;
            }

            // D / Derecha: aplica torque desde la esquina inferior derecha
            if (keys[SDL_SCANCODE_D] || keys[SDL_SCANCODE_RIGHT]) {
                angVel += TORQUE * dt;
                velX += LATERAL_FORCE * dt;
            }
            // A / Izquierda: aplica torque en sentido antihorario
            if (keys[SDL_SCANCODE_A] || keys[SDL_SCANCODE_LEFT]) {
                angVel -= TORQUE * dt;
                velX -= LATERAL_FORCE * dt;
            }

            // Efecto de los Vórtices (Remolinos en Zona 4)
            const float wp1X = 136.0f, wp1Y = 848.0f;
            const float wp2X = 312.0f, wp2Y = 656.0f;
            for (const auto& [wx, wy] : {std::make_pair(wp1X, wp1Y), std::make_pair(wp2X, wp2Y)}) {
                float dx = wx - (posX + 32.0f);
                float dy = wy - (posY + 32.0f);
                float dist = std::sqrt(dx * dx + dy * dy);
                if (dist < 110.0f && dist > 1.0f) {
                    float pull = (110.0f - dist) / 110.0f;
                    velX += (dx / dist) * pull * 260.0f * dt;
                    velY += (dy / dist) * pull * 260.0f * dt;
                    angVel += pull * 480.0f * dt; // El remolino hace girar fuertemente a la botella
                }
            }

            // Friccion y resistencia hidrodinamica
            float linDamp = std::exp(-LINEAR_DAMPING * dt);
            velX *= linDamp;
            velY *= linDamp;

            float rotDamp = std::exp(-ROT_DAMPING * dt);
            angVel *= rotDamp;

            // Clampeo de velocidades maximas
            float currentSpeed = std::sqrt(velX * velX + velY * velY);
            if (currentSpeed > MAX_SPEED) {
                velX = (velX / currentSpeed) * MAX_SPEED;
                velY = (velY / currentSpeed) * MAX_SPEED;
            }
            if (angVel > MAX_ANG_VEL) angVel = MAX_ANG_VEL;
            if (angVel < -MAX_ANG_VEL) angVel = -MAX_ANG_VEL;

            // Integracion Euler
            posX += velX * dt;
            posY += velY * dt;
            angle += angVel * dt;

            while (angle >= 360.0f) angle -= 360.0f;
            while (angle < 0.0f) angle += 360.0f;

            // Limites del mapa
            if (posX < 0.0f) { posX = 0.0f; velX = -velX * 0.5f; }
            if (posX > MAP_WIDTH - SPRITE_SIZE) { posX = MAP_WIDTH - SPRITE_SIZE; velX = -velX * 0.5f; }
            if (posY > MAP_HEIGHT - SPRITE_SIZE) { posY = MAP_HEIGHT - SPRITE_SIZE; velY = -velY * 0.5f; }

            // Deteccion de colisiones en la mascara de colision
            if (collisionSurface) {
                float rad = angle * 0.0174532925f;
                float sinA = std::sin(rad);
                float cosA = std::cos(rad);
                float cx = posX + 32.0f;
                float cy = posY + 32.0f;

                // Muestreo a lo largo del cuerpo rectangular de la botella (longitud 64px, ancho 20px)
                const float offsets[5] = {-22.0f, -11.0f, 0.0f, 11.0f, 22.0f};
                for (float off : offsets) {
                    int sampleX = static_cast<int>(cx + sinA * off);
                    int sampleY = static_cast<int>(cy - cosA * off);

                    Uint8 r, g, b;
                    getPixelColor(collisionSurface, sampleX, sampleY, r, g, b);

                    // Rojo: Obstaculo / Peligro letal
                    if (r > 200 && g < 60 && b < 60) {
                        durability -= 25.0f * dt;
                        velX = -velX * 0.7f;
                        velY = -velY * 0.7f;
                        angVel = -angVel * 0.6f;
                        if (durability <= 0.0f) {
                            durability = 0.0f;
                            state = GameState::GameOver;
                        }
                    }
                    // Negro: Orilla / Muro infranqueable
                    else if (r < 40 && g < 40 && b < 40) {
                        velX = -velX * 0.6f;
                        velY = -velY * 0.6f;
                        angVel = -angVel * 0.5f;
                        posX -= velX * dt * 1.5f;
                        posY -= velY * dt * 1.5f;
                    }
                    // Dorado: Zona de Llegada / Meta
                    else if (r > 200 && g > 180 && b < 60) {
                        state = GameState::Won;
                    }
                }
            }

            // Condicion de victoria por posicion Y
            if (posY <= 64.0f) {
                state = GameState::Won;
            }

            // Animacion de giro / rodar del sprite de la botella
            float motionIntensity = std::abs(angVel) + std::abs(velX) + std::abs(velY);
            float frameRate = (motionIntensity > 60.0f) ? (1.0f / 12.0f) : (1.0f / 6.0f);
            animTimer += dt;
            if (animTimer >= frameRate) {
                animTimer -= frameRate;
                animFrame = (animFrame + 1) % SHEET_COLS;
            }
        }

        // Camara suave que sigue a la botella verticalmente
        float targetCameraY = posY - SCREEN_HEIGHT * 0.6f;
        if (targetCameraY < 0.0f) targetCameraY = 0.0f;
        if (targetCameraY > MAP_HEIGHT - SCREEN_HEIGHT) targetCameraY = static_cast<float>(MAP_HEIGHT - SCREEN_HEIGHT);
        cameraY += (targetCameraY - cameraY) * 6.0f * dt;

        // --- RENDERIZADO ---
        SDL_SetRenderDrawColor(renderer, 20, 24, 34, 255);
        SDL_RenderClear(renderer);

        // 1. Dibujar Fondo de Agua Animada (ciclo de 4 fotogramas cada 140 ms)
        int bgIndex = (currentTicks / 140) % 4;
        SDL_Texture* currentBg = bgTextures[bgIndex];
        if (currentBg) {
            SDL_Rect srcBg = {0, static_cast<int>(cameraY), SCREEN_WIDTH, SCREEN_HEIGHT};
            SDL_Rect dstBg = {0, 0, SCREEN_WIDTH, SCREEN_HEIGHT};
            SDL_RenderCopy(renderer, currentBg, &srcBg, &dstBg);
        }

        // 2. Dibujar la Botella Rotante
        if (state != GameState::GameOver && bottleTexture) {
            SDL_Rect srcRect = {animFrame * SPRITE_SIZE, BOTTLE_ROW * SPRITE_SIZE, SPRITE_SIZE, SPRITE_SIZE};
            SDL_Rect dstRect = {
                static_cast<int>(posX),
                static_cast<int>(posY - cameraY),
                SPRITE_SIZE,
                SPRITE_SIZE
            };
            // SDL_RenderCopyEx aplica rotacion sobre el centro
            SDL_RenderCopyEx(renderer, bottleTexture, &srcRect, &dstRect, angle, nullptr, SDL_FLIP_NONE);
        } else if (state == GameState::GameOver) {
            // Efecto de botella rota (dibujo rojo)
            SDL_Rect dstRect = {
                static_cast<int>(posX),
                static_cast<int>(posY - cameraY),
                SPRITE_SIZE,
                SPRITE_SIZE
            };
            SDL_SetRenderDrawColor(renderer, 220, 40, 40, 200);
            SDL_RenderFillRect(renderer, &dstRect);
        }

        // 3. HUD Superior (Progreso y Durabilidad)
        {
            // Barra de fondo del HUD
            SDL_Rect hudBg = {0, 0, SCREEN_WIDTH, 48};
            SDL_SetRenderDrawBlendMode(renderer, SDL_BLENDMODE_BLEND);
            SDL_SetRenderDrawColor(renderer, 15, 20, 30, 220);
            SDL_RenderFillRect(renderer, &hudBg);

            // Barra de progreso de la travesia (0% abajo, 100% en la meta)
            float progress = 1.0f - (posY / static_cast<float>(START_Y));
            if (progress < 0.0f) progress = 0.0f;
            if (progress > 1.0f) progress = 1.0f;

            SDL_Rect barBg = {16, 12, 200, 10};
            SDL_SetRenderDrawColor(renderer, 40, 50, 70, 255);
            SDL_RenderFillRect(renderer, &barBg);

            SDL_Rect barFill = {16, 12, static_cast<int>(200.0f * progress), 10};
            SDL_SetRenderDrawColor(renderer, 60, 200, 240, 255);
            SDL_RenderFillRect(renderer, &barFill);

            // Barra de durabilidad de la botella
            SDL_Rect durBg = {260, 12, 180, 10};
            SDL_SetRenderDrawColor(renderer, 60, 40, 40, 255);
            SDL_RenderFillRect(renderer, &durBg);

            float durNorm = durability / 100.0f;
            SDL_Rect durFill = {260, 12, static_cast<int>(180.0f * durNorm), 10};
            if (durNorm > 0.5f) {
                SDL_SetRenderDrawColor(renderer, 50, 220, 90, 255);
            } else if (durNorm > 0.25f) {
                SDL_SetRenderDrawColor(renderer, 240, 180, 40, 255);
            } else {
                SDL_SetRenderDrawColor(renderer, 240, 50, 50, 255);
            }
            SDL_RenderFillRect(renderer, &durFill);

            // Marcador de controles en la parte inferior
            SDL_Rect bottomHint = {0, SCREEN_HEIGHT - 28, SCREEN_WIDTH, 28};
            SDL_SetRenderDrawColor(renderer, 10, 15, 22, 210);
            SDL_RenderFillRect(renderer, &bottomHint);
        }

        // 4. Cartel de Victoria / Fin de Partida
        if (state == GameState::Won) {
            SDL_Rect banner = {40, SCREEN_HEIGHT / 2 - 80, SCREEN_WIDTH - 80, 120};
            SDL_SetRenderDrawColor(renderer, 20, 140, 70, 235);
            SDL_RenderFillRect(renderer, &banner);
            SDL_SetRenderDrawColor(renderer, 255, 220, 80, 255);
            SDL_RenderDrawRect(renderer, &banner);
        } else if (state == GameState::GameOver) {
            SDL_Rect banner = {40, SCREEN_HEIGHT / 2 - 80, SCREEN_WIDTH - 80, 120};
            SDL_SetRenderDrawColor(renderer, 150, 20, 20, 235);
            SDL_RenderFillRect(renderer, &banner);
            SDL_SetRenderDrawColor(renderer, 240, 80, 80, 255);
            SDL_RenderDrawRect(renderer, &banner);
        }

        SDL_RenderPresent(renderer);
    }

    // Limpieza de recursos
    if (collisionSurface) {
        SDL_FreeSurface(collisionSurface);
    }
    for (int i = 0; i < 4; ++i) {
        if (bgTextures[i]) {
            SDL_DestroyTexture(bgTextures[i]);
        }
    }
    if (bottleTexture) {
        SDL_DestroyTexture(bottleTexture);
    }

    SDL_DestroyRenderer(renderer);
    SDL_DestroyWindow(window);
    SDL_Quit();

    return 0;
}
