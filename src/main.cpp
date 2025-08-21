#include <SDL.h>
#include "app/App.h"

int SDL_main(int /*argc*/, char** /*argv*/) {
    App app;

    // Aceita tanto Init(w,h) quanto Init() (automático pelo tamanho da tela)
    if (!app.Init(1024, 600)) {
        if (!app.Init()) return 1;
    }

    Uint64 last = SDL_GetPerformanceCounter();
    const double freq = (double)SDL_GetPerformanceFrequency();

    while (app.Running()) {
        SDL_Event e;
        while (SDL_PollEvent(&e)) {
            app.HandleEvent(e);
        }

        Uint64 now = SDL_GetPerformanceCounter();
        float dt = float((now - last) / freq);
        last = now;

        app.Update(dt);
        app.Render();
    }

    app.Shutdown();
    return 0;
}
