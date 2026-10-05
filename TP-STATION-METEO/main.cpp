#include <iostream>
#include "StationMeteo.h"
#include "AfficheurConsole.h"
#include "AfficheurLCD.h"
#include "Capteur.h"
#include "CapteurTemperature.h"
#include "CapteurHumidite.h"
#include "CapteurLuminosite.h"
#include "Alarme.h"
#include <memory>
#include <vector>

using namespace std;

int main() {
  Led led_temp = Led(13);
  Alarme at = Alarme(led_temp, 25);


  AfficheurLCD al = AfficheurLCD();
  AfficheurConsole ac = AfficheurConsole();

  vector<unique_ptr<Capteur>> listeCapteurs = {};
  listeCapteurs.push_back(make_unique<CapteurTemperature>());
  listeCapteurs.push_back(make_unique<CapteurHumidite>());
  listeCapteurs.push_back(make_unique<CapteurLuminosite>());
  StationMeteo sm = StationMeteo(std::move(listeCapteurs), &al, at);

  for (int i = 0; i < 5; i++)
  {
    sm.cycle();
  }
  

  return 0;
}
