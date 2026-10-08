// crc16.hpp : CRC-16/MODBUS (polynôme 0xA001 réfléchi, valeur initiale 0xFFFF).
// La table de 256 entrées est calculée PAR LE COMPILATEUR (consteval) :
// elle est rangée en mémoire morte, aucun calcul au démarrage.
#pragma once
#include <array>
#include <cstdint>
#include <span>

namespace modbus {

consteval std::array<uint16_t, 256> tableCrc() {
  std::array<uint16_t, 256> t{};
  for (uint32_t i = 0; i < 256; i++) {
    uint16_t crc = static_cast<uint16_t>(i);
    for (int bit = 0; bit < 8; ++bit) {
      crc = (crc & 1) ? static_cast<uint16_t>((crc >> 1) ^ 0xA001) : static_cast<uint16_t>(crc >> 1);
    }
    t[i] = crc;
  }
  return t;
}

inline constexpr std::array<uint16_t, 256> TABLE_CRC = tableCrc();

// CRC d'une suite d'octets. constexpr : utilisable aussi à la compilation.
constexpr uint16_t crc16(std::span<const uint8_t> octets) {
  uint16_t crc = 0xFFFF;
  for (const uint8_t o: octets) {
    crc = static_cast<uint16_t>((crc >> 8) ^ TABLE_CRC[(crc ^ o) & 0xFF]);
  }
  return crc;
}

inline constexpr std::array<uint8_t, 6> EXEMPLE_SPEC(0x01, 0x03, 0x00, 0x00, 0x00, 0x0A);
static_assert(crc16(EXEMPLE_SPEC) == 0xCDC5, "CRC16/MODBUS: ...");

}  // namespace modbus
