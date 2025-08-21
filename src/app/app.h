#ifndef APP_H
#define APP_H

#include <SDL.h>
#include <SDL_ttf.h>
#include <string>
#include <vector>
#include <cstdint>
#include "../core/LessonEngine.h"
#include "../ui/KeyboardView.h"

namespace Lessons {
    struct Lesson {
        std::string title;
        std::string text;   // sequência-alvo (UTF-8)
    };

    // Conteúdo inicial simples; você pode trocar/expandir à vontade.
    inline const std::vector<Lesson>& All() {
        static const std::vector<Lesson> k = {
            {"Home row (mão esquerda)",  "asdf jklç"},
            {"Acentos & ç",              "a á à ã â ç A Á À Ã Â Ç"},
            {"Números",                  "123 456 789 0"},
            {"Frase curta",              "Hoje é dia de treinar digitação!"}
        };
        return k;
    }
}

class App {
public:
    App() = default;

    // Dois inits: sem args (auto pela tela) e com (w,h) — o main pode chamar qualquer um.
    bool Init();
    bool Init(int w, int h);

    void Shutdown();

    void HandleEvent(const SDL_Event& e);
    void Update(float dt);
    void Render();

    bool Running() const { return m_running; }

private:
    // janela/render
    SDL_Window*   m_window   = nullptr;
    SDL_Renderer* m_renderer = nullptr;

    // TTF
    TTF_Font*     m_font     = nullptr;
    std::string   m_fontPath;
    bool          m_textInputOn = false;

    // estado do app
    bool   m_running     = true;
    bool   m_fullscreen  = false;
    bool   m_showSummary = false;

    // tempo e métricas
    double m_nowSeconds  = 0.0;  // tempo absoluto do app (para animações)
    int    m_elapsedMs   = 0;    // cronômetro da lição em ms (para HUD)
    // Lições
    size_t m_lessonIndex = 0;

    // componentes
    LessonEngine m_engine;
    KeyboardView m_kb;

private:
    bool InitInternal(int reqW, int reqH); // usado pelos dois Init()
    void StartLesson(size_t index);
    void ReloadFont(); // recarrega RobotoMono conforme altura do render
    void ToggleFullscreen();

    // helpers de desenho
    void drawText(const std::string& s, int x, int y, SDL_Color c = {230,230,235,255});
};

#endif // APP_H
