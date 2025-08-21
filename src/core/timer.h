#ifndef TIMER_H
#define TIMER_H

#include <SDL.h>

struct Timer {
    uint64_t start = 0;
    uint64_t stop = 0;
    bool running = false;

    void Start() { running = true; start = SDL_GetPerformanceCounter(); }
    void Stop() { running = false; stop = SDL_GetPerformanceCounter(); }

    double ElapsedSeconds() const {
        const double freq = (double)SDL_GetPerformanceFrequency();
        uint64_t end = running ? SDL_GetPerformanceCounter() : stop;
        return (end - start) / freq;
    }
};

#endif