// demo_bugs.cpp : quatre erreurs mémoire VOLONTAIRES, à observer avec
// AddressSanitizer (demo_bugs_asan) et Valgrind (demo_bugs).
#include <cstdio>
#include <cstring>

static int debordementTas() {
  int* t = new int[8];
  for (int i = 0; i <= 8; ++i) t[i] = i;      // i == 8 : une case de trop
  const int r = t[3];
  delete[] t;
  return r;
}

static int debordementPile(int indice) {
  int t[8] = {};
  t[indice] = 1;                              // indice 8 : hors du tableau
  return t[0];
}

static void fuite() {
  for (int i = 0; i < 10; ++i) {
    char* ligne = new char[32];               // jamais libéré
    std::snprintf(ligne, 32, "mesure %d", i);
    std::puts(ligne);
  }
}

static int utilisationApresLiberation() {
  int* p = new int(42);
  delete p;
  return *p;                                  // lecture d'une zone libérée
}

int main(int argc, char** argv) {
  const char* mode = argc > 1 ? argv[1] : "";
  if (!std::strcmp(mode, "tas")) return debordementTas() == 3 ? 0 : 1;
  if (!std::strcmp(mode, "pile")) { volatile int i = 8; return debordementPile(i); }
  if (!std::strcmp(mode, "fuite")) { fuite(); return 0; }
  if (!std::strcmp(mode, "uaf")) return utilisationApresLiberation() == 42 ? 0 : 1;
  std::fprintf(stderr, "usage : %s tas | pile | fuite | uaf\n", argv[0]);
  return 2;
}
