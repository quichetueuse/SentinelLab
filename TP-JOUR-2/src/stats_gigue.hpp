#pragma once
#include <cstdint>
#include <cstdio>
#include <algorithm>

struct StatsGigue {
    uint64_t min_us = UINT64_MAX;
    uint64_t max_us = 0;
    uint64_t somme_us = 0;
    uint64_t count = 0;

    void ajouter(int64_t retard_us) {
        if (retard_us < 0) retard_us = 0;
        uint64_t r = static_cast<uint64_t>(retard_us);
        if (r < min_us) min_us = r;
        if (r > max_us) max_us = r;
        somme_us += r;
        count++;
    }

    void afficher(const char* nom) const {
        uint64_t moy = count > 0 ? somme_us / count : 0;
        std::fprintf(stderr, "%s : gigue min %llu µs  max %llu µs  moy %llu µs\n",
                     nom, static_cast<unsigned long long>(min_us),
                     static_cast<unsigned long long>(max_us),
                     static_cast<unsigned long long>(moy));
    }
};