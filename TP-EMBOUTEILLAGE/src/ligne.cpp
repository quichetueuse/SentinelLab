// ligne.cpp : ligne d'embouteillage, un thread par poste.
//   alimentation -> [convoyeur] -> remplissage -> [convoyeur] -> bouchage
//                -> [convoyeur] -> étiquetage -> [convoyeur] -> contrôle
// Toutes les N bouteilles, la ligne change de format : tous les postes se
// synchronisent sur une barrière, comme une ligne réelle qui s'arrête pour
// changer de moule.
#include "file_bornee.hpp"

#include <atomic>
#include <barrier>
#include <chrono>
#include <cstdio>
#include <cstdlib>
#include <cstring>
#include <format>
#include <iostream>
#include <latch>
#include <poll.h>
#include <string>
#include <string_view>
#include <thread>
#include <unistd.h>
#include <vector>

using namespace std::chrono;
using Horloge = steady_clock;

struct Bouteille {
  uint32_t numero = 0;
  uint32_t lot = 0;
};

constexpr std::size_t CAPACITE = 4;
using Convoyeur = FileBornee<Bouteille, CAPACITE>;

struct Poste {
  std::string_view nom;
  milliseconds duree;
  Convoyeur* amont = nullptr;    // nullptr : l'alimentation crée les bouteilles
  Convoyeur* aval = nullptr;     // nullptr : le contrôle est le dernier poste
  std::atomic<uint32_t> traitees{0};
  std::atomic<int64_t> occupe_us{0}, attente_amont_us{0}, attente_aval_us{0}, attente_format_us{0};
};

struct Options {
  uint32_t lots = 3, taille_lot = 40, cadence_ms = 20, remplissage_ms = 30, urgence_ms = 0;
};

struct Ligne;   // déclaré plus bas : la fonction de fin de lot en a besoin

// Fonction exécutée UNE fois par la barrière quand tous les postes sont arrivés.
struct FinDeLot {
  Ligne* ligne;
  void operator()() noexcept;
};

struct Ligne {
  Options opt;
  std::stop_source arret;                      // partagée par tous les postes
  std::latch pret;                             // tous les postes prêts avant de démarrer
  std::barrier<FinDeLot> changementFormat;
  std::atomic<bool> enPause{false};
  std::atomic<bool> terminee{false};
  std::atomic<uint32_t> lotCourant{1}, rejets{0}, controleesDansLot{0};
  Horloge::time_point debutLot = Horloge::now();
  Horloge::time_point debut = Horloge::now();

  Ligne(const Options& o, std::ptrdiff_t nbPostes)
      : opt(o), pret(nbPostes), changementFormat(nbPostes, FinDeLot{this}) {}
};

void FinDeLot::operator()() noexcept {
  Ligne& l = *ligne;
  const uint32_t n = l.controleesDansLot.exchange(0);
  if (n == 0) return;                          // phase vide (postes sortis) : rien à dire
  if (l.arret.stop_requested()) {              // arrêt d'urgence en cours de lot
    std::cout << std::format("[lot {}] interrompu après {} bouteilles\n", l.lotCourant.load(), n);
    return;
  }
  const auto ms = duration<double, std::milli>(Horloge::now() - l.debutLot).count();
  const double cadence = n * 1000.0 / (ms > 0 ? ms : 1);
  std::cout << std::format("[Lot {}] {} bouteilles en {:.0f} ms - candence de {:.1f} b/s\n", l.lotCourant.load(), n, ms, cadence);
  if (l.lotCourant.fetch_add(1) >= l.opt.lots) {
    l.arret.request_stop();
  }
  l.debutLot = Horloge::now();
}

// Complète s par des espaces jusqu'à w caractères affichés (std::format compte ici
// les octets : « é » en UTF-8 en occupe deux).
static std::string colonne(std::string_view s, std::size_t w) {
  std::size_t n = 0;
  for (const char c : s) if ((static_cast<unsigned char>(c) & 0xC0) != 0x80) ++n;
  return std::string(s) + std::string(n < w ? w - n : 0, ' ');
}

static int64_t us(Horloge::duration d) { return duration_cast<microseconds>(d).count(); }

// Boucle d'un poste. Tous les postes exécutent la même fonction.
void travailler(Poste& p, Ligne& l) {
  const std::stop_token st = l.arret.get_token();
  l.pret.arrive_and_wait();
  uint32_t dansLot = 0, numero = 0;
  while (!st.stop_requested()) {
    l.enPause.wait(true);
    Bouteille b;
    auto t0 = Horloge::now();
    if (p.amont) {
      const auto recue = p.amont->retirer(st);
      if (!recue) break;
      b = *recue;
      p.attente_amont_us += us(Horloge::now() - t0);
    } else {
      b = Bouteille{++numero, l.lotCourant.load()};
    }

    t0 = Horloge::now();
    std::this_thread::sleep_for(p.duree);      // le travail du poste
    p.occupe_us += us(Horloge::now() - t0);

    if (p.aval) {
      t0 = Horloge::now();
      if (!p.aval->deposer(b, st)) break;
      p.attente_aval_us += us(Horloge::now() - t0);
    } else {                                   // contrôle qualité
      if (b.numero % 17 == 0) ++l.rejets;      // défaut simulé, reproductible
      ++l.controleesDansLot;
    }
    ++p.traitees;
    if (++dansLot == l.opt.taille_lot) {
      dansLot = 0;
      t0 = Horloge::now();
      l.changementFormat.arrive_and_wait();    // tous les postes changent de format ensemble
      p.attente_format_us += us(Horloge::now() - t0);
    }
  }
  l.changementFormat.arrive_and_drop();
}

// Console : pause, reprise, urgence, stats. Utilise le stop_token PROPRE du jthread.
void console(std::stop_token st, Ligne& l, const std::vector<Poste*>& postes) {
  std::string ligne;
  while (!st.stop_requested()) {
    pollfd pfd{STDIN_FILENO, POLLIN, 0};
    if (poll(&pfd, 1, 200) <= 0) continue;
    do {                                                   // traiter aussi les lignes déjà en tampon
    if (!std::getline(std::cin, ligne)) return;            // fin de l'entrée
    if (ligne == "pause") { l.enPause = true; std::cout << "[ligne] en pause\n"; }
    else if (ligne == "reprise") { l.enPause = false; l.enPause.notify_all(); std::cout << "[ligne] reprise\n"; }
    else if (ligne == "urgence") l.arret.request_stop();
    else if (ligne == "stats")
      for (const Poste* p : postes) std::cout << "  " << colonne(p->nom, 13) << std::format("{:>5} bouteilles\n", p->traitees.load());
    else std::cout << "  commandes : pause, reprise, urgence, stats\n";
    } while (std::cin.rdbuf()->in_avail() > 0);
  }
}

static Options lireOptions(int argc, char** argv) {
  Options o;
  for (int i = 1; i < argc; ++i) {
    auto valeur = [&](uint32_t& c) { if (i + 1 < argc) c = static_cast<uint32_t>(std::strtoul(argv[++i], nullptr, 10)); };
    const std::string_view a = argv[i];
    if (a == "--lots") valeur(o.lots);
    else if (a == "--taille-lot") valeur(o.taille_lot);
    else if (a == "--cadence") valeur(o.cadence_ms);
    else if (a == "--remplissage") valeur(o.remplissage_ms);
    else if (a == "--urgence-apres") valeur(o.urgence_ms);
    else {
      std::fprintf(stderr, "usage : %s [--lots N] [--taille-lot N] [--cadence ms] [--remplissage ms] [--urgence-apres ms]\n", argv[0]);
      std::exit(2);
    }
  }
  return o;
}

int main(int argc, char** argv) {
  const Options opt = lireOptions(argc, argv);
  std::ios::sync_with_stdio(false);            // cin garde son propre tampon : voir console()
  Convoyeur c1, c2, c3, c4;
  Poste alimentation{"alimentation", milliseconds{opt.cadence_ms}, nullptr, &c1};
  Poste remplissage{"remplissage", milliseconds{opt.remplissage_ms}, &c1, &c2};
  Poste bouchage{"bouchage", milliseconds{20}, &c2, &c3};
  Poste etiquetage{"étiquetage", milliseconds{25}, &c3, &c4};
  Poste controle{"contrôle", milliseconds{10}, &c4, nullptr};
  const std::vector<Poste*> postes{&alimentation, &remplissage, &bouchage, &etiquetage, &controle};

  Ligne l(opt, static_cast<std::ptrdiff_t>(postes.size()));
  // Exécuté par le thread qui demande l'arrêt, quel qu'il soit.
  std::stop_callback quandArret(l.arret.get_token(), [&l] {
    l.enPause = false;                         // débloque les postes en pause
    l.enPause.notify_all();
    l.terminee = true;
    l.terminee.notify_all();
  });

  std::cout << std::format("ligne : {} lots de {} bouteilles, cadence {} ms, remplissage {} ms\n",
                           opt.lots, opt.taille_lot, opt.cadence_ms, opt.remplissage_ms);
  std::jthread clavier(console, std::ref(l), std::cref(postes));
  {
    std::vector<std::jthread> threads;
    for (Poste* p : postes) threads.emplace_back(travailler, std::ref(*p), std::ref(l));
    if (opt.urgence_ms) {
      std::this_thread::sleep_for(milliseconds{opt.urgence_ms});
      std::cout << "[ligne] ARRÊT D'URGENCE\n";
      l.arret.request_stop();
    }
    l.terminee.wait(false);                    // attend la fin sans consommer de CPU
  }                                            // les jthread se rejoignent ici (RAII)
  clavier.request_stop();

  const auto total = duration<double>(Horloge::now() - l.debut).count();
  std::cout << "\nposte         traitées  occupé  attente amont  attente aval  changement format\n";
  for (const Poste* p : postes) {
    auto pc = [&](int64_t v) { return 100.0 * static_cast<double>(v) / (total * 1e6); };
    std::cout << colonne(p->nom, 13) << std::format("{:>9} {:>6.0f} % {:>12.0f} % {:>11.0f} % {:>16.0f} %\n",
                 p->traitees.load(), pc(p->occupe_us), pc(p->attente_amont_us), pc(p->attente_aval_us),
                 pc(p->attente_format_us));
  }
  std::cout << std::format("durée {:.2f} s, {} bouteilles contrôlées, {} rejetées\n", total,
                           controle.traitees.load(), l.rejets.load());
  return 0;
}
