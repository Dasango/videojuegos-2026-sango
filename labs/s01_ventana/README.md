# Semana 1 - Ventana y bucle minimo

**Objetivo:** tener el entorno funcionando y entender las etapas del bucle.

## Compilar y ejecutar
```
cmake --preset debug
cmake --build --preset debug
./build/debug/labs/s01_ventana/s01_ventana      # Windows: .\build\debug\labs\s01_ventana\s01_ventana.exe
```

## Ejercicios
1. Dibuja un rectangulo en el centro de la ventana.
2. Haz que el color de fondo cambie con el tiempo (`SDL_GetTicks`).
3. Imprime en consola cuantas veces por segundo se ejecuta el bucle. Que pasa si desactivas vsync?

## Preguntas
- Que pasa si no llamas a `SDL_PollEvent`?
- Por que el bucle no debe tener `sleep` arbitrarios?
