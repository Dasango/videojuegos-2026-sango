# Guia rapida: de Java a C++17

| Tema | Java | C++ (en este curso) |
|---|---|---|
| Objetos | Siempre referencias en el heap | Por valor en el stack por defecto |
| Memoria | Recolector de basura | RAII: el destructor libera. Prohibido `new`/`delete`; usar `std::vector`, `std::unique_ptr` |
| Referencias | Toda variable de objeto es referencia | `T&` referencia, `const T&` para pasar sin copiar |
| Copia | `a = b` copia la referencia | `a = b` copia el objeto completo |
| Archivos | Un `.java` por clase | `.h` (declaracion) + `.cpp` (definicion) |
| Compilacion | `javac` / JVM | Compilar + enlazar (errores de *linker*) |
| Cadenas | `String` | `std::string` |
| Arreglos | `ArrayList<T>` | `std::vector<T>` |
| Polimorfismo | Metodos virtuales por defecto | Solo con `virtual` |
| Errores | Excepciones y NPE | Comportamiento indefinido: usar sanitizers (`cmake --preset asan`) |

## Reglas del curso
1. Nada de `new`/`delete` ni aritmetica de punteros.
2. Pasar objetos grandes por `const&`.
3. Compilar sin advertencias.
4. Probar con el preset `asan` antes de entregar.
