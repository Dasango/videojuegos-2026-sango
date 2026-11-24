# Minijuego: La Gran Travesía de la Botella Flotante (Sumativa 1)

Implementación en C++ con SDL2 (sin motor externo). 

## Cómo compilar y jugar

```powershell
# Compilar
cmake --preset debug
cmake --build --preset debug

# Ejecutar
./build/debug/minijuego/minijuego.exe
```

## Controles
- `[W]` o `[↑]`: Impulso hacia arriba por el río.
- `[A]` / `[D]` o `[←]` / `[→]`: Rotar la botella a izquierda y derecha con inercia hidrodinámica.
- `[S]` o `[↓]`: Frenar / descender.
- `[R]`: Reiniciar la partida.
- `[ESC]`: Salir.
