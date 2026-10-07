// ring_buffer.hpp : file circulaire à taille fixe, sans allocation dynamique.
// Toute la mémoire est réservée dans l'objet lui-même (std::array).
#pragma once
#include <array>
#include <cstddef>

template <typename T, std::size_t N>
class RingBuffer {
  static_assert(N > 0 && (N & (N - 1)) == 0, "N doit être une puissance de 2");
  std::array<T, N> buf_{};
  std::size_t head_ = 0;   // nombre total d'éléments écrits
  std::size_t tail_ = 0;   // nombre total d'éléments lus

public:
  // Ajoute v. Renvoie false si la file est pleine (aucun écrasement silencieux).
  bool push(const T& v) {
    if (full()) return false;
    buf_[head_ & (N - 1)] = v;
    ++head_;
    return true;
  }

  // Retire l'élément le plus ancien dans out. Renvoie false si la file est vide.
  bool pop(T& out) {
    // TODO A1 : refuser si vide, sinon lire à l'indice tail_ modulo N puis avancer tail_
    if (empty()) return false;
    out = buf_[tail_ & (N - 1)];
    ++tail_;
    return true;
  }

  // Élément le plus récent (la file ne doit pas être vide).
  const T& dernier() const { return buf_[(head_ - 1) & (N - 1)]; }

  // TODO A1 : écrire empty(), full() et size() à partir de head_ et tail_
  bool empty() const { return head_ == tail_; }
  bool full() const { return head_ - tail_ == N; }
  std::size_t size() const { return head_ - tail_; }
  static constexpr std::size_t capacity() { return N; }
};
