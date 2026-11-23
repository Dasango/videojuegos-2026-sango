# Nivel 1: La Gran Travesía Fluvial (Mobile Vertical Scroller)

Escenario vertical diseñado para videojuego móvil con vista top-down (estilo retro 16-bit).

## Especificaciones Técnicas

- **Dimensiones de Grilla**: 20 columnas x 128 filas (tiles de 16x16 píxeles).
- **Resolución Renderizada**: 320 x 2048 píxeles (Aspect ratio 5:32 vertical, óptimo para scroll vertical continuo en smartphones).
- **Punto de Inicio (Spawn)**:
  - Coordenadas: `X = 152, Y = 2004` (Fila 125, Columna 9).
  - Personaje: Capitán con impermeable amarillo.
- **Punto de Llegada (Finish Goal)**:
  - Coordenadas: `Y <= 64 px` (Filas 0 a 3, Columnas 3 a 16).
  - Zona de activación: Muelle del Santuario Dorado.

## Archivos Generados

1. **`level01_rendered.png`** (320 x 2048): Renderizado final completo en un solo archivo PNG con fondo, orillas, obstáculos y personaje inicial integrados.
2. **`level01_preview.png`** (640 x 4096): Versión escalada 2x con muestreo Nearest-Neighbor para visualización nítida pixel-art.
3. **`level01_background.png`** (320 x 2048): Capa base del terreno (aguas, orillas de piedra/ladrillo/nieve y muelles de madera).
4. **`level01_obstacles.png`** (320 x 2048): Capa transparente con los 91 obstáculos colocados exactamente en sus posiciones.
5. **`level01_collision.png`** (320 x 2048): Máscara de colisión por código de color:
   - **Blanco `(255, 255, 255)`**: Agua navegable libre.
   - **Negro `(0, 0, 0)`**: Orillas y muros infranqueables.
   - **Rojo `(255, 0, 0)`**: Obstáculos letales (rocas, tuberías, vórtices, icebergs, minas).
   - **Azul `(0, 120, 255)`**: Zona de salida / Muelle de inicio.
   - **Dorado `(255, 215, 0)`**: Zona de meta / Santuario de victoria.
6. **`level01_data.json`**: Metadatos estructurados para carga directa en C++/SDL2 (dimensiones, zonas, spawn, meta, lista de 91 obstáculos con cajas de colisión AABB `[x, y, w, h]`).

## Estructura de Zonas (De Abajo hacia Arriba)

| Zona | Nombre | Filas (Tiles) | Rango Y (px) | Descripción & Obstáculos Clave |
|---|---|---|---|---|
| **1** | **Puerto de Salida (Harbor Basin)** | 112 - 127 | 1792 - 2047 | Dársena de madera y aguas calmas. Introducción de controles. Barco de papel, boyas y balsa flotante. |
| **2** | **Bancos Rocosos y Humedal** | 88 - 111 | 1408 - 1791 | Aguas sinuosas con juncos y nenúfares. Puertas de rocas y arrecife central que bifurca el río en dos caminos. |
| **3** | **Canal Industrial y Tuberías Rojas** | 60 - 87 | 960 - 1407 | Muros de ladrillo y hormigón. Barreras de tuberías pesadas con manómetros, vertederos de agua con corriente y boyas con púas. |
| **4** | **El Abismo de Vórtices y Pecios** | 36 - 59 | 576 - 959 | Rápidos violentos. Dos grandes remolinos (Vórtices Alfa y Beta) que absorben al jugador, redes de pesca y restos de naufragios. |
| **5** | **Garganta Glacial e Icebergs** | 12 - 35 | 192 - 575 | Aguas gélidas turquesas con costas nevadas. Témpanos flotantes, agujas de hielo y la gran puerta de icebergs gemelos. |
| **6** | **El Santuario Dorado (Finish Haven)** | 0 - 11 | 0 - 191 | Llegada triunfal a aguas calmas iluminadas con faroles dorados y el muelle de victoria. |
