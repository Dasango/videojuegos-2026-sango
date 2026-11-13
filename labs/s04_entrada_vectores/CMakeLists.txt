add_executable(s04_entrada_vectores main.cpp)
target_link_libraries(s04_entrada_vectores PRIVATE course_sdl)
course_warnings(s04_entrada_vectores)

add_custom_command(TARGET s04_entrada_vectores POST_BUILD
  COMMAND ${CMAKE_COMMAND} -E copy_if_different
    "${CMAKE_CURRENT_SOURCE_DIR}/player_sprite.bmp"
    "$<TARGET_FILE_DIR:s04_entrada_vectores>/player_sprite.bmp"
)
