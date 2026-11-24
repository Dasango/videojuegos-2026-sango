# Nivel 1: La Gran Travesía de la Botella Flotante (Minijuego C++ / SDL2)

Videojuego retro top-down para móvil diseñado con física de botella flotante rotante y fondo acuático animado.

## Especificaciones del Escenario

- **Dimensiones de Grilla**: 30 columnas x 128 filas (tiles de 16x16 píxeles).
- **Resolución Renderizada**: 480 x 2048 píxeles (canal navegable de 384 px de ancho libre).
- **Área de Viraje Garantizada**: Separación mínima entre obstáculos de 128 a 192 px, permitiendo a la botella (64 px de longitud) rotar 360° en cualquier tramo sin quedar atascada.
- **Fondo Animado**: 4 fotogramas continuos (`bg_frame_0.bmp` a `bg_frame_3.bmp` y `level01_anim_sheet.bmp`) que generan oleaje, corrientes fluviales, vórtices y vertederos en movimiento constante.

## Física y Mecánicas de la Botella

- **Posición Inicial**: Muelle inferior (`X = 208, Y = 1940`), orientada horizontalmente (`ángulo = 90°`).
- **Controles**:
  - `[W]` o `[↑]`: Impulso hacia arriba y avance frontal en la dirección orientada.
  - `[D]` o `[→]`: Aplica torque horario desde la esquina inferior derecha, rotando la botella suavemente con inercia.
  - `[A]` o `[←]`: Aplica torque antihorario suave con inercia hidrodinámica.
  - `[S]` o `[↓]`: Freno y retroceso suave.
  - `[R]`: Reiniciar la partida.
  - `[ESC]`: Salir del juego.
- **Animación del Sprite**:
  - Utiliza `player_sprite.bmp` (576 x 384 px, celdas de 64x64 px).
  - Reproduce en bucle la animación de rodado cilíndrico de la Fila 2 (columnas 0 a 8) conforme la botella flota, gira y avanza.
- **Interacciones de Entorno**:
  - **Orillas / Muros**: Rebote elástico hidrodinámico.
  - **Obstáculos (Rocas, Tuberías, Icebergs, Minas)**: Impacto con pérdida de durabilidad (-25% por colisión sostenida).
  - **Remolinos (Vórtices en Zona 4)**: Fuerza de atracción centrípeta que absorbe y hace rotar a la botella.
  - **Santuario Dorado (`Y <= 64`)**: Llegada victoriosa y pantalla de nivel completado.

## Compilación y Ejecución

El proyecto está integrado en CMake:

```powershell
# Configurar y compilar en modo Debug
cmake --preset debug
cmake --build --preset debug

# Ejecutar el minijuego
./build/debug/minijuego/minijuego.exe
```

También disponible en modo Release:
```powershell
cmake --preset release
cmake --build --preset release
./build/release/minijuego/minijuego.exe
```
