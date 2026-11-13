cmake_minimum_required(VERSION 3.21)
project(Proyecto_Final CXX)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

# las librerias vienen de Librerias.zip y no se suben a git
set(LIBRERIAS_DIR ${CMAKE_SOURCE_DIR}/Librerias)
if(NOT EXISTS ${LIBRERIAS_DIR}/include/glew.h OR NOT EXISTS ${LIBRERIAS_DIR}/lib/glew32.lib)
    message(FATAL_ERROR "Falta la carpeta Librerias. Extrae Librerias.zip dentro de la carpeta del proyecto. Ver README.md")
endif()

# todos los .cpp de src/ entran solos, no hay que listarlos
file(GLOB FUENTES CONFIGURE_DEPENDS ${CMAKE_SOURCE_DIR}/src/*.cpp)
add_executable(Proyecto_Final ${FUENTES})

target_include_directories(Proyecto_Final PRIVATE
    ${CMAKE_SOURCE_DIR}/src
    ${LIBRERIAS_DIR}/include
    ${LIBRERIAS_DIR}/glm)

target_link_directories(Proyecto_Final PRIVATE ${LIBRERIAS_DIR}/lib)
target_link_libraries(Proyecto_Final PRIVATE glew32 glfw3 opengl32 assimp-vc143-mt)

target_compile_definitions(Proyecto_Final PRIVATE _CONSOLE UNICODE _UNICODE)
target_compile_options(Proyecto_Final PRIVATE /W3 /permissive- /sdl)

# en Release se optimiza todo el programa junto, igual que en el .vcxproj
set_property(TARGET Proyecto_Final PROPERTY INTERPROCEDURAL_OPTIMIZATION_RELEASE ON)

# las DLL van junto al .exe
file(GLOB DLLS ${LIBRERIAS_DIR}/dll/*.dll)
add_custom_command(TARGET Proyecto_Final POST_BUILD
    COMMAND ${CMAKE_COMMAND} -E copy_if_different ${DLLS} $<TARGET_FILE_DIR:Proyecto_Final>)
