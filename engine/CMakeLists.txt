add_library(mini STATIC
  src/App.cpp
)
target_include_directories(mini PUBLIC include)
target_link_libraries(mini PUBLIC course_sdl)
course_warnings(mini)
