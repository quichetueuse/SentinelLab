#include <iostream>
#include <vector>
#include <cstring>
#include <cstdint>

// Fausse structure pour l'exemple
struct Mesure {
    float temperature;
    float humidite;
};

// Fausse fonction de vérification (CRC)
uint8_t crc8([[maybe_unused]] const uint8_t* data, [[maybe_unused]] size_t len) {
    return 0x00; // Simplifié pour le test
}

// LE NOUVEAU PARSEUR SÉCURISÉ (Zéro Confiance)
bool parse(const uint8_t* rx, size_t n, Mesure& out) {
    // 1. Vérifie la taille minimale et le "magic byte" (0xAA)
    if (n < 4 || rx[0] != 0xAA) return false;
    
    // 2. Récupère la longueur annoncée
    const uint8_t len = rx[1];
    
    // 3. Vérifie que la longueur correspond EXACTEMENT à notre structure 
    // et que la trame reçue est complète (len + 1 octet start + 1 octet len + 1 octet CRC = len + 3)
    if (len != sizeof(Mesure) || n != size_t{len} + 3u) return false;
    
    // 4. Vérifie l'intégrité des données
    if (crc8(rx + 2, len) != rx[2 + len]) return false;
    
    // 5. Copie sécurisée (on sait maintenant que 'len' == sizeof(Mesure))
    memcpy(&out, rx + 2, len);
    return true;
}

int main() {
    std::vector<uint8_t> buffer;
    char c;
    while (std::cin.get(c)) {
        buffer.push_back(static_cast<uint8_t>(c));
    }

    if (!buffer.empty()) {
        Mesure m;
        if (parse(buffer.data(), buffer.size(), m)) {
            std::cout << "Trame valide et parsée avec succès !" << std::endl;
        } else {
            std::cerr << "Erreur : Trame invalide ou malveillante rejetée." << std::endl;
        }
    }
    return 0;
}
