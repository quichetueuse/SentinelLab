// test_unitaires.cpp : tests du ring buffer et des opérations sur les bits.
// Lancés par ctest, compilés avec AddressSanitizer.
#include "../src/gpio.hpp"
#include "../src/ring_buffer.hpp"

#include <cstdio>

GpioRegs gpio_sim{};
static int echecs = 0, total = 0;

#define VERIFIER(cond)                                                     \
  do {                                                                     \
    ++total;                                                               \
    if (!(cond)) { ++echecs; std::printf("ÉCHEC %s:%d  %s\n", __FILE__, __LINE__, #cond); } \
  } while (0)

static void testRingBufferVide() {
  RingBuffer<int, 4> rb;
  int v = -1;
  VERIFIER(rb.empty());
  VERIFIER(rb.size() == 0);
  VERIFIER(!rb.pop(v));
}

static void testRingBufferPleinEtOrdre() {
  RingBuffer<int, 4> rb;
  for (int i = 0; i < 4; ++i) VERIFIER(rb.push(i));
  VERIFIER(rb.full());
  VERIFIER(!rb.push(99));                     // refus, pas d'écrasement
  int v = -1;
  VERIFIER(rb.pop(v) && v == 0);              // premier entré, premier sorti
  VERIFIER(rb.size() == 3);
}

static void testRingBufferTourne() {
  RingBuffer<int, 4> rb;
  int v = 0;
  for (int i = 0; i < 1000; ++i) {            // fait le tour des indices de nombreuses fois
    VERIFIER(rb.push(i));
    VERIFIER(rb.pop(v) && v == i);
  }
  VERIFIER(rb.empty());
}

static void testBits() {
  volatile uint32_t r = 0;
  bit_set(r, 5);
  VERIFIER(r == 0x20u);
  VERIFIER(bit_test(r, 5));
  bit_toggle(r, 0);
  VERIFIER(r == 0x21u);
  bit_clear(r, 5);
  VERIFIER(r == 0x01u);
  VERIFIER(!bit_test(r, 5));
  bit_set(r, 31);
  VERIFIER(bit_test(r, 31));
}

int main() {
  testRingBufferVide();
  testRingBufferPleinEtOrdre();
  testRingBufferTourne();
  testBits();
  std::printf("%d vérification(s), %d échec(s)\n", total, echecs);
  return echecs == 0 ? 0 : 1;
}
