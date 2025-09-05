#include "core/ProgressStore.h"
#include <fstream>
#include <sstream>
#include <utility>

ProgressStore::ProgressStore(std::string path) : m_path(std::move(path)) {}

bool ProgressStore::Load() {
    m.clear();
    std::ifstream in(m_path);
    if (!in) return false;

    std::string line;
    while (std::getline(in, line)) {
        if (line.empty() || line[0] == '#') continue;
        std::istringstream ss(line);
        int id, stars, acc, tsec; char c1, c2, c3;
        if ((ss >> id >> c1 >> stars >> c2 >> acc >> c3 >> tsec) && c1==':' && c2==':' && c3==':') {
            m[id] = LessonProgress{stars, acc, tsec};
        }
    }
    return true;
}

bool ProgressStore::Save() const {
    std::ofstream out(m_path, std::ios::trunc);
    if (!out) return false;
    for (const auto& [id, p] : m) {
        out << id << ":" << p.bestStars << ":" << p.bestAcc << ":" << p.bestTimeSec << "\n";
    }
    return true;
}

LessonProgress ProgressStore::Get(int id) const {
    auto it = m.find(id);
    if (it == m.end()) return LessonProgress{};
    return it->second;
}

void ProgressStore::Submit(int id, int stars, int accPct, int timeSec) {
    auto &p = m[id]; // cria se não existir
    if (stars   > p.bestStars)   p.bestStars   = stars;
    if (accPct  > p.bestAcc)     p.bestAcc     = accPct;
    if (timeSec > 0 && (p.bestTimeSec == 0 || timeSec < p.bestTimeSec)) p.bestTimeSec = timeSec;
}
