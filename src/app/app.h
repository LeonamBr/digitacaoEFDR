#ifndef APP_H
#define APP_H

#include <SDL.h>
#include <SDL_ttf.h>
#include <SDL_image.h>
#include <string>
#include "core/LessonEngine.h"
#include "core/Lessons.h"
#include "core/ProgressStore.h"
#include "ui/KeyboardView.h"

class App {
public:
    bool Init();            // cria janela, carrega fontes, entra no Main Menu
    void Run();             // loop principal
    void Shutdown();        // libera recursos
    void EnterMainMenu();
    void LeaveMainMenu();
    bool LoadMenuAssets();
    void UnloadMenuAssets();

private:
    // ===== fluxo base =====
    void HandleEvent(const SDL_Event& e);
    void Update(float dt);
    void Render();

    // ===== telas =====
    enum class EScreen { MainMenu, Lesson, Results, Exit };
    void SwitchScreen(EScreen next);

    // render/inputs por tela
    void RenderMainMenu();
    void RenderResults();

    void HandleEventMainMenu(const SDL_Event& e);
    void HandleEventLesson(const SDL_Event& e);
    void HandleEventResults(const SDL_Event& e);

    // ===== lições =====
    void StartLesson(size_t index);
    void NextLesson();
    void PrevLesson();

    // ===== util =====
    void drawText(const std::string& txt, int x, int y, SDL_Color c = {230,230,235,255});
    void DrawPauseOverlay();

    static SDL_FRect Cover(const SDL_FRect& dst, float srcW, float srcH);
    static SDL_FRect Fit(const SDL_FRect& dst, float srcW, float srcH);

    // helpers locais
    std::string MakeAssetPath(const std::string& relative) const;
    void DrawTextureFullWindow(SDL_Texture* tex);

    // ===== SDL =====
    SDL_Window*   m_window   = nullptr;
    SDL_Renderer* m_renderer = nullptr;
    TTF_Font*     m_font     = nullptr; // UI/keys
    TTF_Font*     m_fontLarge= nullptr; // títulos
    SDL_Texture* m_menuBgTex = nullptr;
    SDL_Texture* m_menuLogoTex = nullptr;
    bool m_menuGfxReady = false;

    SDL_Texture* m_menuBackground = nullptr; // bg do menu
    bool m_imgOk = false;                    // guarda se IMG_Init foi ok

    // ===== estado global =====
    int    m_width  = 1024;
    int    m_height = 600;
    bool   m_running = false;

    // ===== estado de tela/menus =====
    EScreen m_screen = EScreen::MainMenu;
    bool   m_paused   = false; // só usado na Lesson
    int    m_pauseSel = 0;     // seleção do pause overlay
    int    m_menuSel  = 0;     // seleção do main menu

    // ===== input =====
    bool   m_shiftHeld = false;

    // ===== lição/engine/teclado =====
    size_t       m_lessonIndex = 0;
    LessonEngine m_engine;
    KeyboardView m_kb;

    // ===== tempo =====
    uint32_t m_prevTicks = 0;
    float    m_elapsedSec = 0.0f; // cronômetro da lição

    // ===== progresso =====
    ProgressStore m_store;

    // ===== resultados =====
    struct ResultsData {
        int accuracy = 0;  // 0..100
        int timeSec  = 0;
        int stars    = 0;  // 0..3
    } m_results;
};

#endif
