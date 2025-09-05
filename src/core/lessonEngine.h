#ifndef LESSON_ENGINE_H
#define LESSON_ENGINE_H

#include <string>
#include <vector>
#include <cstdint>
#include <functional>

class LessonEngine {
public:
    LessonEngine() = default;

    // Configura a sequência alvo (UTF-8) e reseta o progresso/estatísticas.
    void SetSequenceUTF8(const char* utf8);
    // Zera typed/erros/contadores, mantendo a mesma sequência.
    void Reset();

    // Case-sensitivity do matching.
    void SetCaseSensitive(bool cs) { m_caseSensitive = cs; }
    bool CaseSensitive() const { return m_caseSensitive; }

    // Avanço por texto digitado (UTF-8). Conta acertos/erros.
    void PushText(const char* utf8);
    // Pula o glyph atual com penalidade (conta erro e avança).
    void SkipCurrent();

    // Estado
    bool Finished() const { return m_finished; }
    std::string CurrentGlyph() const;   // glyph esperado (UTF-8) ou "" se acabou

    // Exposição amigável (útil para HUD)
    std::string Sequence() const;       // string alvo inteira
    std::string Typed() const;          // string já digitada

    // Métricas
    uint32_t Keystrokes() const { return m_keystrokes; }
    uint32_t Errors() const { return m_errors; }
    // 0..1  (você já multiplica por 100 no App para exibir “xx%”)
    float Accuracy() const {
        if (m_keystrokes == 0) return 1.0f;
        const uint32_t ok = (m_keystrokes > m_errors) ? (m_keystrokes - m_errors) : 0;
        return float(ok) / float(m_keystrokes);
    }
    // 0..100
    float ProgressPercent() const {
        if (m_target.empty()) return 0.0f;
        return 100.0f * float(m_typed.size()) / float(m_target.size());
    }

private:
    // Helpers de UTF-8
    static std::string first_utf8_cp(const char* s); // 1 codepoint (ou "")
    static void utf8_for_each(const char* s, const std::function<void(const std::string&)>& fn);

    // Comparação com optional case-insensitive (apenas ASCII maiúsc/minúsc).
    bool equal_token(const std::string& a, const std::string& b) const;

    std::vector<std::string> m_target;  // tokens UTF-8 da sequência
    std::vector<std::string> m_typed;   // tokens já aceitos
    bool     m_finished      = false;
    bool     m_caseSensitive = true;
    uint32_t m_keystrokes    = 0;
    uint32_t m_errors        = 0;
};

#endif
