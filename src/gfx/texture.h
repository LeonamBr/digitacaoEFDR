#ifndef TEXTURE_H
#define TEXTURE_H

#include <SDL.h>
#include <string>
#include <unordered_map>

// Se usar SDL_image, defina USE_SDL_IMAGE no CMake.
// Senão, o loader cai no SDL_LoadBMP (sem alpha/PNG).
// #define USE_SDL_IMAGE

struct TexInfo {
    SDL_Texture* tex = nullptr;
    int w = 0, h = 0;
};

class TextureCache {
public:
    explicit TextureCache(SDL_Renderer* r) : m_r(r) {}
    ~TextureCache() { Clear(); }

    // Carrega (ou retorna do cache) a textura. Suporta PNG se USE_SDL_IMAGE.
    TexInfo Get(const std::string& path);

    // Libera tudo
    void Clear();

private:
    SDL_Renderer* m_r = nullptr;
    std::unordered_map<std::string, TexInfo> m_map;
};

// Helpers de render
namespace TexDraw {

// Desenha cobrindo toda a área (cover), preservando aspecto
void Cover(SDL_Renderer* r, SDL_Texture* tex, int texW, int texH, int dstW, int dstH);

// Desenha "fit" centralizado (toda imagem visível, pode sobrar borda)
void Fit(SDL_Renderer* r, SDL_Texture* tex, int texW, int texH, int dstW, int dstH, int offsetY = 0);

// Desenha em (x,y) com escala
void At(SDL_Renderer* r, SDL_Texture* tex, int texW, int texH, int x, int y, float scale = 1.0f);

} // namespace TexDraw

#endif
