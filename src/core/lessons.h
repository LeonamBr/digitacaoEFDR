#ifndef CURSO_DIGITACAO_LESSONS_H
#define CURSO_DIGITACAO_LESSONS_H

#include <vector>
#include <string>

namespace Lessons {

struct Lesson {
    const char* name;
    const char* seq;  // UTF-8
};

inline const std::vector<Lesson>& All() {
    static const std::vector<Lesson> k = {
        { "Casa esquerda",          "asdf" },
        { "Casa direita",           "jklç" },
        { "Casas juntas",           "asdf jklç" },
        { "Espaço e casas",         "asdf jklç asdf jklç" },
        { "Palavrinhas 1",          "fala sala salsa" },
        { "Palavrinhas 2",          "caca cala calça" }, 
        { "Mão esquerda treino",    "aa ss dd ff as df" },
        { "Mão direita treino",     "jj kk ll çç jk lç" }
    };
    return k;
}

} // namespace Lessons

#endif