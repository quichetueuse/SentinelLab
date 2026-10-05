#include <iostream>
#include <string>

using namespace std;

class Capteur {
  protected:
    string nom;
    string unite;
    double derniereValeur;
    static int nbCapteurs;

  public:
    virtual double lire() = 0;

    Capteur(string nom, string unite, double derniereValeur): nom(nom), unite(unite), derniereValeur(derniereValeur) {
      nbCapteurs++;
    }

    virtual ~Capteur() {
      nbCapteurs--;
    };

    string getNom() {
      return nom;
    }

    string getUnite() {
      return unite;
    }

    double getDerniereValeur() {
      return derniereValeur;
    }

    static int getNbCapteurs() {
      return nbCapteurs;
    }

};