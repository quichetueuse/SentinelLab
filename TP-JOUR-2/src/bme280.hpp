#pragma once
#include "i2c.hpp"

class Bme280 {
    II2cBus& bus_;
    uint8_t addr_;
public:
    explicit Bme280(II2cBus& bus, uint8_t addr = 0x76) : bus_(bus), addr_(addr) {}

    bool identifier() {
        const uint8_t reg = 0xD0; // registre chip_id
        uint8_t id = 0;
        return bus_.writeRead(addr_, &reg, 1, &id, 1) && id == 0x60;
    }

    bool lireMesure(float& temp) {
        uint8_t reg = 0xF7; // Registre de début des données de mesure (exemple)
        uint8_t data[3] = {0};
        if (!bus_.writeRead(addr_, &reg, 1, data, 3)) return false;
        
        // Conversion simulée en température basée sur les données reçues
        temp = 22.0f + static_cast<float>(data[0]) * 0.01f;
        return true;
    }
};