#include "LessonEngine.h"
#include <algorithm>
#include <cctype>

// Itera codepoints de uma string UTF-8 e chama fn para cada token (std::string)
void LessonEngine::utf8_for_each(const char* s, const std::function<void(const std::string&)>& fn) {
    if (!s) return;
    const unsigned char* p = (const unsigned char*)s;
    while (*p) {
        size_t len = 1;
        if      ((*p & 0x80) == 0x00) len = 1;
        else if ((*p & 0xE0) == 0xC0) len = 2;
        else if ((*p & 0xF0) == 0xE0) len = 3;
        else if ((*p & 0xF8) == 0xF0) len = 4;
        fn(std::string((const char*)p, (const char*)p + len));
        p += len;
    }
}

void LessonEngine::SetSequenceUTF8(const char* utf8) {
    m_target.clear();
    m_typed.clear();
    m_errors = 0;
    m_keystrokes = 0;

    utf8_for_each(utf8, [&](const std::string& tok){
        m_target.push_back(tok);
    });
}

void LessonEngine::ResetTyped() {
    m_typed.clear();
    m_errors = 0;
    m_keystrokes = 0;
}

static inline char lower_ascii(char c) {
    if (c >= 'A' && c <= 'Z') return char(c - 'A' + 'a');
    return c;
}

static std::string lower_if_ascii_letter(const std::string& tok) {
    if (tok.size() == 1) {
        return std::string(1, lower_ascii(tok[0]));
    }
    return tok; // não mexe em multibyte (acentos etc.)
}

void LessonEngine::PushText(const char* utf8) {
    // Considera só o primeiro codepoint de 'utf8'
    std::string tok;
    utf8_for_each(utf8, [&](const std::string& t){
        if (tok.empty()) tok = t;
    });
    if (tok.empty() || Finished()) return;

    m_keystrokes++;

    const size_t i = m_typed.size();
    if (i >= m_target.size()) return;

    const std::string& expect = m_target[i];

    bool ok = false;
    if (m_caseSensitive) {
        ok = (tok == expect);
    } else {
        ok = (lower_if_ascii_letter(tok) == lower_if_ascii_letter(expect));
    }

    if (ok) {
        m_typed.push_back(tok);
    } else {
        m_errors++;
        // não avança na sequência
    }
}

std::string LessonEngine::Sequence() const {
    std::string s;
    for (auto& t : m_target) s += t;
    return s;
}

std::string LessonEngine::Typed() const {
    std::string s;
    for (auto& t : m_typed) s += t;
    return s;
}

std::string LessonEngine::CurrentGlyph() const {
    if (Finished()) return "";
    return m_target[m_typed.size()];
}

float LessonEngine::ProgressPercent() const {
    if (m_target.empty()) return 0.f;
    return 100.0f * float(m_typed.size()) / float(m_target.size());
}

float LessonEngine::Accuracy() const {
    const int total = m_keystrokes;
    if (total <= 0) return 100.f;
    const int correct = total - m_errors;
    return 100.0f * float(std::max(0, correct)) / float(total);
}

bool LessonEngine::Finished() const {
    return !m_target.empty() && (m_typed.size() >= m_target.size());
}

void LessonEngine::SkipCurrent() {
    if (Finished()) return;
}
