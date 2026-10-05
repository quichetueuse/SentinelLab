#include <iostream>
#include <string>
#include <Capteur.h>
#include <random>

using namespace std;

class CapteurHumidite : public Capteur {

  public:
    CapteurHumidite() : Capteur("Capteur d'humidité", "%", 0) {}

    double lire() override {
      std::random_device rd;
      std::mt19937 gen(rd());
      std::uniform_int_distribution<int> distrib(30, 90); 
      double random_temperature = distrib(gen);
      return random_temperature;
    }

};