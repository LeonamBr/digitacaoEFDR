// main.cpp
#include <SDL.h>        // <— tem que vir antes para ativar o wrapper de SDL_main
#include "app/App.h"

int main(int argc, char* argv[]) {
    App app;
    if (!app.Init(1024, 600)) return 1;

    bool running = true;
    Uint64 last = SDL_GetPerformanceCounter();

    while (running) {
        SDL_Event e;
        while (SDL_PollEvent(&e)) {
            if (e.type == SDL_QUIT) running = false;
        }

        Uint64 now = SDL_GetPerformanceCounter();
        float dt = float(now - last) / float(SDL_GetPerformanceFrequency());
        last = now;

        app.Update(dt);
        app.Render();
    }
    return 0;
}
