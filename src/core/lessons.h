#ifndef LESSONS_H
#define LESSONS_H

#include <string>
#include <vector>

namespace Lessons {

struct Lesson {
    std::string title;
    std::string text;   // sequência UTF-8 (o App usa .text.c_str())
};

// Carrega (uma única vez) de resources/lessons.json.
// Se o arquivo não existir ou estiver inválido, volta um fallback de 5 lições.
const std::vector<Lesson>& All();

// (Opcional) Força recarregar do disco — útil se você editar o JSON em runtime.
void Reload();

} // namespace Lessons

#endif
