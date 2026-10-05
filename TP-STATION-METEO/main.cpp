#include <iostream>
#include <StationMeteo.h>
#include <AfficheurConsole.h>
#include <AfficheurLCD.h>
#include <Capteur.h>
#include <CapteurTemperature.h>
#include <CapteurHumidite.h>
#include <CapteurLuminosite.h>
#include <Alarme.h>
#include <memory>
#include <vector>

using namespace std;

int main() {
  Led led_temp = Led(13);
  Alarme at = Alarme(led_temp, 23);

  CapteurLuminosite cl = CapteurLuminosite();
  CapteurHumidite ch = CapteurHumidite();
  CapteurTemperature ct = CapteurTemperature();

  AfficheurLCD& al = AfficheurLCD();
  AfficheurConsole& ac = AfficheurConsole();

  vector<unique_ptr<Capteur>> listeCapteurs = {};
  StationMeteo sm = StationMeteo(listeCapteurs, al, at);



  return 0;
}
