add_executable(s06_sumativa_i
  Game/main.cpp
  Game/Mapa.cpp
  Game/Obstaculos.cpp
  Game/Personaje.cpp
  Game/Burbuja.cpp
  Game/OrdenUI.cpp
  Game/UI.cpp
  Game/Juego.cpp
)
target_include_directories(s06_sumativa_i PRIVATE ${CMAKE_CURRENT_SOURCE_DIR})
target_link_libraries(s06_sumativa_i PRIVATE course_sdl)
course_warnings(s06_sumativa_i)

add_custom_command(TARGET s06_sumativa_i POST_BUILD
  COMMAND ${CMAKE_COMMAND} -E copy_directory
    "${CMAKE_CURRENT_SOURCE_DIR}/assets"
    "$<TARGET_FILE_DIR:s06_sumativa_i>/assets"
)
