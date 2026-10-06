// alloc_guard.cpp : remplace l'operator new global de tout le programme.
#include "alloc_guard.hpp"

#include <atomic>
#include <cstdio>
#include <cstdlib>
#include <new>
#include <unistd.h>

namespace {
std::atomic<std::size_t> g_init{0};
std::atomic<std::size_t> g_regime{0};
std::atomic<bool> g_fini{false};
}  // namespace

void alloc_guard::fin_initialisation() { g_fini.store(true); }
std::size_t alloc_guard::allocations_init() { return g_init.load(); }
std::size_t alloc_guard::allocations_regime() { return g_regime.load(); }

void* operator new(std::size_t n) {
  if (g_fini.load() == false) {
    ++g_init;
  } else {
    ++g_regime;
    std::printf(stderr, "Le programme est initialise, impossible d allouer plus de memoire.");
    std::abort();
  }

  if (void* p = std::malloc(n ? n : 1)) return p;
  throw std::bad_alloc{};
}

void* operator new[](std::size_t n) { return ::operator new(n); }
void operator delete(void* p) noexcept { std::free(p); }
void operator delete(void* p, std::size_t) noexcept { std::free(p); }
void operator delete[](void* p) noexcept { std::free(p); }
void operator delete[](void* p, std::size_t) noexcept { std::free(p); }
