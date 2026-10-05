#include <iostream>
#include <thread>
#include <chrono>
#include <string>
#include <cstdint>
#include "SimSensor.h"

using namespace std;

enum class Etat { IDLE, MESURE, ENVOI, ERREUR };

int main(int argc, char* argv[]) {
  SimSensor capteur;
  capteur.begin();

  int panneDebut = -1;
  int panneFin = -1;

  if (argc > 1) {
    string arg = argv[1];
    if (arg.rfind("--panne=", 0) == 0) {
      string range = arg.substr(8);
      size_t colon = range.find(':');
      if (colon != string::npos) {
        panneDebut = stoi(range.substr(0, colon));
        panneFin = stoi(range.substr(colon + 1));
      }
    }
  }

Etat etat = Etat::IDLE;
    Mesure m{};

    for (int t = 0; t <= 12; ++t) {
        // Gestion de l'injection de panne selon le tick 't'
        if (panneDebut != -1 && t >= panneDebut && t < panneFin) {
            capteur.injecterPanne(true);
        } else {
            capteur.injecterPanne(false);
        }

        switch (etat) {
            case Etat::IDLE:
                std::cout << "t=" << t << "   IDLE -> MESURE";
                etat = Etat::MESURE;
                // Pas de break pour enchaîner ou simuler le tick suivant selon la logique
                // Ici on simule l'enchaînement direct par tick :
                [[fallthrough]];

            case Etat::MESURE:
                if (capteur.read(m)) {
                    std::cout << " -> ENVOI \t" << m.temp << " °C\n";
                    etat = Etat::IDLE; // Retour à IDLE après succès
                } else {
                    std::cout << " -> ERREUR \t(capteur muet)\n";
                    etat = Etat::ERREUR;
                }
                break;

            case Etat::ENVOI:
                etat = Etat::IDLE;
                break;

            case Etat::ERREUR:
                std::cout << "t=" << t << "   ERREUR -> IDLE \t(reset)\n";
                etat = Etat::IDLE;
                break;
        }

        std::this_thread::sleep_for(std::chrono::milliseconds(300));
    }

    return 0;
}