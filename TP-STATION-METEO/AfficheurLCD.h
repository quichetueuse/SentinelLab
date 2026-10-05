#include <iostream>
#include <string>
#include <Afficheur.h>

using namespace std;

class AfficheurLCD : public Afficheur {
  public:
    void afficher(const string& nom, double valeur, const string& unite) override {
      string ligne = "";
      ligne.append("|");
      ligne.append(nom);
      ligne.append(" : ");
      ligne.append(to_string(valeur));
      ligne.append(unite);
      int remaining_space = 16 - ligne.size();
      for(int i = 0; i < remaining_space - 1; i++) {
        ligne.append(" ");
      }
      ligne.append("|");
      
      cout << ligne << endl;
    }
    void effacer() override {
      
    }
};