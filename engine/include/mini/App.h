#pragma once
// Semanas 1-2: ventana y bucle de juego.
// En la semana 1 el estudiante escribe el bucle a mano en labs/s01_ventana.
// A partir de la semana 2 se traslada aqui, para reutilizarlo.
#include <SDL.h>

#include <string>

namespace mini {

class App {
public:
    App(const std::string& title, int width, int height);
    ~App();
    App(const App&) = delete;
    App& operator=(const App&) = delete;

    bool ok() const { return window_ != nullptr && renderer_ != nullptr; }
    SDL_Renderer* renderer() const { return renderer_; }
    int width() const { return width_; }
    int height() const { return height_; }

    // TODO (semana 2): agregar run() con paso de tiempo fijo y variable.

private:
    SDL_Window* window_ = nullptr;
    SDL_Renderer* renderer_ = nullptr;
    int width_;
    int height_;
};

}  // namespace mini
