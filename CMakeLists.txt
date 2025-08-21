cmake_minimum_required(VERSION 3.22)
project(curso_digitacao LANGUAGES C CXX)

set(CMAKE_CXX_STANDARD 23)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

# =========================================
# SDL2 (estático "magrinho" para MinGW)
# =========================================
# desliga coisas que não precisamos
set(SDL_TEST OFF CACHE BOOL "" FORCE)
set(SDL_INSTALL_TESTS OFF CACHE BOOL "" FORCE)
set(SDL2_DISABLE_INSTALL ON CACHE BOOL "" FORCE)

# queremos SDL2 estático (mude para SHARED=ON se preferir DLL)
set(SDL_SHARED OFF CACHE BOOL "" FORCE)
set(SDL_STATIC ON  CACHE BOOL "" FORCE)

# corta subsistemas que só incham a build neste app
set(SDL_HIDAPI OFF CACHE BOOL "" FORCE)
set(SDL_JOYSTICK OFF CACHE BOOL "" FORCE)
set(SDL_HAPTIC  OFF CACHE BOOL "" FORCE)
set(SDL_AUDIO   OFF CACHE BOOL "" FORCE)
set(SDL_X11 OFF CACHE BOOL "" FORCE)
set(SDL_WAYLAND OFF CACHE BOOL "" FORCE)

add_subdirectory(external/SDL)

# =========================================
# SDL_ttf (com freetype vendorizado)
# =========================================
# o próprio SDL_ttf tem seu CMake; só diga o que quer
set(BUILD_SHARED_LIBS OFF CACHE BOOL "" FORCE)      # gerar lib estática para o TTF
set(SDL2TTF_VENDORED   ON  CACHE BOOL "" FORCE)     # usar freetype interno (mais simples)
add_subdirectory(external/SDL_ttf)

# =========================================
# EXECUTÁVEL DO APP
# =========================================
add_executable(curso_digitacao
    src/main.cpp
    src/app/App.cpp
    src/core/LessonEngine.cpp
    src/ui/KeyboardView.cpp
)
target_include_directories(curso_digitacao PRIVATE src)

# SDL2main fornece WinMain em Windows/MinGW
if (TARGET SDL2::SDL2main)
    target_link_libraries(curso_digitacao PRIVATE SDL2::SDL2main)
endif()

# ---- Link SDL2 (cobre estático/dinâmico) ----
if (TARGET SDL2::SDL2-static)
    message(STATUS "Linkando: SDL2::SDL2-static")
    target_link_libraries(curso_digitacao PRIVATE SDL2::SDL2-static)
elseif (TARGET SDL2::SDL2)
    message(STATUS "Linkando: SDL2::SDL2")
    target_link_libraries(curso_digitacao PRIVATE SDL2::SDL2)
else()
    message(FATAL_ERROR "Nenhum target SDL2 encontrado (verifique add_subdirectory(external/SDL)).")
endif()

# ---- Link SDL_ttf (cobre shared/static) ----
if (TARGET SDL2_ttf::SDL2_ttf)
    message(STATUS "Linkando: SDL2_ttf::SDL2_ttf")
    target_link_libraries(curso_digitacao PRIVATE SDL2_ttf::SDL2_ttf)
elseif (TARGET SDL2_ttf::SDL2_ttf-static)
    message(STATUS "Linkando: SDL2_ttf::SDL2_ttf-static")
    target_link_libraries(curso_digitacao PRIVATE SDL2_ttf::SDL2_ttf-static)
elseif (TARGET SDL2_ttf)
    message(STATUS "Linkando: SDL2_ttf (base)")
    target_link_libraries(curso_digitacao PRIVATE SDL2_ttf)
else()
    message(FATAL_ERROR "Nenhum target SDL2_ttf encontrado (verifique add_subdirectory(external/SDL_ttf)).")
endif()

# =========================================
# INCLUDES — garante que <SDL.h> e <SDL_ttf.h> sejam achados
# =========================================
# 1) herda includes dos targets (robusto)
foreach(dep SDL2::SDL2 SDL2::SDL2-static SDL2 SDL2_ttf::SDL2_ttf SDL2_ttf::SDL2_ttf-static SDL2_ttf)
    if (TARGET ${dep})
        get_target_property(_incs ${dep} INTERFACE_INCLUDE_DIRECTORIES)
        if (_incs)
            target_include_directories(curso_digitacao PRIVATE ${_incs})
        endif()
    endif()
endforeach()

# 2) fallbacks explícitos (cinto + suspensório)
#    - SDL gera headers em build/external/SDL/include/SDL2 (onde está SDL.h)
#    - SDL_ttf deixa SDL_ttf.h na raiz do submódulo durante o build
target_include_directories(curso_digitacao PRIVATE
    ${CMAKE_BINARY_DIR}/external/SDL/include
    ${CMAKE_BINARY_DIR}/external/SDL/include/SDL2
    ${CMAKE_SOURCE_DIR}/external/SDL_ttf
    ${CMAKE_BINARY_DIR}/external/SDL_ttf
)

# =========================================
# COPIAR ASSETS
# =========================================
add_custom_command(TARGET curso_digitacao POST_BUILD
    COMMAND ${CMAKE_COMMAND} -E copy_directory
            ${CMAKE_SOURCE_DIR}/resources $<TARGET_FILE_DIR:curso_digitacao>/resources)
