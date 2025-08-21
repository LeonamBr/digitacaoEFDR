#ifndef LESSON_ENGINE_H
#define LESSON_ENGINE_H

#include <string>
#include <vector>
#include <functional>
#include <cstdint>

class LessonEngine {
public:
    LessonEngine() = default;

    void SetSequenceUTF8(const char* utf8);
    void PushText(const char* utf8); // recebe o glifo digitado (ou uma string com 1 codepoint)
    void ResetTyped();

    // leitura de estado
    std::string Sequence() const; // sequência alvo completa
    std::string Typed() const;    // o que foi digitado até agora
    std::string CurrentGlyph() const;    // próximo glifo esperado (ou "")
    float ProgressPercent() const;       // 0..100
    float Accuracy() const;              // 0..100
    bool Finished() const;
    void SkipCurrent();

    // alias p/ compatibilidade com chamadas antigas
    std::string NextGlyph() const { return CurrentGlyph(); }

    // regras
    void SetCaseSensitive(bool v) { m_caseSensitive = v; }

private:
    static void utf8_for_each(const char* s, const std::function<void(const std::string&)>& fn);

    std::vector<std::string> m_target; // tokens (codepoints) da sequência
    std::vector<std::string> m_typed;  // tokens digitados corretos
    int m_errors = 0;
    int m_keystrokes = 0;
    bool m_caseSensitive = true;
};

#endif // LESSON_ENGINE_H
