#ifndef KEYBOARD_VIEW_H
#define KEYBOARD_VIEW_H

#include <SDL.h>
#include <SDL_ttf.h>
#include <string>
#include <vector>
#include <unordered_map>

class KeyboardView {
public:
    // Atualiza o "tempo atual" (em segundos) para animar.
    void Update(double nowSeconds);

    // Registra um feedback visual para um glyph digitado.
    // glyph: UTF-8 (ex.: "a", "ç", " ")
    // ok   : true = acerto (flash verde); false = erro (flash vermelho)
    void Feedback(const std::string& glyph, bool ok);

    // Desenha o teclado; highlightGlyph é o próximo glyph da lição.
    void Draw(SDL_Renderer* r, TTF_Font* font,
              const std::string& highlightGlyph,
              int x = 20, int y = 160, int totalWidth = 980, int keyH = 46);

    // Maiúsculas visuais quando segurando shift
    void SetShiftHeld(bool v) { m_shiftDown = v; }

private:
    struct Key { std::string label; float w; }; // w = largura relativa
    using Row = std::vector<Key>;

    enum class Mode { Idle, Target, Hit, Miss };

    static const std::vector<Row>& LayoutABNT2();
    static std::string Normalize(const std::string& g); // " "->"Space", "A"->"a"
    void DrawKey(SDL_Renderer* r, TTF_Font* font,
                 const Key& k, SDL_Rect rc, Mode mode, float pulseScale);

    // feedback por tecla (label normalizado -> início/tempo/tipo)
    struct Flash { double t0; double dur; Mode type; };
    std::unordered_map<std::string, Flash> m_flashes;

    // tempo atual em segundos (definido via Update)
    double m_now = 0.0;

    // shift visual
    bool m_shiftDown = false;
};

#endif // KEYBOARD_VIEW_H
