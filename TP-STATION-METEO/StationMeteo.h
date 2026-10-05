#include <iostream>
#include <string>
#include <vector>
#include <Capteur.h>
#include <AfficheurConsole.h>
#include <Alarme.h>
#include <memory>

using namespace std;

class StationMeteo {
  private:
    vector<unique_ptr<Capteur>> listeCapteurs;
    Afficheur* ptrAfficheur;
    Alarme alarmeTemperature;

  public:
    StationMeteo(vector<unique_ptr<Capteur>> listeCapteurs, Afficheur* ptrAfficheur, Alarme alarmeTemperature): listeCapteurs(std::move(listeCapteurs)), ptrAfficheur(ptrAfficheur), alarmeTemperature(alarmeTemperature) {}

    void ajouterCapteur(unique_ptr<Capteur> capteur) {
      listeCapteurs.push_back(move(capteur));
      cout << "Ajout d'un capteur" << endl;
    }

    void changerAfficheur(Afficheur* a) {
      ptrAfficheur = a;
      cout << "Changement de l'afficheur" << endl;
    }

    void cycle() {
      for(unique_ptr<Capteur>& capteur: listeCapteurs) {
        double valeur_capteur = capteur->lire();
        ptrAfficheur->afficher(capteur->getNom(), valeur_capteur, capteur->getUnite());
        alarmeTemperature.verifier(valeur_capteur);
      }
    }

    const vector<unique_ptr<Capteur>>& getListeCapteurs() const {
      return listeCapteurs;
    }
    Afficheur* getAfficheur() {
      return ptrAfficheur;
    }
    Alarme getAlarme() {
      return alarmeTemperature;
    }
};