#pragma once;
#include <iostream>
#include <string>
#include "Led.h"

using namespace std;

class Alarme {
  private:
    Led led;
    double seuil;
  public:
    Alarme(Led led, double seuil): led(led), seuil(seuil) {}
    void verifier(double valeur) {
      if (valeur < seuil) {
        led.allumer();
      } else {
        led.eteindre();
      }
    }
    Led getLed() {
      return led;
    }
    double getSeuil() {
      return seuil;
    }
};