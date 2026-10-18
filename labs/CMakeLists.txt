# Cada laboratorio es un ejecutable. Se agrega solo si tiene CMakeLists.txt,
# asi el docente puede ir publicando las practicas semana a semana.
file(GLOB lab_dirs LIST_DIRECTORIES true RELATIVE ${CMAKE_CURRENT_SOURCE_DIR} ${CMAKE_CURRENT_SOURCE_DIR}/s*)
foreach(d ${lab_dirs})
  if(EXISTS ${CMAKE_CURRENT_SOURCE_DIR}/${d}/CMakeLists.txt)
    add_subdirectory(${d})
  endif()
endforeach()
