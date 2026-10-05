#pragma once;
#include <iostream>
#include <string>
#include "Capteur.h"
#include <random>

using namespace std;

class CapteurTemperature : public Capteur {

  public:
    CapteurTemperature() : Capteur("Capteur de temperature", "C", 0) {}

    double lire() override {
      std::random_device rd;
      std::mt19937 gen(rd());
      std::uniform_real_distribution<double> distrib(15.0, 35.0); 
      double random_temperature = distrib(gen);
      return random_temperature;
    }

};