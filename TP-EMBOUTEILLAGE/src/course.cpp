// course.cpp : quatre threads incrémentent un compteur commun dix millions de fois
// chacun. Trois versions : sans protection, std::atomic_ref, std::mutex.
#include <atomic>
#include <chrono>
#include <cstdio>
#include <format>
#include <iostream>
#include <mutex>
#include <thread>
#include <vector>

constexpr int THREADS = 4, TOURS = 10'000'000;

template <typename F>
void mesurer(const char* nom, F&& incrementer, const int& compteur) {
  const auto t0 = std::chrono::steady_clock::now();
  {
    std::vector<std::jthread> equipe;
    for (int t = 0; t < THREADS; ++t)
      equipe.emplace_back([&] { for (int i = 0; i < TOURS; ++i) incrementer(); });
  }   // jthread : join automatique en sortie de bloc
  const auto ms = std::chrono::duration<double, std::milli>(std::chrono::steady_clock::now() - t0).count();
  std::cout << std::format("{:<16} {:>9} / {}  ({:>6.1f} ms)\n", nom, compteur, THREADS * TOURS, ms);
}

int main() {
  // 1) Sans protection. volatile force chaque lecture et écriture en mémoire
  //    (rappel du jour 2 : volatile n'apporte AUCUNE atomicité).
  int a = 0;
  volatile int& brut = a;
  mesurer("sans protection", [&] { brut = brut + 1; }, a);

  // 2) std::atomic_ref (C++20) : opérations atomiques sur un int ordinaire.
  int b = 0;
  mesurer("atomic_ref", [&] { std::atomic_ref<int>(b).fetch_add(1, std::memory_order_relaxed); }, b);

  // 3) Section critique protégée par un mutex.
  int c = 0;
  std::mutex m;
  mesurer("mutex", [&] { std::scoped_lock lk(m); ++c; }, c);
  return 0;
}
