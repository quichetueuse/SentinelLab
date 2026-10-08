// esclave.cpp : variateur Modbus RTU. Lit les requêtes sur stdin, répond sur stdout,
// trace sur stderr. Sur un tube, une trame = un bloc reçu d'un seul read(), comme
// le silence de 3,5 caractères délimite une trame sur une vraie liaison RS-485.
#include "modbus.hpp"
#include "variateur.hpp"

#include <chrono>
#include <cstdio>
#include <cstdlib>
#include <cstring>
#include <random>
#include <unistd.h>

int main(int argc, char** argv) {
  uint8_t adresse = 0x11;
  double bruit = 0.0;          // probabilité de corrompre une réponse
  for (int i = 1; i < argc; ++i) {
    if (!std::strcmp(argv[i], "--adresse") && i + 1 < argc)
      adresse = static_cast<uint8_t>(std::strtoul(argv[++i], nullptr, 0));
    else if (!std::strcmp(argv[i], "--bruit") && i + 1 < argc)
      bruit = std::strtod(argv[++i], nullptr);
    else {
      std::fprintf(stderr, "usage : %s [--adresse N] [--bruit 0..1]\n", argv[0]);
      return 2;
    }
  }

  Variateur variateur;
  std::mt19937 alea(42);                                   // reproductible
  std::uniform_real_distribution<double> tirage(0.0, 1.0);
  auto precedent = std::chrono::steady_clock::now();
  std::fprintf(stderr, "[esclave] variateur prêt à l'adresse 0x%02X, bruit %.0f %%\n",
               adresse, bruit * 100.0);

  std::array<uint8_t, 256> tampon{};
  for (;;) {
    const ssize_t n = read(STDIN_FILENO, tampon.data(), tampon.size());
    if (n <= 0) break;                                     // le maître a fermé le tube
    const std::span<const uint8_t> requete{tampon.data(), static_cast<std::size_t>(n)};

    const auto maintenant = std::chrono::steady_clock::now();
    variateur.avancer(std::chrono::duration<double>(maintenant - precedent).count());
    precedent = maintenant;

    auto reponse = modbus::traiter(variateur, adresse, requete);
    if (!reponse) {
      std::fprintf(stderr, "[esclave] <- %s  (ignorée)\n", modbus::hex(requete).c_str());
      continue;
    }
    if (bruit > 0.0 && reponse->taille > 3 && tirage(alea) < bruit) {
      const std::size_t i = 3 + static_cast<std::size_t>(alea() % (reponse->taille - 3));
      reponse->octets[i] ^= static_cast<uint8_t>(1u << (alea() % 8));   // un bit inversé
    }
    std::fprintf(stderr, "[esclave] <- %s  -> %s\n", modbus::hex(requete).c_str(),
                 modbus::hex(reponse->vue()).c_str());
    if (write(STDOUT_FILENO, reponse->octets.data(), reponse->taille) < 0) break;
  }
  std::fprintf(stderr, "[esclave] fin\n");
  return 0;
}
