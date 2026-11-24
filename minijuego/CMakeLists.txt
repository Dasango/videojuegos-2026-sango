add_executable(minijuego
    main.cpp
)

target_link_libraries(minijuego PRIVATE course_sdl)
course_warnings(minijuego)

# Copiar assets y BMPs junto al ejecutable
add_custom_command(TARGET minijuego POST_BUILD
    COMMAND ${CMAKE_COMMAND} -E copy_directory
        "${CMAKE_CURRENT_SOURCE_DIR}/assets"
        "$<TARGET_FILE_DIR:minijuego>/assets"
)

add_custom_command(TARGET minijuego POST_BUILD
    COMMAND ${CMAKE_COMMAND} -E copy_if_different
        "${CMAKE_CURRENT_SOURCE_DIR}/player_sprite.bmp"
        "$<TARGET_FILE_DIR:minijuego>/player_sprite.bmp"
)

add_custom_command(TARGET minijuego POST_BUILD
    COMMAND ${CMAKE_COMMAND} -E copy_if_different
        "${CMAKE_CURRENT_SOURCE_DIR}/level01_collision.bmp"
        "$<TARGET_FILE_DIR:minijuego>/level01_collision.bmp"
)
