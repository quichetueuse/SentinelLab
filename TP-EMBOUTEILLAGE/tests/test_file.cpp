// test_file.cpp : tests de FileBornee, compilés avec ThreadSanitizer.
#include "../src/file_bornee.hpp"

#include <atomic>
#include <chrono>
#include <cstdio>
#include <thread>
#include <vector>

static int total = 0, echecs = 0;
#define VERIFIER(c) do { ++total; if (!(c)) { ++echecs; std::printf("ÉCHEC ligne %d : %s\n", __LINE__, #c); } } while (0)

int main() {
  using namespace std::chrono;
  std::stop_source jamais;

  {   // ordre FIFO et capacité
    FileBornee<int, 4> f;
    for (int i = 1; i <= 4; ++i) VERIFIER(f.deposer(i, jamais.get_token()));
    for (int i = 1; i <= 4; ++i) VERIFIER(f.retirer(jamais.get_token()) == i);
  }
  {   // file pleine : deposer reste bloqué jusqu'à l'arrêt
    FileBornee<int, 2> f;
    std::stop_source arret;
    f.deposer(1, arret.get_token());
    f.deposer(2, arret.get_token());
    std::jthread arreteur([&] { std::this_thread::sleep_for(100ms); arret.request_stop(); });
    const auto t0 = steady_clock::now();
    VERIFIER(!f.deposer(3, arret.get_token()));
    VERIFIER(steady_clock::now() - t0 >= 90ms);
  }
  {   // file vide : retirer rend nullopt à l'arrêt
    FileBornee<int, 2> f;
    std::stop_source arret;
    arret.request_stop();
    VERIFIER(!f.retirer(arret.get_token()).has_value());
  }
  {   // 3 producteurs, 2 consommateurs, 30 000 éléments : rien perdu, rien dupliqué
    FileBornee<int, 8> f;
    std::atomic<long> somme{0};
    std::atomic<int> recus{0};
    std::stop_source arret;
    {
      std::vector<std::jthread> equipe;
      for (int p = 0; p < 3; ++p)
        equipe.emplace_back([&] { for (int i = 1; i <= 10000; ++i) f.deposer(i, arret.get_token()); });
      for (int c = 0; c < 2; ++c)
        equipe.emplace_back([&] {
          while (recus < 30000)
            if (auto v = f.retirer(arret.get_token())) { somme += *v; ++recus; }
            else return;
        });
      const auto limite = steady_clock::now() + 5s;          // pas de blocage si la file est fausse
      while (recus < 30000 && steady_clock::now() < limite) std::this_thread::sleep_for(10ms);
      arret.request_stop();
    }
    VERIFIER(recus == 30000);
    VERIFIER(somme == 3L * 10000 * 10001 / 2);
  }
  std::printf("%d vérification(s), %d échec(s)\n", total, echecs);
  return echecs ? 1 : 0;
}
