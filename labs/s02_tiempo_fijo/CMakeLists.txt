add_executable(s02_tiempo_fijo main.cpp)
target_link_libraries(s02_tiempo_fijo PRIVATE course_sdl)
course_warnings(s02_tiempo_fijo)

# Copiar sprite.bmp al directorio del ejecutable
add_custom_command(TARGET s02_tiempo_fijo POST_BUILD
  COMMAND ${CMAKE_COMMAND} -E copy_if_different
    "${CMAKE_CURRENT_SOURCE_DIR}/sprite.bmp"
    "$<TARGET_FILE_DIR:s02_tiempo_fijo>/sprite.bmp"
)
