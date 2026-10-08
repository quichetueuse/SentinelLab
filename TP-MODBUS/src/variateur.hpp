// variateur.hpp : modèle simplifié d'un variateur de vitesse pour moteur.
#pragma once
#include "modbus.hpp"

#include <algorithm>
#include <cmath>
#include <cstdint>
#include <optional>

class Variateur {
public:
  enum Registre : uint16_t {
    CONSIGNE = 0,      // RW  tr/min, 0 à 3000
    VITESSE = 1,       // R   tr/min
    COURANT = 2,       // R   dixièmes d'ampère
    TEMPERATURE = 3,   // R   dixièmes de °C
    ETAT = 4,          // R   bits d'état ci-dessous
    COMMANDE = 5,      // RW  0 arrêt, 1 marche, 2 acquitter le défaut
    NB_REGISTRES = 6
  };
  static constexpr uint16_t BIT_MARCHE = 1u << 0, BIT_DEFAUT = 1u << 1,
                            BIT_SURCHAUFFE = 1u << 2, BIT_CONSIGNE_ATTEINTE = 1u << 3;
  static constexpr uint16_t VITESSE_MAX = 3000;

  std::optional<uint16_t> lire(uint16_t a) const {
    switch (a) {
      case CONSIGNE: return consigne_;
      case VITESSE: return static_cast<uint16_t>(std::lround(vitesse_));
      case COURANT: return static_cast<uint16_t>(std::lround((marche_ ? 15.0 : 0.0) + vitesse_ / 30.0));
      case TEMPERATURE: return static_cast<uint16_t>(std::lround(temperature_ * 10.0));
      case ETAT: return etat();
      case COMMANDE: return static_cast<uint16_t>(marche_ ? 1 : 0);
      default: return std::nullopt;
    }
  }

  std::optional<modbus::CodeException> ecrire(uint16_t a, uint16_t v) {
    using enum modbus::CodeException;   // C++20 : les énumérateurs sans préfixe
    switch (a) {
      case CONSIGNE:
        if (v > VITESSE_MAX) return ValeurIllegale;
        consigne_ = v; 
        return std::nullopt;
      case COMMANDE:
        switch(v) {
          case 0:
            marche_ = false; 
            return std::nullopt;
          case 1:
            if (defaut_) return ValeurIllegale;
            marche_ = true;
            return std::nullopt;
          case 2:
             if (temperature_ > 60) return ValeurIllegale;
             defaut_ = false;
             return std::nullopt;
          default:
            return ValeurIllegale; 
        }
      default:
        return AdresseIllegale;
      }

  }

  // Fait évoluer le moteur pendant dt secondes, par pas de 10 ms.
  void avancer(double dt) {
    for (; dt > 0.0; dt -= PAS) pas(std::min(dt, PAS));
  }

private:
  static constexpr double PAS = 0.01;                   // secondes
  void pas(double dt) {
    const double cible = (marche_ && !defaut_) ? consigne_ : 0.0;
    const double delta = RAMPE * dt;
    vitesse_ += std::clamp(cible - vitesse_, -delta, delta);
    const double equilibre = 25.0 + vitesse_ / 40.0;   // 100 °C à 3000 tr/min
    temperature_ += (equilibre - temperature_) * dt / CONSTANTE_THERMIQUE;
    if (temperature_ > 80.0 && !defaut_) {             // protection thermique
      defaut_ = true;
      marche_ = false;
    }
  }

  static constexpr double RAMPE = 1000.0;               // tr/min par seconde
  static constexpr double CONSTANTE_THERMIQUE = 20.0;   // secondes

  uint16_t etat() const {
    uint16_t e = 0;
    if (marche_) e |= BIT_MARCHE;
    if (defaut_) e |= BIT_DEFAUT;
    if (temperature_ > 80.0) e |= BIT_SURCHAUFFE;
    if (marche_ && std::abs(vitesse_ - consigne_) < 10.0) e |= BIT_CONSIGNE_ATTEINTE;
    return e;
  }

  uint16_t consigne_ = 0;
  double vitesse_ = 0.0;
  double temperature_ = 25.0;
  bool marche_ = false;
  bool defaut_ = false;
};

static_assert(modbus::Equipement<Variateur>, "Variateur doit satisfaire le concept Equipement");
