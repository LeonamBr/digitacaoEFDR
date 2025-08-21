#include "App.h"
#include <algorithm>
#include <cmath>

// ------------------------ helpers locais ----------------------------------

static std::string first_utf8_cp(const char* s) {
    if (!s || !*s) return "";
    const unsigned char *p = (const unsigned char*)s;
    size_t len = 1;
    if ((*p & 0x80) == 0x00) len = 1;
    else if ((*p & 0xE0) == 0xC0) len = 2;
    else if ((*p & 0xF0) == 0xE0) len = 3;
    else if ((*p & 0xF8) == 0xF0) len = 4;
    return std::string((const char*)p, (const char*)p + len);
}

static std::string base_path() {
    char* b = SDL_GetBasePath();
    std::string out = b ? b : "";
    if (b) SDL_free(b);
    return out;
}

static SDL_Texture* RenderText(SDL_Renderer* r, TTF_Font* f,
                               const std::string& txt, SDL_Color c) {
    if (!f) return nullptr;
    SDL_Surface* s = TTF_RenderUTF8_Blended(f, txt.c_str(), c);
    if (!s) return nullptr;
    SDL_Texture* t = SDL_CreateTextureFromSurface(r, s);
    SDL_FreeSurface(s);
    return t;
}

// ----------------------------- App ----------------------------------------

bool App::Init() {
    return InitInternal(0, 0);
}

bool App::Init(int w, int h) {
    return InitInternal(w, h);
}

bool App::InitInternal(int reqW, int reqH) {
    if (SDL_Init(SDL_INIT_VIDEO | SDL_INIT_TIMER) != 0) {
        SDL_Log("SDL_Init falhou: %s", SDL_GetError());
        return false;
    }
    if (TTF_Init() != 0) {
        SDL_Log("TTF_Init falhou: %s", TTF_GetError());
        return false;
    }

    SDL_SetHint(SDL_HINT_RENDER_SCALE_QUALITY, "1");

    // tamanho da janela
    int winW = reqW, winH = reqH;
    if (winW <= 0 || winH <= 0) {
        SDL_Rect usable{0,0,1280,720};
        SDL_GetDisplayUsableBounds(0, &usable);
        winW = std::max(960, (int)(usable.w * 0.9));
        winH = std::max(540, (int)(usable.h * 0.75));
    }

    m_window = SDL_CreateWindow(
        "Curso de Digitacao EFDR",
        SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED,
        winW, winH,
        SDL_WINDOW_ALLOW_HIGHDPI // sem resizable por estética
    );
    if (!m_window) {
        SDL_Log("CreateWindow falhou: %s", SDL_GetError());
        return false;
    }

    m_renderer = SDL_CreateRenderer(
        m_window, -1, SDL_RENDERER_ACCELERATED | SDL_RENDERER_PRESENTVSYNC
    );
    if (!m_renderer) {
        SDL_Log("CreateRenderer falhou: %s", SDL_GetError());
        return false;
    }
    SDL_SetRenderDrawBlendMode(m_renderer, SDL_BLENDMODE_BLEND);

    ReloadFont();

    // Engine em modo case-sensitive
    m_engine.SetCaseSensitive(true);

    // Começar a receber texto UTF-8
    SDL_StartTextInput();
    m_textInputOn = true;

    // primeira lição
    StartLesson(0);
    return true;
}

void App::ReloadFont() {
    // tenta abrir RobotoMono-regular.ttf em caminhos razoáveis
    const char* REL = "resources/fonts/RobotoMono-regular.ttf";
    const std::vector<std::string> candidates = {
        base_path() + REL,
        REL,
        base_path() + std::string("resources/RobotoMono-regular.ttf"),
        "resources/RobotoMono-regular.ttf",
    };

    if (m_font) { TTF_CloseFont(m_font); m_font = nullptr; }

    int rw=0, rh=0; SDL_GetRendererOutputSize(m_renderer, &rw, &rh);
    const int px = std::max(14, rh / 36);

    for (const auto& p : candidates) {
        m_font = TTF_OpenFont(p.c_str(), px);
        if (m_font) { m_fontPath = p; break; }
    }
    if (!m_font) {
        SDL_Log("Falha ao abrir RobotoMono-regular.ttf: %s", TTF_GetError());
    } else {
        TTF_SetFontHinting(m_font, TTF_HINTING_LIGHT);
        SDL_Log("Fonte: %s", m_fontPath.c_str());
    }
}

void App::Shutdown() {
    if (m_textInputOn) { SDL_StopTextInput(); m_textInputOn = false; }
    if (m_font)     { TTF_CloseFont(m_font);     m_font = nullptr; }
    if (m_renderer) { SDL_DestroyRenderer(m_renderer); m_renderer = nullptr; }
    if (m_window)   { SDL_DestroyWindow(m_window);     m_window = nullptr; }
    TTF_Quit();
    SDL_Quit();
}

void App::ToggleFullscreen() {
    m_fullscreen = !m_fullscreen;
    SDL_SetWindowFullscreen(m_window, m_fullscreen ? SDL_WINDOW_FULLSCREEN_DESKTOP : 0);
}

void App::StartLesson(size_t index) {
    const auto& all = Lessons::All();
    if (all.empty()) return;

    if (index >= all.size()) index = 0;
    m_lessonIndex = index;

    m_engine.SetSequenceUTF8(all[index].text.c_str());

    m_elapsedMs   = 0;
    m_showSummary = false;
}

void App::HandleEvent(const SDL_Event& e) {
    switch (e.type) {
    case SDL_QUIT:
        m_running = false;
        return;

    case SDL_KEYDOWN: {
        const SDL_Keycode sym = e.key.keysym.sym;

        // shift visual
        if (sym == SDLK_LSHIFT || sym == SDLK_RSHIFT) {
            m_kb.SetShiftHeld(true);
        }

        // >>> ENTER: pular questão <<<
        if (!e.key.repeat && (
                e.key.keysym.scancode == SDL_SCANCODE_RETURN ||
                e.key.keysym.scancode == SDL_SCANCODE_KP_ENTER)) {

            if (m_engine.Finished()) {
                // Próxima lição
                const auto total = Lessons::All().size();
                if (total > 0) {
                    size_t next = (m_lessonIndex + 1) % total;
                    StartLesson(next);
                }
            } else {
                // Feedback visual de erro na tecla atual e pula
                const std::string g = m_engine.CurrentGlyph();
                if (!g.empty()) m_kb.Feedback(g, false);
                m_engine.SkipCurrent();
            }
            return; // já tratou o evento
        }

        if (sym == SDLK_ESCAPE) { m_running = false; return; }
        if (sym == SDLK_F11)    { ToggleFullscreen(); return; }
        if (sym == SDLK_r)      { StartLesson(m_lessonIndex); return; }

        if (sym == SDLK_F2)  { if (m_lessonIndex > 0) StartLesson(m_lessonIndex - 1); return; }
        if (sym == SDLK_F3)  { StartLesson(m_lessonIndex + 1); return; }
        if (sym == SDLK_RETURN) {
            if (m_engine.Finished()) {
                m_showSummary = true;
            }
            return;
        }
        return;
    }

    case SDL_KEYUP: {
        const SDL_Keycode sym = e.key.keysym.sym;
        if (sym == SDLK_LSHIFT || sym == SDLK_RSHIFT) {
            m_kb.SetShiftHeld(false);
        }
        return;
    }

    case SDL_TEXTINPUT: {
        // pega só o primeiro codepoint que veio nesse evento
        const std::string tok = first_utf8_cp(e.text.text);
        if (tok.empty()) return;

        // comparar com o esperado
        const std::string expect = m_engine.NextGlyph();
        const bool ok = (!expect.empty() && tok == expect);

        // feedback visual no teclado
        m_kb.Feedback(ok ? expect : tok, ok);

        // empurrar para o motor (ele cuida do avanço/erros)
        m_engine.PushText(tok.c_str());

        if (m_engine.Finished()) {
            m_showSummary = true;
        }
        return;
    }

    default: return;
    }
}

void App::Update(float dt) {
    // tempo absoluto pra animações
    m_nowSeconds += dt;

    // cronômetro da lição: só conta enquanto não terminou
    if (!m_engine.Finished()) {
        m_elapsedMs += int(std::round(dt * 1000.0f));
    }

    // shift visual (seguro mesmo sem key event)
    m_kb.SetShiftHeld((SDL_GetModState() & KMOD_SHIFT) != 0);

    // animações do teclado
    m_kb.Update(m_nowSeconds);
}

void App::drawText(const std::string& s, int x, int y, SDL_Color c) {
    SDL_Texture* t = RenderText(m_renderer, m_font, s, c);
    if (!t) return;
    int tw=0, th=0; SDL_QueryTexture(t, nullptr, nullptr, &tw, &th);
    SDL_Rect dst{ x, y, tw, th };
    SDL_RenderCopy(m_renderer, t, nullptr, &dst);
    SDL_DestroyTexture(t);
}

void App::Render() {
    int W=0, H=0; SDL_GetRendererOutputSize(m_renderer, &W, &H);

    SDL_SetRenderDrawColor(m_renderer, 24, 26, 28, 255);
    SDL_RenderClear(m_renderer);

    const int marginX = 20;
    int y = 16;

    // título da lição
    const auto& lessons = Lessons::All();
    const auto& lesson  = lessons[m_lessonIndex];
    drawText("Licao " + std::to_string(m_lessonIndex + 1) + ": " + lesson.title, marginX, y);
    y += 28;

    // instruções/top bar
    drawText("F2/F3: trocar licao   R: recomeçar   Enter: continuar   F11: fullscreen   ESC: sair",
             marginX, y, SDL_Color{180,180,190,255});
    y += 28;

    // sequência alvo e o que foi digitado
    drawText(std::string("Alvo: ") + m_engine.Sequence(), marginX, y);
    y += 24;
    drawText(std::string("Voce: ") + m_engine.Typed(), marginX, y);
    y += 24;

    // progresso e tempo
    const int prog = (int)m_engine.ProgressPercent();
    drawText("Progresso: " + std::to_string(prog) + "%", marginX, y);
    y += 24;

    // tempo (mm:ss)
    int ms = m_elapsedMs;
    int sec = ms/1000;
    int min = sec/60;
    sec %= 60;
    char buf[32]; SDL_snprintf(buf, sizeof(buf), "Tempo: %02d:%02d", min, sec);
    drawText(buf, marginX, y);
    y += 24;

    if (m_showSummary) {
        const float acc = m_engine.Accuracy();
        drawText("Concluido! Acuracia: " + std::to_string((int)std::round(acc)) + "%  (Enter para proxima)",
                 marginX, y, SDL_Color{120,220,120,255});
        y += 24;
    }

    // Teclado colado ao rodapé
    const int keyH = std::max(32, H / 18);
    const int kbRows = 5;
    const int vgap = 8;
    const int kbHeight = kbRows * keyH + (kbRows - 1) * vgap;
    const int kbY = H - kbHeight - 16;
    const int kbX = 16;
    const int kbW = W - 2 * kbX;

    m_kb.Draw(m_renderer, m_font, m_engine.CurrentGlyph(), kbX, kbY, kbW, keyH);

    SDL_RenderPresent(m_renderer);
}
