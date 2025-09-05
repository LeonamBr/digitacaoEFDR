#include <SDL_main.h>
#include "app/App.h"

int SDL_main(int, char**) {
    App app;
    if (!app.Init()) return 1;
    app.Run();
    app.Shutdown();
    return 0;
}
