#include "LessonEngine.h"

void LessonEngine::LoadSequence(const std::string& seq) {
    m_seq = seq; m_index = 0;
}

bool LessonEngine::TryConsume(const std::string& ch) {
    if (Done()) return false;
    const char expected = m_seq[m_index];
    if (!ch.empty() && ch[0] == expected) {
        ++m_index; return true;
    }
    return false;
}

bool LessonEngine::Done() const { return m_index >= m_seq.size(); }

std::string LessonEngine::Current() const {
    if (Done()) return "";
    return std::string(1, m_seq[m_index]);
}

std::string LessonEngine::Remaining() const {
    if (Done()) return "(concluida)";
    return m_seq.substr(m_index);
}