# Fundamentos para Desarrollo de Videojuegos 2026-2027

Semanas 1-9: C++17 + SDL2 (sin motor). Semanas 10-16: Godot 4 + GDScript.

## Requisitos
- Compilador C++17 (GCC, Clang o MSVC), CMake >= 3.21, Ninja, Git.
- Linux: `sudo apt install build-essential cmake ninja-build libsdl2-dev`
- macOS: `brew install cmake ninja sdl2`
- Windows: Visual Studio (C++) o MSYS2; SDL2 se descarga automaticamente.
  (Opcional: vcpkg, ver preset `vcpkg`.)

## Compilar
```
cmake --preset debug
cmake --build --preset debug
```
Presets: `debug`, `release` (para mediciones), `asan` (sanitizers), `vcpkg`.

## Estructura
- `engine/` mini-biblioteca que crece cada semana
- `labs/` practicas semanales (una carpeta cada una)
- `minijuego/` Sumativa 1
- `bench/` mediciones de la semana 8
- `godot/` segunda mitad del curso
- `docs/` guias y plantillas
