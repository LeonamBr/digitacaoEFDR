cmake_minimum_required(VERSION 3.20)

# =========================
# Projeto
# =========================
project(curso_digitacao LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 23)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

# Saída organizada
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR})
set(CMAKE_ARCHIVE_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/lib)
set(CMAKE_LIBRARY_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/lib)

# Compila tudo como PIC (bom p/ libs estáticas)
set(CMAKE_POSITION_INDEPENDENT_CODE ON)

# Warnings
if (MSVC)
  add_compile_options(/W4 /permissive- /Zc:__cplusplus)
else()
  add_compile_options(-Wall -Wextra -Wpedantic -Wno-unused-parameter)
endif()

# Windows: evitar conflito com min/max
if (WIN32)
  add_definitions(-DNOMINMAX)
endif()

# =========================
# Sources do app
# =========================
# Ajuste o padrão conforme seu layout real:
# Ex.: src/app, src/ui, src/core, src/gfx etc.
file(GLOB_RECURSE APP_SOURCES
  "src/*.cpp"
)
file(GLOB_RECURSE APP_HEADERS
  "src/*.h"
)

add_executable(curso_digitacao ${APP_SOURCES} ${APP_HEADERS})

target_include_directories(curso_digitacao PRIVATE
  ${CMAKE_CURRENT_SOURCE_DIR}/src
)

# =========================
# SDL2 vendorizado
# =========================
# Esperado: external/SDL (submódulo oficial)
# Alvos comuns: SDL2::SDL2, SDL2::SDL2-static, SDL2-static, SDL2
set(BUILD_SHARED_LIBS OFF CACHE BOOL "" FORCE)
set(SDL_SHARED OFF CACHE BOOL "" FORCE)
set(SDL_STATIC ON CACHE BOOL "" FORCE)

add_subdirectory(external/SDL)

# Link “defensivo” (cobre diferentes nomes de target)
if (TARGET SDL2::SDL2-static)
  target_link_libraries(curso_digitacao PRIVATE SDL2::SDL2-static)
elseif (TARGET SDL2-static)
  target_link_libraries(curso_digitacao PRIVATE SDL2-static)
elseif (TARGET SDL2::SDL2)
  target_link_libraries(curso_digitacao PRIVATE SDL2::SDL2)
elseif (TARGET SDL2)
  target_link_libraries(curso_digitacao PRIVATE SDL2)
else()
  message(FATAL_ERROR "Não encontrei o target do SDL2. Verifique external/SDL.")
endif()

# ---- Também linke o SDL2main (fornece WinMain no Windows) ----
if (TARGET SDL2::SDL2main)
  target_link_libraries(curso_digitacao PRIVATE SDL2::SDL2main)
elseif (TARGET SDL2main)
  target_link_libraries(curso_digitacao PRIVATE SDL2main)
elseif (TARGET SDL2::SDL2-static) # às vezes o SDL2main vem junto; mas testamos alvos conhecidos acima
  # nada a fazer aqui se não houver um alvo SDL2main separado
else()
  message(WARNING "Nenhum target SDL2main encontrado. Se der 'undefined reference to WinMain', verifique external/SDL.")
endif()

# Inclui headers expostos pelo SDL2 (opcional, ajuda no IntelliSense)
if (TARGET SDL2::SDL2)
  get_target_property(_SDL_INC SDL2::SDL2 INTERFACE_INCLUDE_DIRECTORIES)
  if (_SDL_INC)
    target_include_directories(curso_digitacao PRIVATE ${_SDL_INC})
  endif()
endif()

# =========================
# SDL_ttf vendorizado
# =========================
# Esperado: external/SDL_ttf (submódulo oficial)
# Alvos comuns: SDL2_ttf::SDL2_ttf, SDL2_ttf::SDL2_ttf-static, SDL2_ttf
set(SDL2TTF_VENDORED ON CACHE BOOL "" FORCE)
add_subdirectory(external/SDL_ttf)

if (TARGET SDL2_ttf::SDL2_ttf)
  target_link_libraries(curso_digitacao PRIVATE SDL2_ttf::SDL2_ttf)
elseif (TARGET SDL2_ttf::SDL2_ttf-static)
  target_link_libraries(curso_digitacao PRIVATE SDL2_ttf::SDL2_ttf-static)
elseif (TARGET SDL2_ttf)
  target_link_libraries(curso_digitacao PRIVATE SDL2_ttf)
else()
  message(FATAL_ERROR "Não encontrei o target do SDL_ttf. Verifique external/SDL_ttf.")
endif()

# =========================
# SDL_image vendorizado
# =========================
# Esperado: external/SDL_image (submódulo oficial)
# Alvos comuns: SDL2_image::SDL2_image, SDL2_image::SDL2_image-static, SDL2_image
set(SDL2IMAGE_VENDORED ON CACHE BOOL "" FORCE)
set(SDL2IMAGE_DEPS_SHARED OFF CACHE BOOL "" FORCE)   # usa deps vendorizadas estáticas
set(SDL2IMAGE_SAMPLES OFF CACHE BOOL "" FORCE)
set(SDL2IMAGE_TESTS   OFF CACHE BOOL "" FORCE)

# Habilite apenas os formatos que você vai usar (acelera build)
set(SDL2IMAGE_AVIF  OFF CACHE BOOL "" FORCE)
set(SDL2IMAGE_BMP   ON  CACHE BOOL "" FORCE)
set(SDL2IMAGE_GIF   OFF CACHE BOOL "" FORCE)
set(SDL2IMAGE_JPG   ON  CACHE BOOL "" FORCE)
set(SDL2IMAGE_JXL   OFF CACHE BOOL "" FORCE)
set(SDL2IMAGE_LBM   OFF CACHE BOOL "" FORCE)
set(SDL2IMAGE_PCX   OFF CACHE BOOL "" FORCE)
set(SDL2IMAGE_PNG   ON  CACHE BOOL "" FORCE)
set(SDL2IMAGE_PNM   OFF CACHE BOOL "" FORCE)
set(SDL2IMAGE_QOI   OFF CACHE BOOL "" FORCE)
set(SDL2IMAGE_SVG   OFF CACHE BOOL "" FORCE)
set(SDL2IMAGE_TGA   OFF CACHE BOOL "" FORCE)
set(SDL2IMAGE_TIF   OFF CACHE BOOL "" FORCE)
set(SDL2IMAGE_WEBP  OFF CACHE BOOL "" FORCE)
set(SDL2IMAGE_XCF   OFF CACHE BOOL "" FORCE)
set(SDL2IMAGE_XPM   OFF CACHE BOOL "" FORCE)
set(SDL2IMAGE_XV    OFF CACHE BOOL "" FORCE)

add_subdirectory(external/SDL_image)

if (TARGET SDL2_image::SDL2_image)
  target_link_libraries(curso_digitacao PRIVATE SDL2_image::SDL2_image)
elseif (TARGET SDL2_image::SDL2_image-static)
  target_link_libraries(curso_digitacao PRIVATE SDL2_image::SDL2_image-static)
elseif (TARGET SDL2_image)
  target_link_libraries(curso_digitacao PRIVATE SDL2_image)
else()
  message(FATAL_ERROR "Não encontrei o target do SDL_image. Verifique external/SDL_image.")
endif()

# =========================
# Recursos (copiar após build)
# =========================
# Copia a pasta resources/ para a pasta de build (onde fica o executável)
add_custom_command(TARGET curso_digitacao POST_BUILD
  COMMAND ${CMAKE_COMMAND} -E copy_directory
          ${CMAKE_CURRENT_SOURCE_DIR}/resources
          ${CMAKE_RUNTIME_OUTPUT_DIRECTORY}/resources
  COMMENT "Copiando pasta resources/ para a pasta de execução..."
)

# =========================
# Definições/flags específicas (opcional)
# =========================
# Se precisar de uma macro p/ habilitar algo no código:
# target_compile_definitions(curso_digitacao PRIVATE USE_SDL_IMAGE)
#
# Em MinGW, às vezes precisa linkar explicitamente threads:
# if (MINGW)
#   target_link_libraries(curso_digitacao PRIVATE -static -static-libgcc -static-libstdc++)
# endif()
