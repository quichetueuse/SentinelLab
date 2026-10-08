// MqttTransport.h
#pragma once
#include "ITransport.hpp"
#include <mosquitto.h>

class MqttTransport : public ITransport {
    mosquitto* m_ = nullptr;
    std::string base_;
    std::function<void(const std::string&, const std::string&)> cb_;

    static void on_msg(mosquitto*, void* self, const mosquitto_message* msg) {
        auto* t = static_cast<MqttTransport*>(self);
        if (t->cb_)
            t->cb_(msg->topic, std::string((char*)msg->payload, msg->payloadlen));
    }
public:
    explicit MqttTransport(std::string base) : base_(std::move(base)) {
        mosquitto_lib_init();
        m_ = mosquitto_new("node-1", true, this);
        mosquitto_message_callback_set(m_, on_msg);
    }
    ~MqttTransport() {
        mosquitto_disconnect(m_);
        mosquitto_loop_stop(m_, false);
        mosquitto_destroy(m_);
        mosquitto_lib_cleanup();
    }
    bool connect() override {

        const std::string etat = base_ + "/etat";
        mosquitto_will_set(m_, etat.c_str(), 7, "offline", 1, true);

        if (mosquitto_connect(m_, "localhost", 1883, 30) != MOSQ_ERR_SUCCESS) return false;
        mosquitto_loop_start(m_);

        publish(etat, "online", 1, true); 
        const std::string led = base_ + "/cmd/led";
        mosquitto_subscribe(m_, nullptr, led.c_str(), 1);
        return true;
    }
    void publish(const std::string& topic, const std::string& p,
             int qos = 1, bool retain = false) override {
      mosquitto_publish(m_, nullptr, topic.c_str(), (int)p.size(), p.data(), qos, retain);
    }
    void onMessage(std::function<void(const std::string&, const std::string&)> cb) override {
        cb_ = std::move(cb);
    }
};