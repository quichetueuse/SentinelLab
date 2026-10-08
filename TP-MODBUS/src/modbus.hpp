// modbus.hpp : trames Modbus RTU, côté esclave. Fonctions 0x03 et 0x06.
//   requête  : [esclave][fonction][adresse hi lo][valeur ou quantité hi lo][CRC lo hi]
//   réponse 03 : [esclave][03][nb octets][registres hi lo ...][CRC lo hi]
//   réponse 06 : écho de la requête
//   exception  : [esclave][fonction | 0x80][code][CRC lo hi]
#pragma once
#include "crc16.hpp"

#include <array>
#include <concepts>
#include <cstdint>
#include <format>
#include <optional>
#include <span>
#include <string>

namespace modbus {

enum class CodeException : uint8_t {
  FonctionIllegale = 0x01,
  AdresseIllegale = 0x02,
  ValeurIllegale = 0x03,
};

constexpr uint8_t LIRE_REGISTRES = 0x03;
constexpr uint8_t ECRIRE_REGISTRE = 0x06;
constexpr uint16_t QUANTITE_MAX = 125;   // limite de la spécification pour 0x03

// Un équipement est tout type qui sait lire et écrire un registre de 16 bits.
// Le concept remplace une classe de base virtuelle : vérifié à la compilation, sans vtable.
template <typename E>
concept Equipement = requires(E e, const E ce, uint16_t adresse, uint16_t valeur) {
  { ce.lire(adresse) } -> std::same_as<std::optional<uint16_t>>;
  { e.ecrire(adresse, valeur) } -> std::same_as<std::optional<CodeException>>;
};

struct Requete {
  uint8_t esclave;
  uint8_t fonction;
  uint16_t adresse;
  uint16_t valeur;   // quantité pour 0x03, valeur pour 0x06
  bool operator==(const Requete&) const = default;   // comparaison générée (C++20)
};

constexpr uint16_t lire16(std::span<const uint8_t> o, std::size_t i) {
  return static_cast<uint16_t>(o[i] << 8 | o[i + 1]);   // Modbus : poids fort d'abord
}

// Trame de taille fixe maximale, sans allocation dynamique.
struct Trame {
  std::array<uint8_t, 256> octets{};
  std::size_t taille = 0;

  void ajouter(uint8_t o) { octets[taille++] = o; }
  void ajouter16(uint16_t v) {
    ajouter(static_cast<uint8_t>(v >> 8));
    ajouter(static_cast<uint8_t>(v & 0xFF));
  }
  void terminer() {                                   // CRC : poids FAIBLE d'abord
    const uint16_t c = crc16(vue());
    ajouter(static_cast<uint8_t>(c & 0xFF));
    ajouter(static_cast<uint8_t>(c >> 8));
  }
  std::span<const uint8_t> vue() const { return {octets.data(), taille}; }
};

inline std::string hex(std::span<const uint8_t> o) {
  std::string s;
  for (const uint8_t b : o) s += std::format("{:02X} ", b);
  if (!s.empty()) s.pop_back();
  return s;
}

// Décode une requête de 8 octets. nullopt si la taille ou le CRC sont faux.
constexpr std::optional<Requete> decoderRequete(std::span<const uint8_t> t) {
  if (t.size() != 8) return std::nullopt;
  const uint16_t recu = static_cast<uint16_t>(t[6] | t[7] << 8);
  if (crc16(t.first(6)) != recu) return std::nullopt;
  return Requete{.esclave = t[0], .fonction = t[1], .adresse = lire16(t, 2), .valeur = lire16(t, 4)};
}

inline Trame exception(uint8_t esclave, uint8_t fonction, CodeException code) {
  Trame r;
  r.ajouter(esclave);
  r.ajouter(static_cast<uint8_t>(fonction | 0x80));
  r.ajouter(static_cast<uint8_t>(code));
  r.terminer();
  return r;
}

// Traite une requête brute pour l'équipement eq. Renvoie la réponse, ou nullopt
// si l'esclave doit rester muet (autre adresse, trame invalide) : règle Modbus.
template <Equipement E>
std::optional<Trame> traiter(E& eq, uint8_t monAdresse, std::span<const uint8_t> brute) {
  const auto req = decoderRequete(brute);
  if (!req || req->esclave != monAdresse) return std::nullopt;

  switch (req->fonction) {
    case LIRE_REGISTRES: {
      const uint16_t n = req->valeur;
      if (n == 0 || n > QUANTITE_MAX)
        return exception(monAdresse, req->fonction, CodeException::ValeurIllegale);
      Trame r;
      r.ajouter(monAdresse); r.ajouter(LIRE_REGISTRES); r.ajouter(static_cast<uint8_t>(2 * n));
      for (uint32_t i = 0; i < n; ++i) {
          const uint32_t a = static_cast<uint32_t>(req->adresse) + i;
          const auto v = a <= 0xFFFF ? eq.lire(static_cast<uint16_t>(a)) : std::nullopt;
          if (!v) return exception(monAdresse, req->fonction, CodeException::AdresseIllegale);
          r.ajouter16(*v);
      }
      r.terminer();
        return r;
      return exception(monAdresse, req->fonction, CodeException::FonctionIllegale);
    }
    case ECRIRE_REGISTRE: {
      if (const auto refus = eq.ecrire(req->adresse, req->valeur))
        return exception(monAdresse, req->fonction, *refus);
      Trame r;
      for (std::size_t i = 0; i < 6; ++i) r.ajouter(brute[i]);
      r.terminer();
      return r;
    }
    default:
      return exception(monAdresse, req->fonction, CodeException::FonctionIllegale);
  }
}

}  // namespace modbus
