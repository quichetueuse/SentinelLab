// test_modbus.cpp : tests du CRC, du décodage et du traitement des requêtes.
#include "../src/modbus.hpp"
#include "../src/variateur.hpp"

#include <cstdio>
#include <map>
#include <vector>

static int total = 0, echecs = 0;
#define VERIFIER(c) do { ++total; if (!(c)) { ++echecs; std::printf("ÉCHEC ligne %d : %s\n", __LINE__, #c); } } while (0)

// Un faux équipement pour tester traiter() sans le variateur : il suffit qu'il
// satisfasse le concept Equipement.
struct FauxEquipement {
  std::map<uint16_t, uint16_t> regs{{0, 100}, {1, 200}, {2, 300}};
  std::optional<uint16_t> lire(uint16_t a) const {
    if (auto it = regs.find(a); it != regs.end()) return it->second;
    return std::nullopt;
  }
  std::optional<modbus::CodeException> ecrire(uint16_t a, uint16_t v) {
    if (!regs.contains(a)) return modbus::CodeException::AdresseIllegale;
    regs[a] = v;
    return std::nullopt;
  }
};
static_assert(modbus::Equipement<FauxEquipement>);

static std::vector<uint8_t> requete(uint8_t esc, uint8_t f, uint16_t a, uint16_t v) {
  modbus::Trame t;
  t.ajouter(esc); t.ajouter(f); t.ajouter16(a); t.ajouter16(v); t.terminer();
  return {t.octets.begin(), t.octets.begin() + static_cast<long>(t.taille)};
}

static std::vector<uint8_t> enVecteur(const modbus::Trame& t) {
  return {t.octets.begin(), t.octets.begin() + static_cast<long>(t.taille)};
}

int main() {
  using V = std::vector<uint8_t>;
  // CRC : deux exemples de la spécification Modbus
  VERIFIER(modbus::crc16(V{0x01, 0x03, 0x00, 0x00, 0x00, 0x0A}) == 0xCDC5);
  VERIFIER(modbus::crc16(V{0x11, 0x03, 0x00, 0x6B, 0x00, 0x03}) == 0x8776);
  VERIFIER(modbus::TABLE_CRC[1] == 0xC0C1);

  // Décodage
  const auto r = requete(0x11, 0x03, 0x006B, 3);
  VERIFIER(modbus::hex(r) == "11 03 00 6B 00 03 76 87");
  VERIFIER((modbus::decoderRequete(r) ==
            modbus::Requete{.esclave = 0x11, .fonction = 0x03, .adresse = 0x6B, .valeur = 3}));
  auto abimee = r;
  abimee[3] ^= 0x01;
  VERIFIER(!modbus::decoderRequete(abimee).has_value());

  // Traitement sur le faux équipement
  FauxEquipement eq;
  auto rep = modbus::traiter(eq, 0x11, requete(0x11, 0x03, 0, 3));
  VERIFIER(rep && modbus::hex(rep->vue()).starts_with("11 03 06 00 64 00 C8 01 2C"));
  VERIFIER(!modbus::traiter(eq, 0x11, requete(0x12, 0x03, 0, 3)));            // pas pour moi : muet
  rep = modbus::traiter(eq, 0x11, requete(0x11, 0x03, 2, 5));                 // dépasse la carte
  VERIFIER(rep && rep->octets[1] == 0x83 && rep->octets[2] == 0x02);
  rep = modbus::traiter(eq, 0x11, requete(0x11, 0x03, 0, 200));               // quantité > 125
  VERIFIER(rep && rep->octets[1] == 0x83 && rep->octets[2] == 0x03);
  rep = modbus::traiter(eq, 0x11, requete(0x11, 0x06, 1, 999));
  VERIFIER(rep && enVecteur(*rep) == requete(0x11, 0x06, 1, 999) && eq.regs[1] == 999);
  rep = modbus::traiter(eq, 0x11, requete(0x11, 0x2B, 0, 0));                 // fonction inconnue
  VERIFIER(rep && rep->octets[1] == 0xAB && rep->octets[2] == 0x01);
  VERIFIER(rep && modbus::crc16(rep->vue()) == 0);   // propriété : CRC d'une trame complète = 0

  // Variateur
  Variateur v;
  VERIFIER(v.ecrire(Variateur::CONSIGNE, 3500) == modbus::CodeException::ValeurIllegale);
  VERIFIER(!v.ecrire(Variateur::CONSIGNE, 1500));
  VERIFIER(v.ecrire(Variateur::VITESSE, 10) == modbus::CodeException::AdresseIllegale);
  VERIFIER(!v.ecrire(Variateur::COMMANDE, 1));
  v.avancer(1.0);
  VERIFIER(v.lire(Variateur::VITESSE) == 1000);   // rampe de 1000 tr/min par seconde
  v.avancer(1.0);
  VERIFIER(v.lire(Variateur::VITESSE) == 1500);
  VERIFIER((*v.lire(Variateur::ETAT) & Variateur::BIT_CONSIGNE_ATTEINTE) != 0);

  std::printf("%d vérification(s), %d échec(s)\n", total, echecs);
  return echecs ? 1 : 0;
}
