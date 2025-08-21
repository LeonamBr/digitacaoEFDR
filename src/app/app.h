#pragma once
#include <string>
#include <SDL.h>
#include <SDL_ttf.h>

class App {
public:
    App();
    ~App();

    bool Init(int width, int height);
    void Shutdown();

    void Update(float dt);
    void Render();

private:
    SDL_Window*   m_window   = nullptr;
    SDL_Renderer* m_renderer = nullptr;
    TTF_Font*     m_font     = nullptr;

    SDL_Color m_fg{255,255,255,255};
    SDL_Color m_bg{ 32, 32, 32,255};

    std::string target = "asdf jkl;";
};
