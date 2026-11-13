add_executable(s05_transformaciones_2d main.cpp)
target_link_libraries(s05_transformaciones_2d PRIVATE course_sdl)
course_warnings(s05_transformaciones_2d)

add_custom_command(TARGET s05_transformaciones_2d POST_BUILD
  COMMAND ${CMAKE_COMMAND} -E copy_if_different
    "${CMAKE_CURRENT_SOURCE_DIR}/player_sprite.bmp"
    "$<TARGET_FILE_DIR:s05_transformaciones_2d>/player_sprite.bmp"
)
