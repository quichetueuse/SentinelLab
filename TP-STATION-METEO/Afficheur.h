#include <iostream>
#include <string>

using namespace std;

class Afficheur {
  public:
    virtual void afficher(const string& nom, double valeur, const string& unite) = 0;
    virtual void effacer() = 0;
    virtual ~Afficheur() = default;
};