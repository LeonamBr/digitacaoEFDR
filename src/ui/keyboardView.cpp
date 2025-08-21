#include "KeyboardView.h"
#include <algorithm>
#include <cmath>

// renderiza um texto simples (sem cache p/ começar)
static SDL_Texture* RenderText(SDL_Renderer* r, TTF_Font* f,
                               const std::string& txt, SDL_Color c) {
    if (!f) return nullptr;
    SDL_Surface* s = TTF_RenderUTF8_Blended(f, txt.c_str(), c);
    if (!s) return nullptr;
    SDL_Texture* t = SDL_CreateTextureFromSurface(r, s);
    SDL_FreeSurface(s);
    return t;
}

void KeyboardView::Update(double nowSeconds) {
    m_now = nowSeconds;
    // limpa flashes expirados (lazy)
    for (auto it = m_flashes.begin(); it != m_flashes.end(); ) {
        const auto& f = it->second;
        if (m_now - f.t0 > f.dur) it = m_flashes.erase(it);
        else ++it;
    }
}

void KeyboardView::Feedback(const std::string& glyph, bool ok) {
    const std::string lab = Normalize(glyph);
    if (lab.empty()) return;
    const double HIT_DUR  = 0.15;
    const double MISS_DUR = 0.18;
    m_flashes[lab] = Flash{ m_now, ok ? HIT_DUR : MISS_DUR, ok ? Mode::Hit : Mode::Miss };
}

// Layout ABNT2 simplificado (com 'ç' ao lado do 'l')
const std::vector<KeyboardView::Row>& KeyboardView::LayoutABNT2() {
    static const std::vector<Row> L = {
        // 1ª fileira
        { {"`",1},{"1",1},{"2",1},{"3",1},{"4",1},{"5",1},{"6",1},{"7",1},{"8",1},{"9",1},{"0",1},{"-",1},{"=",1},{"Back",2} },
        // 2ª fileira
        { {"Tab",1.5},{"q",1},{"w",1},{"e",1},{"r",1},{"t",1},{"y",1},{"u",1},{"i",1},{"o",1},{"p",1},{"´",1},{"[",1},{"]",1} },
        // 3ª fileira (… k l ç ~ Enter)
        { {"Caps",1.8},{"a",1},{"s",1},{"d",1},{"f",1},{"g",1},{"h",1},{"j",1},{"k",1},{"l",1},{"ç",1},{"~",1},{"Enter",2} },
        // 4ª fileira
        { {"Shift",2.3},{"z",1},{"x",1},{"c",1},{"v",1},{"b",1},{"n",1},{"m",1},{",",1},{".",1},{";",1},{"/",1},{"Shift",2.3} },
        // 5ª fileira
        { {"Ctrl",1.3},{"Win",1.3},{"Alt",1.3},{"Space",6},{"AltGr",1.3},{"Win",1.3},{"Menu",1.3},{"Ctrl",1.3} }
    };
    return L;
}

std::string KeyboardView::Normalize(const std::string& g) {
    if (g.empty()) return "";
    if (g == " ") return "Space";
    // minúsculas para letras ASCII
    std::string s = g;
    if (s.size()==1 && s[0]>='A' && s[0]<='Z') s[0] = char(s[0]-'A'+'a');
    return s; // símbolos/UTF-8 ("ç") já batem com o label
}

void KeyboardView::DrawKey(SDL_Renderer* r, TTF_Font* font,
                           const Key& k, SDL_Rect rc, Mode mode, float pulseScale) {
    // pulso: aplica pequena variação de escala ao alvo
    if (mode == Mode::Target && pulseScale != 1.0f) {
        const int cx = rc.x + rc.w/2;
        const int cy = rc.y + rc.h/2;
        rc.w = int(rc.w * pulseScale);
        rc.h = int(rc.h * pulseScale);
        rc.x = cx - rc.w/2;
        rc.y = cy - rc.h/2;
    }

    SDL_Color fill, border{100,100,105,255}, label{230,230,235,255};
    switch (mode) {
        case Mode::Idle:   fill = SDL_Color{55,55,58,255};   break;
        case Mode::Target: fill = SDL_Color{60,120,70,255};  break; // verde suave
        case Mode::Hit:    fill = SDL_Color{30,170,90,255};  break; // verde forte
        case Mode::Miss:   fill = SDL_Color{170,60,60,255};  break; // vermelho
    }

    SDL_SetRenderDrawColor(r, fill.r, fill.g, fill.b, fill.a);
    SDL_RenderFillRect(r, &rc);

    SDL_SetRenderDrawColor(r, border.r, border.g, border.b, border.a);
    SDL_RenderDrawRect(r, &rc);

    if (font) {
        std::string text = k.label;
        if (m_shiftDown && text.size()==1) {
            if (text[0]>='a' && text[0]<='z') text[0] = char(text[0]-'a'+'A');
        }
        SDL_Texture* t = RenderText(r, font, text, label);
        if (t) {
            int tw=0, th=0; SDL_QueryTexture(t, nullptr, nullptr, &tw, &th);
            SDL_Rect td{ rc.x + (rc.w - tw)/2, rc.y + (rc.h - th)/2, tw, th };
            SDL_RenderCopy(r, t, nullptr, &td);
            SDL_DestroyTexture(t);
        }
    }
}

void KeyboardView::Draw(SDL_Renderer* r, TTF_Font* font,
                        const std::string& highlightGlyph,
                        int x, int y, int totalWidth, int keyH)
{
    const auto& rows = LayoutABNT2();
    const std::string hot = Normalize(highlightGlyph);

    // pulso para a tecla-alvo (0.98..1.04, ~8Hz)
    const float amp = 0.04f;
    const float w   = 8.0f; // frequência em Hz aprox.
    const float pulseScale = hot.empty() ? 1.0f : (1.0f + amp * std::sin(2.0f * 3.1415926f * w * float(m_now)));

    int curY = y;
    for (const auto& row : rows) {
        float wsum = 0.f; for (auto& k: row) wsum += k.w;
        const float unit = float(totalWidth) / wsum;

        int curX = x;
        for (const auto& k : row) {
            SDL_Rect rc{ curX, curY, int(k.w*unit - 6), keyH };

            Mode mode = Mode::Idle;
            // prioridade: flashes (Hit/Miss) sobre Target
            if (auto it = m_flashes.find(k.label); it != m_flashes.end()) {
                mode = it->second.type;
            } else if (!hot.empty() && k.label == hot) {
                mode = Mode::Target;
            }

            const float s = (mode == Mode::Target) ? pulseScale : 1.0f;
            DrawKey(r, font, k, rc, mode, s);
            curX += int(k.w * unit);
        }
        curY += keyH + 8;
    }
}
