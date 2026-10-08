// file_bornee.hpp : convoyeur entre deux postes. File de capacité N fixe,
// producteurs et consommateurs multiples, interruptible par un stop_token.
// Schéma classique à deux sémaphores : places libres et bouteilles présentes.
#pragma once
#include <array>
#include <chrono>
#include <cstddef>
#include <mutex>
#include <optional>
#include <semaphore>
#include <stop_token>

template <typename T, std::size_t N>
class FileBornee {
  std::array<T, N> buf_{};
  std::size_t tete_ = 0, queue_ = 0;
  std::mutex verrou_;                            // protège buf_, tete_, queue_
  std::counting_semaphore<N> places_{N};         // places libres
  std::counting_semaphore<N> presents_{0};       // éléments disponibles

  // Attend un jeton du sémaphore par tranches de 20 ms pour rester interruptible.
  static bool acquerir(std::counting_semaphore<N>& s, std::stop_token st) {
    while (!s.try_acquire_for(std::chrono::milliseconds{20}))
      if (st.stop_requested()) return false;
    return true;
  }

public:
  // Dépose v ; bloque tant que la file est pleine. false si l'arrêt est demandé.
  bool deposer(const T& v, std::stop_token st) {
    if (!acquerir(places_, st)) return false;
    {
      std::scoped_lock lk(verrou_);
      buf_[queue_] = v;
      queue_ = (queue_ + 1) % N;
    }
    presents_.release();
    return true;
  }

  // Retire l'élément le plus ancien ; bloque tant que la file est vide.
  std::optional<T> retirer(std::stop_token st) {
    if (!acquerir(presents_, st)) return std::nullopt;
    T v;
    {
      std::scoped_lock lk (verrou_);
      v = buf_[tete_];
      tete_ = (tete_ + 1) % N;
    }
    places_.release();
    return v;
  }
};
