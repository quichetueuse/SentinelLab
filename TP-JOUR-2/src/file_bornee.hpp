#pragma once
#include <array>
#include <cstddef>
#include <mutex>

template <typename T, std::size_t N>
class FileBornee {
    static_assert(N > 0 && (N & (N - 1)) == 0, "N doit être une puissance de 2");
    std::array<T, N> buf_{};
    std::size_t head_ = 0;
    std::size_t tail_ = 0;
    mutable std::mutex mutex_;

public:
    // Tente d'ajouter un élément sans bloquer (utilisé par l'acquisition)
    bool try_push(const T& v) {
        std::lock_guard<std::mutex> lock(mutex_);
        if (head_ - tail_ == N) {
            return false; // Plein
        }
        buf_[head_ & (N - 1)] = v;
        ++head_;
        return true;
    }

    // Tente de retirer un élément
    bool try_pop(T& out) {
        std::lock_guard<std::mutex> lock(mutex_);
        if (head_ == tail_) {
            return false; // Vide
        }
        out = buf_[tail_ & (N - 1)];
        ++tail_;
        return true;
    }

    std::size_t size() const {
        std::lock_guard<std::mutex> lock(mutex_);
        return head_ - tail_;
    }

    bool empty() const {
        std::lock_guard<std::mutex> lock(mutex_);
        return head_ == tail_;
    }

    static constexpr std::size_t capacity() { return N; }
};