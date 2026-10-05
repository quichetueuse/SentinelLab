#pragma once
#include "ISensor.h"

class SimSensor final : public ISensor {
    float t_ = 20.f;
    bool panne_ = false;
  public:
    void injecterPanne(bool p) { panne_ = p; }
    bool begin() override { return true; }
    bool read(Mesure& m) override {
      if (panne_) return false;
      t_ += 0.1f;
      m = { t_, 45.0f, 1013.0f, 0 };
      return true;
    }
};