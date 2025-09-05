#include "core/LessonEngine.h"
#include <functional>
#include <algorithm>

static inline bool is_ascii_lower(char c){ return (c>='a' && c<='z'); }
static inline bool is_ascii_upper(char c){ return (c>='A' && c<='Z'); }
static inline char to_ascii_lower(char c){
    return is_ascii_upper(c) ? char(c - 'A' + 'a') : c;
}

std::string LessonEngine::first_utf8_cp(const char* s) {
    if (!s || !*s) return {};
    const unsigned char c0 = (unsigned char)s[0];
    int len = 1;
    if      ((c0 & 0x80) == 0x00) len = 1;
    else if ((c0 & 0xE0) == 0xC0) len = 2;
    else if ((c0 & 0xF0) == 0xE0) len = 3;
    else if ((c0 & 0xF8) == 0xF0) len = 4;
    else return {};
    return std::string(s, s + len);
}

void LessonEngine::utf8_for_each(const char* s, const std::function<void(const std::string&)>& fn) {
    if (!s) return;
    const char* p = s;
    while (*p) {
        const std::string tok = first_utf8_cp(p);
        if (tok.empty()) break;
        fn(tok);
        p += tok.size();
    }
}

bool LessonEngine::equal_token(const std::string& a, const std::string& b) const {
    if (m_caseSensitive) return a == b;
    // case-insensitive leve só para ASCII (A<->a). UTF-8 fica “as-is”.
    if (a.size() == 1 && b.size() == 1) {
        return to_ascii_lower(a[0]) == to_ascii_lower(b[0]);
    }
    return a == b;
}

void LessonEngine::SetSequenceUTF8(const char* utf8) {
    m_target.clear();
    m_typed.clear();
    m_keystrokes = 0;
    m_errors     = 0;
    m_finished   = false;

    utf8_for_each(utf8, [&](const std::string& tok){
        m_target.push_back(tok);
    });

    // sequência vazia => termina de cara
    if (m_target.empty()) m_finished = true;
}

void LessonEngine::Reset() {
    // mantém m_target, mas zera progresso
    m_typed.clear();
    m_keystrokes = 0;
    m_errors     = 0;
    m_finished   = m_target.empty();
}

std::string LessonEngine::CurrentGlyph() const {
    if (m_finished) return {};
    if (m_typed.size() >= m_target.size()) return {};
    return m_target[m_typed.size()];
}

void LessonEngine::SkipCurrent() {
    if (m_finished) return;
    if (m_typed.size() >= m_target.size()) return;
    // penaliza e avança
    m_keystrokes++;
    m_errors++;
    m_typed.push_back(m_target[m_typed.size()]);
    if (m_typed.size() == m_target.size()) m_finished = true;
}

void LessonEngine::PushText(const char* utf8) {
    if (m_finished) return;

    utf8_for_each(utf8, [&](const std::string& tok){
        if (m_finished) return;
        m_keystrokes++;

        if (m_typed.size() < m_target.size()) {
            const std::string& expect = m_target[m_typed.size()];
            if (equal_token(tok, expect)) {
                m_typed.push_back(expect); // avança com o token esperado
                if (m_typed.size() == m_target.size()) m_finished = true;
            } else {
                // erro não avança
                m_errors++;
            }
        } else {
            m_finished = true;
        }
    });
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
