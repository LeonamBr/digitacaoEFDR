# Typing Tutor SDL2 — Starter (C++23)

Projeto inicial de tutor de digitação com **SDL2 + SDL2_ttf**:
- Captura `SDL_TEXTINPUT` (UTF‑8) e destaca o próximo caractere da lição.
- Estatísticas básicas: acurácia, CPM, tempo.
- Layout visual simples de teclado (pode ser refinado para ABNT2 completo).

## Como rodar

### Linux (Debian/Ubuntu)
```bash
sudo apt install libsdl2-dev libsdl2-ttf-dev
cmake -S . -B build
cmake --build build -j
./build/typing_tutor
```

### Windows (vcpkg)
```powershell
vcpkg install sdl2 sdl2-ttf
cmake -S . -B build -DCMAKE_TOOLCHAIN_FILE=[path]\vcpkg.cmake
cmake --build build -j
```

### macOS (Homebrew)
```bash
brew install sdl2 sdl2_ttf
cmake -S . -B build
cmake --build build -j
./build/typing_tutor
```

### Fonte
Coloque uma fonte `.ttf` como `assets/DejaVuSans.ttf` (ou edite no código).
