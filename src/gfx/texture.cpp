#include "gfx/Texture.h"
#include <algorithm>

#ifdef USE_SDL_IMAGE
#include <SDL_image.h>
#endif

static TexInfo MakeTex(SDL_Renderer* r, SDL_Surface* s) {
    TexInfo ti;
    if (!s) return ti;
    ti.tex = SDL_CreateTextureFromSurface(r, s);
    ti.w = s->w; ti.h = s->h;
    SDL_FreeSurface(s);
    return ti;
}

TexInfo TextureCache::Get(const std::string& path) {
    if (auto it = m_map.find(path); it != m_map.end()) return it->second;

    TexInfo ti;
#ifdef USE_SDL_IMAGE
    SDL_Surface* surf = IMG_Load(path.c_str());
    if (!surf) {
        // fallback: tenta BMP
        surf = SDL_LoadBMP(path.c_str());
    }
#else
    SDL_Surface* surf = SDL_LoadBMP(path.c_str());
#endif
    ti = MakeTex(m_r, surf);
    m_map[path] = ti;
    return ti;
}

void TextureCache::Clear() {
    for (auto& [k, v] : m_map) {
        if (v.tex) SDL_DestroyTexture(v.tex);
    }
    m_map.clear();
}

// ----------------- Draw helpers -----------------
void TexDraw::Cover(SDL_Renderer* r, SDL_Texture* tex, int tw, int th, int dw, int dh) {
    if (!tex || tw==0 || th==0 || dw==0 || dh==0) return;
    const float s = std::max(dw / float(tw), dh / float(th));
    const int w = int(tw * s);
    const int h = int(th * s);
    const int x = (dw - w) / 2;
    const int y = (dh - h) / 2;
    SDL_Rect dst{ x, y, w, h };
    SDL_RenderCopy(r, tex, nullptr, &dst);
}

void TexDraw::Fit(SDL_Renderer* r, SDL_Texture* tex, int tw, int th, int dw, int dh, int offsetY) {
    if (!tex || tw==0 || th==0 || dw==0 || dh==0) return;
    const float s = std::min(dw / float(tw), dh / float(th));
    const int w = int(tw * s);
    const int h = int(th * s);
    const int x = (dw - w) / 2;
    const int y = (dh - h) / 2 + offsetY;
    SDL_Rect dst{ x, y, w, h };
    SDL_RenderCopy(r, tex, nullptr, &dst);
}

void TexDraw::At(SDL_Renderer* r, SDL_Texture* tex, int tw, int th, int x, int y, float scale) {
    if (!tex || tw==0 || th==0) return;
    const int w = int(tw * scale);
    const int h = int(th * scale);
    SDL_Rect dst{ x, y, w, h };
    SDL_RenderCopy(r, tex, nullptr, &dst);
}
