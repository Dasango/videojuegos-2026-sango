cmake_minimum_required(VERSION 3.21)
project(videojuegos2026 LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

# ---------------------------------------------------------------
# SDL2: 1) si esta instalado (vcpkg, apt, brew) se usa.
#       2) si no, se descarga y compila automaticamente.
# ---------------------------------------------------------------
find_package(SDL2 CONFIG QUIET)
if(NOT SDL2_FOUND)
  message(STATUS "SDL2 no encontrado: descargando con FetchContent...")
  include(FetchContent)
  set(SDL_SHARED OFF CACHE BOOL "" FORCE)
  set(SDL_STATIC ON CACHE BOOL "" FORCE)
  set(SDL_TEST OFF CACHE BOOL "" FORCE)
  FetchContent_Declare(SDL2
    GIT_REPOSITORY https://github.com/libsdl-org/SDL.git
    GIT_TAG release-2.30.9
    GIT_SHALLOW TRUE)
  FetchContent_MakeAvailable(SDL2)
endif()

# Alias uniforme para todos los proyectos del curso.
# IMPORTANTE: SDL2main debe ir ANTES que SDL2 (MinGW es sensible al orden).
add_library(course_sdl INTERFACE)
if(TARGET SDL2::SDL2main)
  target_link_libraries(course_sdl INTERFACE SDL2::SDL2main)
elseif(TARGET SDL2main)
  target_link_libraries(course_sdl INTERFACE SDL2main)
endif()
if(TARGET SDL2::SDL2)
  target_link_libraries(course_sdl INTERFACE SDL2::SDL2)
else()
  target_link_libraries(course_sdl INTERFACE SDL2-static)
endif()

# Advertencias comunes
function(course_warnings target)
  if(MSVC)
    target_compile_options(${target} PRIVATE /W4)
  else()
    target_compile_options(${target} PRIVATE -Wall -Wextra -Wpedantic)
  endif()
endfunction()

add_subdirectory(engine)
add_subdirectory(labs)
# Se activan cuando existan sus CMakeLists.txt
foreach(dir minijuego bench tests)
  if(EXISTS ${CMAKE_SOURCE_DIR}/${dir}/CMakeLists.txt)
    add_subdirectory(${dir})
  endif()
endforeach()
