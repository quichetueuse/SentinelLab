#pragma once
#include <cstdint>
#include <cstddef>
#include <cstdio>
#include <vector>

// Interface abstraite du bus I2C
class II2cBus {
public:
    virtual ~II2cBus() = default;
    virtual bool writeRead(uint8_t addr, const uint8_t* tx, size_t ntx,
                           uint8_t* rx, size_t nrx) = 0;
};

// Implémentation simulée du bus I2C avec option de traçage
class SimI2cBus : public II2cBus {
    bool trace_;
public:
    explicit SimI2cBus(bool trace = false) : trace_(trace) {}

    bool writeRead(uint8_t addr, const uint8_t* tx, size_t ntx,
                   uint8_t* rx, size_t nrx) override {
        if (trace_) {
            std::printf("I2C S 0x%02X W", addr);
            for (size_t i = 0; i < ntx; ++i) {
                std::printf(" [%02X]", tx[i]);
            }
        }

        // Simulation de la réponse du capteur BME280 à l'adresse 0x76
        if (addr == 0x76 && ntx > 0) {
            uint8_t reg = tx[0];
            if (reg == 0xD0) { // Registre CHIP_ID
                if (nrx > 0) {
                    rx[0] = 0x60; // ID standard du BME280
                }
            } else {
                // Autres registres de mesure simulés (ex: température)
                for (size_t i = 0; i < nrx; ++i) {
                    rx[i] = static_cast<uint8_t>(i + 42); 
                }
            }
        }

        if (trace_) {
            if (nrx > 0) {
                std::printf(" Sr 0x%02X R", addr);
                for (size_t i = 0; i < nrx; ++i) {
                    std::printf(" [%02X]", rx[i]);
                }
            }
            std::printf(" P\n");
            std::fflush(stdout);
        }
        return true;
    }
};