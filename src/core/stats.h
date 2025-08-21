#ifndef STATS_H
#define STATS_H

#include <cstdint>

struct Stats {
    uint64_t hits = 0;
    uint64_t misses = 0;

    double Accuracy() const {
        const double tot = double(hits + misses);
        return tot > 0.0 ? double(hits)/tot : 1.0;
    }
};

#endif