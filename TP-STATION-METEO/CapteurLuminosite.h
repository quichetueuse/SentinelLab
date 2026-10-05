#pragma once;
#include <iostream>
#include <string>
#include "Capteur.h"
#include <random>

using namespace std;

class CapteurLuminosite : public Capteur {

  public:
    CapteurLuminosite() : Capteur("Capteur de luminosite", "lux", 0) {}

    double lire() override {
      std::random_device rd;
      std::mt19937 gen(rd());
      std::uniform_int_distribution<int> distrib(0, 1000); 
      double random_temperature = distrib(gen);
      return random_temperature;
    }

};