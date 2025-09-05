#ifndef PROGRESSSTORE_H
#define PROGRESSSTORE_H

#include <string>
#include <unordered_map>

struct LessonProgress {
    int bestStars   = 0;  // 0..3
    int bestAcc     = 0;  // 0..100
    int bestTimeSec = 0;  // menor é melhor; 0 = ainda sem tempo salvo
};

class ProgressStore {
public:
    explicit ProgressStore(std::string path = "progress.dat");

    bool Load();                // tenta ler do disco (ignora se não existir)
    bool Save() const;          // grava no disco
    LessonProgress Get(int id) const;

    // atualiza os melhores resultados (mantém o melhor de cada métrica)
    void Submit(int id, int stars, int accPct, int timeSec);

private:
    std::string m_path;
    std::unordered_map<int, LessonProgress> m;
};

#endif