#include "App.h"
#include <stdexcept>
#include <string>

// ---------- helper local: renderiza texto -> SDL_Texture* ----------
static SDL_Texture* RenderText(SDL_Renderer* r, TTF_Font* f,
                               const std::string& text, SDL_Color color)
{
    SDL_Surface* surf = TTF_RenderUTF8_Blended(f, text.c_str(), color);
    if (!surf) throw std::runtime_error(std::string("TTF_RenderUTF8_Blended: ") + TTF_GetError());
    SDL_Texture* tex = SDL_CreateTextureFromSurface(r, surf);
    SDL_FreeSurface(surf);
    if (!tex) throw std::runtime_error(std::string("SDL_CreateTextureFromSurface: ") + SDL_GetError());
    return tex;
}

App::App() = default;

App::~App() {
    Shutdown(); // garante teardown
}

bool App::Init(int width, int height) {
    if (SDL_Init(SDL_INIT_VIDEO) != 0) {
        SDL_Log("SDL_Init failed: %s", SDL_GetError());
        return false;
    }
    if (TTF_Init() != 0) {
        SDL_Log("TTF_Init failed: %s", TTF_GetError());
        return false;
    }

    m_window = SDL_CreateWindow("Curso de Digitacao",
                                SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED,
                                width, height, SDL_WINDOW_SHOWN);
    if (!m_window) {
        SDL_Log("CreateWindow failed: %s", SDL_GetError());
        return false;
    }

    m_renderer = SDL_CreateRenderer(m_window, -1,
        SDL_RENDERER_ACCELERATED | SDL_RENDERER_PRESENTVSYNC);
    if (!m_renderer) {
        SDL_Log("CreateRenderer failed: %s", SDL_GetError());
        return false;
    }

    // ajuste o caminho/arquivo conforme seu repo
    m_font = TTF_OpenFont("resources/fonts/RobotoMono-Regular.ttf", 24);
    if (!m_font) {
        SDL_Log("TTF_OpenFont failed: %s", TTF_GetError());
        return false;
    }
    return true;
}

void App::Shutdown() {
    if (m_font)     { TTF_CloseFont(m_font); m_font = nullptr; }
    if (m_renderer) { SDL_DestroyRenderer(m_renderer); m_renderer = nullptr; }
    if (m_window)   { SDL_DestroyWindow(m_window); m_window = nullptr; }
    if (TTF_WasInit()) TTF_Quit();
    if (SDL_WasInit(SDL_INIT_EVERYTHING)) SDL_Quit();
}

void App::Update(float /*dt*/) {
    // por enquanto nada; aqui vai game loop/lógica
}

void App::Render() {
    // fundo
    SDL_SetRenderDrawColor(m_renderer, m_bg.r, m_bg.g, m_bg.b, m_bg.a);
    SDL_RenderClear(m_renderer);

    // exemplo de HUD simples
    SDL_Texture* t1 = RenderText(m_renderer, m_font,
                                 "Alvo: " + (target.empty() ? "-" : target), m_fg);
    int w=0,h=0; SDL_QueryTexture(t1, nullptr, nullptr, &w, &h);
    SDL_Rect dst{20, 20, w, h};
    SDL_RenderCopy(m_renderer, t1, nullptr, &dst);
    SDL_DestroyTexture(t1);

    SDL_RenderPresent(m_renderer);
}
