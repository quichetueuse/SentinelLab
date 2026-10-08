// main.cpp
#include "MqttTransport.hpp"
#include <chrono>
#include <thread>
#include <iostream>
#include <random>

int main() {
    MqttTransport tr("eidl/b01/node/1");
    tr.onMessage([](const std::string& topic, const std::string& msg) {
        if (topic.ends_with("/cmd/led"))
            std::cout << "LED : " << msg << std::endl;
    });
    if (!tr.connect()) return 1;

    std::mt19937 rng{42};
    std::uniform_real_distribution<double> temp(18.0, 35.0);
    for (int i = 0; i < 100; ++i) {
        long long ts = std::chrono::duration_cast<std::chrono::milliseconds>(
            std::chrono::system_clock::now().time_since_epoch()).count();
        std::string json = "{\"t\":" + std::to_string(temp(rng)) +
                           ",\"h\":45.2,\"p\":1013.1,\"ts\":" + std::to_string(ts) + "}";
        tr.publish("eidl/b01/node/1/mesures", json);
        std::this_thread::sleep_for(std::chrono::milliseconds(200));
    }
}