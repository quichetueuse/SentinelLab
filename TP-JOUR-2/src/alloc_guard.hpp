// alloc_guard.hpp : compte les allocations dynamiques et interdit toute
// allocation une fois l'initialisation terminée (règle des systèmes critiques).
#pragma once
#include <cstddef>

namespace alloc_guard {
void fin_initialisation();          // à appeler juste avant la boucle principale
std::size_t allocations_init();     // nombre d'appels à new avant fin_initialisation()
std::size_t allocations_regime();   // nombre d'appels à new après (doit rester 0)
}  // namespace alloc_guard
