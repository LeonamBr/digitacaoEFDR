#ifndef LESSON_ENGINE_H
#define LESSON_ENGINE_H

#include <string>

class LessonEngine {
    public:
        void LoadSequence(const std::string& seq);
        bool TryConsume(const std::string& ch); // retorna true quando o char esperado é acertado
        bool Done() const;

        std::string Current() const; // caractere atual
        std::string Remaining() const; // sequência restante (inclui o atual)

    private:
        std::string m_seq; // em UTF-8 simples (ex.: "asdf jklç")
        size_t m_index = 0;
};

#endif