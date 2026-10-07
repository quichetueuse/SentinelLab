// noeud.cpp : nœud SentinelLab simulé, jour 2.
// Partie A : budget mémoire (historique sans allocation, garde d'allocation, énergie).
// Partie B : GPIO virtuels, « interruptions » par signaux, PWM, chien de garde, arrêt propre.
#include "alloc_guard.hpp"
#include "gpio.hpp"
#include "ring_buffer.hpp"

#include <atomic>
#include <chrono>
#include <cmath>
#include <csignal>
#include <cstdio>
#include <cstdlib>
#include <cstring>
#include <thread>
#include <unistd.h>
#include "i2c.hpp"
#include "bme280.hpp"
#include "file_bornee.hpp"
#include "stats_gigue.hpp"

std::atomic<uint32_t> debordements{0};

using namespace std::chrono;

GpioRegs gpio_sim{};

struct Mesure {
  uint32_t t_ms;
  float temp;
};

void tacheAcquisition(std::stop_token st, Bme280& capteur, FileBornee<Mesure, 16>& file) {
    using namespace std::chrono;
    const auto periode = milliseconds{100};
    auto echeance = steady_clock::now();
    StatsGigue gigue;
    
    while (!st.stop_requested()) {
        echeance += periode;
        std::this_thread::sleep_until(echeance); // instant absolu
        
        auto maintenant = steady_clock::now();
        int64_t retard = duration_cast<microseconds>(maintenant - echeance).count();
        gigue.ajouter(retard);

        Mesure m;
        float temp = 0.0f;
        if (capteur.lireMesure(temp)) {
            m.t_ms = static_cast<uint32_t>(duration_cast<milliseconds>(maintenant.time_since_epoch()).count());
            m.temp = temp;
            if (!file.try_push(m)) {
                debordements.fetch_add(1, std::memory_order_relaxed);
            }
        }
    }
    gigue.afficher("acquisition");
}

// ---------------------------------------------------------------- options
struct Options {
  uint32_t periode_ms = 500;   // période d'acquisition
  uint32_t emission_ms = 50;   // durée simulée d'une émission radio (temps actif)
  uint32_t duree_s = 0;        // 0 = sans fin
  uint32_t bloquer_a = 0;      // tick auquel la boucle se bloque (test du chien de garde)
  bool silencieux = false;     // n'affiche pas une ligne par tick
  bool fuite = false;          // alloue à chaque tick (test de la garde d'allocation)
  bool trace_i2c = false;
  bool binaire = false;
};

static Options lireOptions(int argc, char** argv) {
  Options o;
  for (int i = 1; i < argc; ++i) {
    const char* a = argv[i];
    auto valeur = [&](uint32_t& champ) {
      if (i + 1 < argc) champ = static_cast<uint32_t>(std::strtoul(argv[++i], nullptr, 10));
    };
    if (!std::strcmp(a, "--periode")) valeur(o.periode_ms);
    else if (!std::strcmp(a, "--emission")) valeur(o.emission_ms);
    else if (!std::strcmp(a, "--duree")) valeur(o.duree_s);
    else if (!std::strcmp(a, "--bloquer-a")) valeur(o.bloquer_a);
    else if (!std::strcmp(a, "--silencieux")) o.silencieux = true;
    else if (!std::strcmp(a, "--fuite")) o.fuite = true;
    else if (!std::strcmp(a, "--trace-i2c")) o.trace_i2c = true;
    else if (!std::strcmp(a, "--binaire")) o.binaire = true;  
    else {
      std::fprintf(stderr,
          "usage : %s [--periode ms] [--emission ms] [--duree s] [--bloquer-a tick]\n"
          "          [--silencieux] [--fuite]\n", argv[0]);
      std::exit(2);
    }
  }
  if (o.emission_ms >= o.periode_ms) o.emission_ms = o.periode_ms / 2;
  return o;
}

// ------------------------------------------------- « interruptions » (signaux)
// Seuls des types lock-free ou sig_atomic_t sont touchés dans les handlers.
static_assert(std::atomic<uint32_t>::is_always_lock_free);
static std::atomic<uint32_t> g_appuis{0};          // SIGUSR1 : bouton
static volatile std::sig_atomic_t g_stats = 0;     // SIGUSR2 : afficher les statistiques
static volatile std::sig_atomic_t g_arret = 0;     // SIGTERM, SIGINT : arrêt propre

extern "C" void isr_bouton(int) {
  g_appuis.fetch_add(1, std::memory_order_relaxed);
}
extern "C" void isr_stats(int) { g_stats = 1; }
extern "C" void isr_arret(int) { g_arret = 1; }

static void installer(int sig, void (*handler)(int)) {
  struct sigaction sa {};
  sa.sa_handler = handler;
  sigemptyset(&sa.sa_mask);
  sa.sa_flags = SA_RESTART;
  sigaction(sig, &sa, nullptr);
}

// ------------------------------------------------------------ chien de garde
static std::atomic<uint32_t> g_battement{0};

static void chienDeGarde(std::stop_token st) {
  uint32_t dernier = g_battement.load();
  auto derniereVue = steady_clock::now();
  while (!st.stop_requested()) {
    std::this_thread::sleep_for(milliseconds{200});
    const uint32_t actuel = g_battement.load();
    if (actuel != dernier) {
      dernier = actuel;
      derniereVue = steady_clock::now();
    } else {
        const auto ecoule = duration_cast<milliseconds>(steady_clock::now() - derniereVue).count();
      if (ecoule > 3000) {
        const char msg[] = "Chien de garde ERREUR - blocage de la boucle detecte, arret d urgence ";
        ::write(STDERR_FILENO, msg, sizeof(msg) - 1);
        std::abort();
      }
    }
  }
}

// ---------------------------------------------------------------- capteur
static float simulerTemperature(uint32_t t_ms) {
  static uint32_t graine = 12345;                       // bruit pseudo-aléatoire reproductible
  graine = graine * 1103515245u + 12345u;
  const float bruit = static_cast<float>((graine >> 16) % 41) / 100.0f - 0.2f;
  const float periode = 60000.0f;                       // une oscillation par minute
  return 22.0f + 6.0f * std::sin(6.2831853f * static_cast<float>(t_ms) / periode) + bruit;
}

// -------------------------------------------------------------------- PWM
// Rapport cyclique en % : 0 à 18 °C, 100 à 30 °C, linéaire entre les deux.
static int rapportCyclique(float temp) {
  if (temp <= 18.0f) return 0;
  if (temp >= 30.0f) return 100;
  const float pourcent = (temp - 18.0f) / (30.0f - 18.0f) * 100.0f;

  return static_cast<int>(std::round(pourcent));
}

static void barre(char (&out)[11], int pourcent) {
  const int pleins = pourcent / 10;
  for (int i = 0; i < 10; ++i) out[i] = i < pleins ? '#' : '-';
  out[10] = '\0';
}

// ------------------------------------------------------------ statistiques
struct Stats {
  uint32_t ticks = 0;
  uint32_t appuis = 0;
  double actif_ms = 0.0;
  double ecoule_ms = 0.0;
};

static void afficherStats(const Stats& s, const RingBuffer<Mesure, 64>& h, const Options& o) {
  const double cycle = s.ecoule_ms > 0 ? s.actif_ms / s.ecoule_ms : 0.0;
  // Modèle énergétique (ordres de grandeur ESP32) : voir cours 3.8
  const double i_actif = 100.0, i_veille = 0.010, capacite = 2000.0;  // mA, mA, mAh
  const double i_moy = i_actif * cycle + i_veille * (1.0 - cycle);
  const double heures = i_moy > 0 ? capacite / i_moy : 0.0;
  std::printf("=== statistiques ===\n");
  std::printf("ticks            : %u\n", s.ticks);
  std::printf("allocations      : init %zu, régime %zu\n",
              alloc_guard::allocations_init(), alloc_guard::allocations_regime());
  std::printf("historique       : %zu/%zu mesures", h.size(), h.capacity());
  if (!h.empty()) std::printf(", dernière %.2f °C", static_cast<double>(h.dernier().temp));
  std::printf("\nbouton           : %u appui(s), LED %s (ODR=0x%08X)\n", s.appuis,
              bit_test(GPIO->ODR, PIN_LED) ? "ON" : "off", static_cast<unsigned>(GPIO->ODR));
  std::printf("rapport cyclique : %.1f %% (période %u ms, émission %u ms)\n",
              cycle * 100.0, o.periode_ms, o.emission_ms);
  std::printf("courant moyen    : %.3f mA, autonomie estimée %.0f h (%.1f jours)\n",
              i_moy, heures, heures / 24.0);
  std::fflush(stdout);
}

// ------------------------------------------------------------------- main
int main(int argc, char** argv) {
  const Options opt = lireOptions(argc, argv);
  std::setvbuf(stdout, nullptr, _IOLBF, 0);   // une ligne = un envoi, même dans un tube

  SimI2cBus i2c_bus(opt.trace_i2c);
  Bme280 capteur(i2c_bus, 0x76);

  if (!capteur.identifier()) {
    std::fprintf(stderr, "Le capteur BME280 n'est pas visible sur le bus I2C");
  }

  installer(SIGUSR1, isr_bouton);
  installer(SIGUSR2, isr_stats);
  installer(SIGTERM, isr_arret);
  installer(SIGINT, isr_arret);

  static RingBuffer<Mesure, 64> historique;   // .bss : taille connue à la compilation
  Stats stats;
  std::jthread garde(chienDeGarde);           // le thread alloue : avant la fin de l'init

  if (const char* n = std::getenv("REDEMARRAGES"))
    std::fprintf(stderr, "[noeud] démarrage, redémarrages précédents : %s\n", n);
  std::fprintf(stderr, "[noeud] pid %d, période %u ms. kill -USR1 %d = bouton, -USR2 = stats\n",
               getpid(), opt.periode_ms, getpid());

  // Lance le thread qui gère la récup des données des capteurs en parallele

  FileBornee<Mesure, 16> file_mesures;
  std::jthread acquisition_thread(tacheAcquisition, std::ref(capteur), std::ref(file_mesures));

  alloc_guard::fin_initialisation();          // plus aucune allocation à partir d'ici

  const auto debut = steady_clock::now();
  auto prochain = debut;
  for (uint32_t tick = 1; !g_arret; ++tick) {

    prochain += milliseconds{opt.periode_ms};
    std::this_thread::sleep_until(prochain);
    const auto reveil = steady_clock::now();
    stats.ticks = tick;
    const auto t_ms = static_cast<uint32_t>(duration_cast<milliseconds>(reveil - debut).count());

    // Acquisition et historique (aucune allocation)
    Mesure m;
    if (file_mesures.try_pop(m)) {
      if (!historique.push(m)) {
        Mesure ancienne;
        historique.pop(ancienne);
        historique.push(m);
      }
    }
    else {
      m.t_ms = t_ms;
      m.temp = 22.0f;
    }
    if (opt.fuite) {
      auto* copie = new Mesure(m);            // volontairement fautif : voir étape A2
      (void)copie;
    }

    // Traitement différé de l'« interruption » bouton
    const uint32_t appuis = g_appuis.exchange(0);
    stats.appuis += appuis;
    for (uint32_t i = 0; i < appuis; i++) {
      bit_toggle(GPIO->ODR, PIN_LED);
    }

    const int pwm = rapportCyclique(m.temp);
    if (!opt.silencieux) {
      char b[11];
      barre(b, pwm);
      std::printf("t=%6u ms  T=%5.2f °C  LED=%-3s  PWM [%s] %3d %%\n", t_ms,
                  static_cast<double>(m.temp), bit_test(GPIO->ODR, PIN_LED) ? "ON" : "off", b, pwm);
    }
    if (g_stats) {
      g_stats = 0;
      afficherStats(stats, historique, opt);
    }

    // Émission radio simulée : compte comme temps actif
    std::this_thread::sleep_for(milliseconds{opt.emission_ms});
    stats.actif_ms += duration<double, std::milli>(steady_clock::now() - reveil).count();
    stats.ecoule_ms = duration<double, std::milli>(steady_clock::now() - debut).count();

    g_battement.fetch_add(1);                 // « je suis vivant »

    if (opt.bloquer_a && tick == opt.bloquer_a) {
      std::fprintf(stderr, "[noeud] blocage volontaire au tick %u\n", tick);
      for (;;) std::this_thread::sleep_for(seconds{1});   // le chien de garde doit réagir
    }
    if (opt.duree_s && t_ms >= opt.duree_s * 1000u) break;
  }


  afficherStats(stats, historique, opt);
  return 0;
}
