#include "app/App.h"
#include <algorithm>
#include <cstdio>
#include <cmath>

// ===================================================
// Init / Run / Shutdown
// ===================================================
bool App::Init() {
    if (SDL_Init(SDL_INIT_VIDEO) != 0) return false;
    
    if (TTF_Init() != 0) return false;

    if ((IMG_Init(IMG_INIT_PNG | IMG_INIT_JPG) & (IMG_INIT_PNG | IMG_INIT_JPG)) == 0) {
    // log opcional: IMG_GetError()
    return false;
    }

    m_window = SDL_CreateWindow("Curso de Digitacao",
                                SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED,
                                m_width, m_height, SDL_WINDOW_RESIZABLE);
    if (!m_window) return false;

    m_renderer = SDL_CreateRenderer(m_window, -1,
                                    SDL_RENDERER_ACCELERATED | SDL_RENDERER_PRESENTVSYNC);
    if (!m_renderer) return false;

    // mundo lógico base (pode manter fixo; seu layout já é proporcional)
    SDL_RenderSetLogicalSize(m_renderer, m_width, m_height);
    SDL_RenderSetIntegerScale(m_renderer, SDL_TRUE);

    // fontes
    const char* kFontPath = "resources/fonts/RobotoMono-Regular.ttf";
    m_font      = TTF_OpenFont(kFontPath, 20);
    m_fontLarge = TTF_OpenFont(kFontPath, 28);
    if (!m_font || !m_fontLarge) return false;

    // engine
    m_engine.SetCaseSensitive(true);

    // começa no menu
    m_prevTicks = SDL_GetTicks();
    m_running   = true;
    SwitchScreen(EScreen::MainMenu);
    LoadMenuAssets();
    return true;
}

void App::Run() {
    while (m_running) {
        SDL_Event e;
        while (SDL_PollEvent(&e)) HandleEvent(e);

        uint32_t now = SDL_GetTicks();
        float dt = (now - m_prevTicks) * 0.001f;
        m_prevTicks = now;

        Update(dt);
        Render();
        SDL_RenderPresent(m_renderer);
    }
}

void App::Shutdown() {
    if (m_fontLarge) { TTF_CloseFont(m_fontLarge); m_fontLarge = nullptr; }
    if (m_font)      { TTF_CloseFont(m_font);      m_font = nullptr; }
    if (m_renderer)  { SDL_DestroyRenderer(m_renderer); m_renderer = nullptr; }
    if (m_window)    { SDL_DestroyWindow(m_window);     m_window   = nullptr; }
    TTF_Quit();
    SDL_Quit();
}

// ===================================================
// State machine
// ===================================================
void App::SwitchScreen(EScreen next) {
    m_screen = next;
    if (m_screen == EScreen::Lesson) {
        m_paused = false;
        m_elapsedSec = 0.0f;
        m_prevTicks = SDL_GetTicks();
    }
}

// ===================================================
// Lições
// ===================================================
void App::StartLesson(size_t index) {
    const auto& all = Lessons::All();
    if (all.empty()) return;

    if (index >= all.size()) index = 0;
    m_lessonIndex = index;

    m_engine.SetSequenceUTF8(all[m_lessonIndex].text.c_str());
    m_engine.Reset();

    // título da janela = título da lição
    SDL_SetWindowTitle(m_window, all[m_lessonIndex].title.c_str());

    m_elapsedSec = 0.0f;
    m_kb.Clear(); // zera animações
}

void App::NextLesson() {
    const auto& all = Lessons::All();
    if (all.empty()) return;
    m_lessonIndex = (m_lessonIndex + 1) % all.size();
    StartLesson(m_lessonIndex);
}

void App::PrevLesson() {
    const auto& all = Lessons::All();
    if (all.empty()) return;
    m_lessonIndex = (m_lessonIndex + all.size() - 1) % all.size();
    StartLesson(m_lessonIndex);
}

// ===================================================
// Eventos (dispatcher por tela)
// ===================================================
void App::HandleEvent(const SDL_Event& e) {
    if (e.type == SDL_QUIT) { m_running = false; return; }

    if (e.type == SDL_WINDOWEVENT && e.window.event == SDL_WINDOWEVENT_RESIZED) {
        m_width = e.window.data1; m_height = e.window.data2;
        SDL_RenderSetLogicalSize(m_renderer, m_width, m_height);
    }

    switch (m_screen) {
        case EScreen::MainMenu: HandleEventMainMenu(e); break;
        case EScreen::Lesson:   HandleEventLesson(e);   break;
        case EScreen::Results:  HandleEventResults(e);  break;
        default: break;
    }
}

void App::HandleEventMainMenu(const SDL_Event& e) {
    if (e.type != SDL_KEYDOWN) return;
    const SDL_Keycode k = e.key.keysym.sym;

    if (k == SDLK_F11) {
        Uint32 flags = SDL_GetWindowFlags(m_window);
        bool fs = (flags & SDL_WINDOW_FULLSCREEN_DESKTOP) != 0;
        SDL_SetWindowFullscreen(m_window, fs ? 0 : SDL_WINDOW_FULLSCREEN_DESKTOP);
        SDL_RenderSetLogicalSize(m_renderer, m_width, m_height);
        return;
    }
    if (k == SDLK_ESCAPE) { m_running = false; return; }

    const int N = 4; // itens do menu
    if (k == SDLK_UP)    m_menuSel = (m_menuSel + N - 1) % N;
    if (k == SDLK_DOWN)  m_menuSel = (m_menuSel + 1) % N;

    if (k == SDLK_RETURN || k == SDLK_KP_ENTER) {
        if (m_menuSel == 0) { // Novo jogo
            // opcional: limpar save (não há Clear() em ProgressStore; podemos sobrescrever progresso ao avançar)
            StartLesson(0);
            SwitchScreen(EScreen::Lesson);
        } else if (m_menuSel == 1) { // Continuar
            m_store.Load();
            size_t go = 0;
            const auto& all = Lessons::All();
            for (size_t i=0; i<all.size(); ++i) {
                LessonProgress p = m_store.Get((int)i);
                if (p.bestStars < 3) { go = i; break; }
                if (i == all.size()-1) go = i; // se tudo com 3 estrelas, abre a última
            }
            StartLesson(go);
            SwitchScreen(EScreen::Lesson);
        } else if (m_menuSel == 2) { // Aulas (por enquanto abre 0)
            StartLesson(0);
            SwitchScreen(EScreen::Lesson);
        } else { // Sair
            m_running = false;
        }
    }
}

void App::HandleEventLesson(const SDL_Event& e) {
    switch (e.type) {
    case SDL_KEYDOWN: {
        const SDL_Keycode k = e.key.keysym.sym;

        if (k == SDLK_ESCAPE) { m_paused = !m_paused; m_pauseSel = 0; return; }

        if (k == SDLK_F11) {
            Uint32 flags = SDL_GetWindowFlags(m_window);
            bool fs = (flags & SDL_WINDOW_FULLSCREEN_DESKTOP) != 0;
            SDL_SetWindowFullscreen(m_window, fs ? 0 : SDL_WINDOW_FULLSCREEN_DESKTOP);
            SDL_RenderSetLogicalSize(m_renderer, m_width, m_height);
            return;
        }

        if (k == SDLK_LSHIFT || k == SDLK_RSHIFT) { m_shiftHeld = true; return; }

        if (m_paused) {
            if (k == SDLK_UP)   { if (m_pauseSel > 0) m_pauseSel--; }
            if (k == SDLK_DOWN) { if (m_pauseSel < 3) m_pauseSel++; }
            if (k == SDLK_RETURN || k == SDLK_KP_ENTER) {
                switch (m_pauseSel) {
                    case 0: m_paused = false;                        break; // Continuar
                    case 1: StartLesson(m_lessonIndex); m_paused=false; break; // Reiniciar
                    case 2: NextLesson(); m_paused=false;            break; // Próxima
                    case 3: SwitchScreen(EScreen::MainMenu);         break; // Sair (volta ao menu)
                }
            }
            return;
        }

        // atalhos
        if (m_engine.Finished() && (k == SDLK_RETURN || k == SDLK_KP_ENTER)) {
            // Results será disparado no Update; aqui podemos ignorar
            return;
        }
        if (k == SDLK_F3) { PrevLesson(); return; }
        if (k == SDLK_F4) { NextLesson(); return; }
        if (k == SDLK_F2) { m_engine.SkipCurrent(); return; }
        break;
    }
    case SDL_KEYUP:
        if (e.key.keysym.sym == SDLK_LSHIFT || e.key.keysym.sym == SDLK_RSHIFT) {
            m_shiftHeld = false;
        }
        break;

    case SDL_TEXTINPUT: {
        if (m_paused) break;
        if (m_engine.Finished()) break;

        const char* t = e.text.text;
        std::string cp;
        if (t && *t) {
            unsigned char c0 = (unsigned char)t[0];
            int len = 1;
            if      ((c0 & 0x80) == 0x00) len = 1;
            else if ((c0 & 0xE0) == 0xC0) len = 2;
            else if ((c0 & 0xF0) == 0xE0) len = 3;
            else if ((c0 & 0xF8) == 0xF0) len = 4;
            cp.assign(t, t + len);
        }

        if (!cp.empty()) {
            const std::string expect = m_engine.CurrentGlyph();
            const bool ok = (!expect.empty() && cp == expect);
            m_engine.PushText(cp.c_str());
            m_kb.Feedback(cp, ok);
        }
        break;
    }

    default: break;
    }
}

void App::HandleEventResults(const SDL_Event& e) {
    if (e.type != SDL_KEYDOWN) return;
    const SDL_Keycode k = e.key.keysym.sym;

    if (k == SDLK_ESCAPE) { SwitchScreen(EScreen::MainMenu); return; }
    if (k == SDLK_r)      { StartLesson(m_lessonIndex); SwitchScreen(EScreen::Lesson); return; }
    if (k == SDLK_RETURN || k == SDLK_KP_ENTER) {
        StartLesson(m_lessonIndex + 1);
        SwitchScreen(EScreen::Lesson);
        return;
    }
}

// ===================================================
// Update
// ===================================================
void App::Update(float dt) {

    m_kb.Update(SDL_GetTicks64() / 1000.0);

    if (m_screen != EScreen::Lesson) return;
    if (m_paused) return;

    m_elapsedSec += dt;
    m_kb.SetShiftHeld(m_shiftHeld);
    m_kb.Update(m_elapsedSec);

    if (m_engine.Finished()) {
        // calcula resultados
        int acc  = (int)std::round(100.0f * m_engine.Accuracy());
        int tsec = (int)std::round(m_elapsedSec);
        int stars = (acc >= 95) ? 3 : (acc >= 85) ? 2 : (acc >= 70) ? 1 : 0;

        // salva progresso
        m_store.Load();
        m_store.Submit((int)m_lessonIndex, stars, acc, tsec);
        m_store.Save();

        // guarda e vai para Results
        m_results = { acc, tsec, stars };
        SwitchScreen(EScreen::Results);
    }
}

// ===================================================
// Helpers de texto (simples, sem cache)
// ===================================================
void App::drawText(const std::string& txt, int x, int y, SDL_Color c) {
    SDL_Surface* s = TTF_RenderUTF8_Blended(m_font, txt.c_str(), c);
    if (!s) return;
    SDL_Texture* t = SDL_CreateTextureFromSurface(m_renderer, s);
    SDL_FreeSurface(s);
    if (!t) return;
    int w,h; SDL_QueryTexture(t, nullptr, nullptr, &w, &h);
    SDL_Rect dst{ x, y, w, h };
    SDL_RenderCopy(m_renderer, t, nullptr, &dst);
    SDL_DestroyTexture(t);
}

void App::DrawPauseOverlay() {
    SDL_SetRenderDrawBlendMode(m_renderer, SDL_BLENDMODE_BLEND);
    SDL_SetRenderDrawColor(m_renderer, 0, 0, 0, 140);
    SDL_Rect full{0,0,m_width,m_height};
    SDL_RenderFillRect(m_renderer, &full);

    const char* items[4] = { "Continuar", "Reiniciar licao", "Proxima licao", "Menu" };
    int x = 220, y = 140;
    for (int i=0;i<4;++i) {
        std::string line = (i==m_pauseSel ? "> " : "  ");
        line += items[i];
        drawText(line, x, y);
        y += 40;
    }
    drawText("ESC para voltar", x, y+20);
}

// helpers locais
static void drawLine(SDL_Renderer* r, TTF_Font* f, const std::string& s, SDL_Color c, int x, int y) {
    if (!f || s.empty()) return;
    SDL_Surface* surf = TTF_RenderUTF8_Blended(f, s.c_str(), c);
    if (!surf) return;
    SDL_Texture* tex = SDL_CreateTextureFromSurface(r, surf);
    SDL_Rect dst{ x, y, surf->w, surf->h };
    SDL_FreeSurface(surf);
    SDL_RenderCopy(r, tex, nullptr, &dst);
    SDL_DestroyTexture(tex);
}

static int drawWrapped(SDL_Renderer* r, TTF_Font* f, const std::string& s, SDL_Color c,
                       int x, int y, int maxW, int lineGap) {
    if (!f || s.empty()) return y;
    std::string line, word;
    int curY = y;

    auto flushLine = [&]{
        if (line.empty()) return;
        drawLine(r, f, line, c, x, curY);
        curY += TTF_FontHeight(f) + lineGap;
        line.clear();
    };

    size_t i = 0;
    while (i < s.size()) {
        size_t j = s.find(' ', i);
        if (j == std::string::npos) j = s.size();
        word = s.substr(i, j - i);

        const std::string candidate = line.empty() ? word : (line + " " + word);
        int w=0,h=0; TTF_SizeUTF8(f, candidate.c_str(), &w, &h);
        if (w <= maxW) {
            line = candidate;
        } else {
            flushLine();
            TTF_SizeUTF8(f, word.c_str(), &w, &h);
            if (w > maxW) { drawLine(r, f, word, c, x, curY); curY += TTF_FontHeight(f) + lineGap; }
            else { line = word; }
        }
        i = j; if (i < s.size() && s[i] == ' ') ++i;
    }
    flushLine();
    return curY;
}

// ===================================================
// Render (dispatcher por tela)
// ===================================================
void App::Render() {
    if (m_screen == EScreen::MainMenu) {
        RenderMainMenu();
        return;
    }
    if (m_screen == EScreen::Results) {
        RenderResults();
        return;
    }

    // ===== Tela de LIÇÃO =====
    SDL_SetRenderDrawColor(m_renderer, 22, 23, 25, 255);
    SDL_RenderClear(m_renderer);

    int W = 0, H = 0;
    SDL_RenderGetLogicalSize(m_renderer, &W, &H);
    if (W == 0 || H == 0) SDL_GetRendererOutputSize(m_renderer, &W, &H);

    const int margin      = std::max(16, W / 64);
    const int between     = std::max(8,  W / 160);
    const int hudMaxWidth = W - 2 * margin;

    // teclado ancorado no rodapé
    const int rows = 5;
    const int keyH = std::clamp(H / 14, 28, 96);
    const int gap  = std::max(6, W / 200);
    const int kbH  = rows * keyH + (rows - 1) * gap;

    const int kbX = margin;
    const int kbW = W - 2 * margin;
    const int kbY = H - margin - kbH;

    SDL_Color fg {230,230,235,255};
    SDL_Color dim{180,180,186,255};

    int y = margin;

    // título (da lição, vindo do título da janela)
    const char* wtitle = SDL_GetWindowTitle(m_window);
    if (wtitle && *wtitle) {
        drawLine(m_renderer, m_fontLarge, wtitle, fg, margin, y);
        y += TTF_FontHeight(m_fontLarge) + between;
    }

    drawLine(m_renderer, m_font, std::string("Alvo: ") + m_engine.Sequence(), fg, margin, y);
    y += TTF_FontHeight(m_font) + between;

    drawLine(m_renderer, m_font, std::string("Voce: ") + m_engine.Typed(), fg, margin, y);
    y += TTF_FontHeight(m_font) + between;

    const int prog     = (int)m_engine.ProgressPercent();
    const int accInt   = (int)std::round(100.0f * m_engine.Accuracy());
    const std::string stats =
        "Progresso: " + std::to_string(prog) + "%    Precisao: " + std::to_string(accInt) + "%";
    drawLine(m_renderer, m_font, stats, fg, margin, y);
    y += TTF_FontHeight(m_font) + between;

    char tbuf[32]; std::snprintf(tbuf, sizeof tbuf, "Tempo: %.1fs", double(m_elapsedSec));
    drawLine(m_renderer, m_font, tbuf, fg, margin, y);
    y += TTF_FontHeight(m_font) + between;

    const std::string dicas =
        "Dicas: ENTER -> proxima quando terminar | ESC -> pause | "
        "F2 pular glyph | F3/F4 navegar";
    drawWrapped(m_renderer, m_font, dicas, dim, margin, y, hudMaxWidth, 4);

    // teclado
    m_kb.Draw(m_renderer, m_font, m_engine.CurrentGlyph(), kbX, kbY, kbW, keyH);

    // overlay de pausa (opcional)
    if (m_paused) {
        DrawPauseOverlay();
    }
}

// ===================================================
// Renders extras
// ===================================================
// ---- Helpers Cover/Fit ----
SDL_FRect App::Cover(const SDL_FRect& dst, float srcW, float srcH) {
    if (dst.w <= 0.f || dst.h <= 0.f || srcW <= 0.f || srcH <= 0.f) return dst;
    float sw = dst.w, sh = dst.h;
    float s = std::max(sw / srcW, sh / srcH);
    float w = srcW * s, h = srcH * s;
    return SDL_FRect{ dst.x + (sw - w) * 0.5f, dst.y + (sh - h) * 0.5f, w, h };
}

SDL_FRect App::Fit(const SDL_FRect& dst, float srcW, float srcH) {
    if (dst.w <= 0.f || dst.h <= 0.f || srcW <= 0.f || srcH <= 0.f) return dst;
    float sw = dst.w, sh = dst.h;
    float s = std::min(sw / srcW, sh / srcH);
    float w = srcW * s, h = srcH * s;
    return SDL_FRect{ dst.x + (sw - w) * 0.5f, dst.y + (sh - h) * 0.5f, w, h };
}

// ---- Ciclo de vida do menu ----
void App::EnterMainMenu() {
    // paths de assets (ajuste se quiser)
    const std::string bgPath   = "resources/images/backgrounds/bg_efdr.png";
    const std::string logoPath = "resources/images/logos/bg_efdr.png";

    auto loadTex = [this](const std::string& path) -> SDL_Texture* {
        if (path.empty()) return nullptr;
        SDL_Texture* t = IMG_LoadTexture(/*renderer*/ m_renderer, path.c_str());
        if (!t) {
            // Fallback BMP (se quiser)
            SDL_Surface* s = SDL_LoadBMP(path.c_str());
            if (!s) return nullptr;
            t = SDL_CreateTextureFromSurface(m_renderer, s);
            SDL_FreeSurface(s);
        }
        return t;
    };

    // libera anteriores, se houver
    if (m_menuBgTex)   { SDL_DestroyTexture(m_menuBgTex);   m_menuBgTex = nullptr; }
    if (m_menuLogoTex) { SDL_DestroyTexture(m_menuLogoTex); m_menuLogoTex = nullptr; }

    m_menuBgTex   = loadTex(bgPath);
    m_menuLogoTex = loadTex(logoPath);

    m_menuGfxReady = (m_menuBgTex != nullptr) || (m_menuLogoTex != nullptr);
}

void App::LeaveMainMenu() {
    if (m_menuBgTex)   { SDL_DestroyTexture(m_menuBgTex);   m_menuBgTex = nullptr; }
    if (m_menuLogoTex) { SDL_DestroyTexture(m_menuLogoTex); m_menuLogoTex = nullptr; }
    m_menuGfxReady = false;
}

// ---- Render do menu (passo 1) ----
void App::RenderMainMenu() {

    DrawTextureFullWindow(m_menuBackground);
    // fundo sólido por segurança
    SDL_SetRenderDrawColor(m_renderer, 8, 10, 14, 255);
    SDL_RenderClear(m_renderer);

    int rw = 0, rh = 0;
    SDL_GetRendererOutputSize(m_renderer, &rw, &rh);
    const SDL_FRect viewport{ 0.f, 0.f, float(rw), float(rh) };

    // 1) BG (cover)
    if (m_menuBgTex) {
        int tw = 0, th = 0; SDL_QueryTexture(m_menuBgTex, nullptr, nullptr, &tw, &th);
        SDL_FRect bgDst = Cover(viewport, float(tw), float(th));
        SDL_RenderCopyF(m_renderer, m_menuBgTex, nullptr, &bgDst);
    }

    // 2) Logo (fit numa caixa relativa no topo)
    if (m_menuLogoTex) {
        int lw = 0, lh = 0; SDL_QueryTexture(m_menuLogoTex, nullptr, nullptr, &lw, &lh);

        // caixa destino: 60% da largura, 22% da altura, ancorada no topo
        SDL_FRect logoBox{
            viewport.x + viewport.w * 0.20f,
            viewport.y + viewport.h * 0.10f,
            viewport.w * 0.60f,
            viewport.h * 0.22f
        };
        SDL_FRect logoDst = Fit(logoBox, float(lw), float(lh));
        SDL_RenderCopyF(m_renderer, m_menuLogoTex, nullptr, &logoDst);
    }

    // 3) Placeholder dos itens (texto/retângulos) — até criarmos os botões texturizados
    // aqui só para orientação visual
    const float colW = viewport.w * 0.36f;
    const float colH = viewport.h * 0.45f;
    SDL_FRect menuCol{
        viewport.x + (viewport.w - colW) * 0.5f,
        viewport.y + viewport.h * 0.40f,
        colW,
        colH
    };

    // desenha “slots” de botões
    const int btnCount = 4;
    for (int i = 0; i < btnCount; ++i) {
        SDL_FRect r{
            menuCol.x,
            menuCol.y + i * (menuCol.h / btnCount) + 8.0f,
            menuCol.w,
            (menuCol.h / btnCount) - 16.0f
        };
        // borda leve
        SDL_SetRenderDrawColor(m_renderer, 255, 255, 255, 30);
        SDL_RenderDrawRectF(m_renderer, &r);
    }

    // finalize
    SDL_RenderPresent(m_renderer);
}

void App::RenderResults() {
    SDL_SetRenderDrawColor(m_renderer, 24,24,24,255);
    SDL_RenderClear(m_renderer);
    SDL_Color fg{220,220,220,255};

    int W=0,H=0; SDL_RenderGetLogicalSize(m_renderer, &W, &H);
    if (W==0 || H==0) SDL_GetRendererOutputSize(m_renderer, &W, &H);

    int x = W/8, y = H/6;
    drawLine(m_renderer, m_fontLarge, "Resultado da licao", fg, x, y);
    y += TTF_FontHeight(m_fontLarge) + 16;

    char buf[128];
    std::snprintf(buf, sizeof buf, "Precisao: %d%%   Tempo: %ds", m_results.accuracy, m_results.timeSec);
    drawLine(m_renderer, m_font, buf, fg, x, y);
    y += TTF_FontHeight(m_font) + 8;

    std::string stars = std::string(m_results.stars, '★');
    if (stars.empty()) stars = "☆";
    drawLine(m_renderer, m_fontLarge, "Estrelas: " + stars, fg, x, y);
    y += TTF_FontHeight(m_fontLarge) + 20;

    drawLine(m_renderer, m_font, "ENTER: proxima | R: repetir | ESC: menu", fg, x, y);
}

// ---------------------------------------------------------
// Monta caminho a partir do diretório do executável
// Evita não encontrar arquivo quando roda a partir do build/
// ---------------------------------------------------------
std::string App::MakeAssetPath(const std::string& relative) const {
    char* base = SDL_GetBasePath();
    std::string out;
    if (base) {
        out = std::string(base) + relative;
        SDL_free(base);
    } else {
        // fallback relativo
        out = relative;
    }
    return out;
}

// ---------------------------------------------------------
// Carrega os assets do MENU (chame ao entrar no estado Menu)
// ---------------------------------------------------------
bool App::LoadMenuAssets() {
    // Inicializa SDL_image (uma vez)
    if (!m_imgOk) {
        const int want = IMG_INIT_PNG | IMG_INIT_JPG;
        const int got  = IMG_Init(want);
        if ((got & want) != want) {
            SDL_Log("IMG_Init falhou: %s", IMG_GetError());
            m_imgOk = false;
        } else {
            m_imgOk = true;
        }
    }

    // Libera se já havia textura
    if (m_menuBackground) {
        SDL_DestroyTexture(m_menuBackground);
        m_menuBackground = nullptr;
    }

    // Ajuste o caminho relativo conforme seu repo:
    // Ex.: "assets/ui/menu_bg.png" (sugestão)
    const std::string rel  = "resources/images/backgrounds/bg_efdr.png";
    const std::string path = MakeAssetPath(rel);

    // Use IMG_LoadTexture para criar direto no renderer existente
    m_menuBackground = IMG_LoadTexture(m_renderer, path.c_str());
    if (!m_menuBackground) {
        SDL_Log("IMG_LoadTexture falhou (%s): %s", path.c_str(), IMG_GetError());
        return false;
    }

    return true;
}

// ---------------------------------------------------------
// Descarta assets do MENU (chame ao sair do estado Menu)
// ---------------------------------------------------------
void App::UnloadMenuAssets() {
    if (m_menuBackground) {
        SDL_DestroyTexture(m_menuBackground);
        m_menuBackground = nullptr;
    }
    // Não chame IMG_Quit aqui para não afetar outras texturas do app
}

// ---------------------------------------------------------
// Desenha textura preenchendo a janela, mantendo aspecto
// ---------------------------------------------------------
void App::DrawTextureFullWindow(SDL_Texture* tex) {
    if (!tex) return;

    int winW = 0, winH = 0;
    SDL_GetRendererOutputSize(m_renderer, &winW, &winH);

    int tw = 0, th = 0;
    SDL_QueryTexture(tex, nullptr, nullptr, &tw, &th);

    // Letterbox/pillarbox mantendo aspecto
    SDL_Rect dst{0, 0, winW, winH};
    const double sX = (tw > 0) ? (double)winW / tw : 1.0;
    const double sY = (th > 0) ? (double)winH / th : 1.0;
    const double s  = (sX < sY) ? sX : sY;
    const int rw = (int)(tw * s);
    const int rh = (int)(th * s);
    dst.w = rw;
    dst.h = rh;
    dst.x = (winW - rw) / 2;
    dst.y = (winH - rh) / 2;

    SDL_RenderCopy(m_renderer, tex, nullptr, &dst);
}
