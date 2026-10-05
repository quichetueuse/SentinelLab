#include <iostream>
#include <string>
#include <Afficheur.h>

using namespace std;

class AfficheurConsole : public Afficheur {
  public:
    void afficher(const string& nom, double valeur, const string& unite) override {
      cout << nom << " : " << valeur << unite << endl;
    }

    void effacer() override {

    }
};