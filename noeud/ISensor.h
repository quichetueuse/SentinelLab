#pragma once
#include <cstdint>

struct Mesure {
  float temp;
  float hum;
  float press;
  uint32_t t_ms;
};

class ISensor {
  public:
    virtual ~ISensor() = default;
    virtual bool begin() = 0;
    virtual bool read(Mesure& out) = 0;
};