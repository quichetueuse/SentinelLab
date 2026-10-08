// ITransport.h
#pragma once
#include <string>
#include <functional>
struct ITransport {
    virtual ~ITransport() = default;
    virtual bool connect() = 0;
    virtual void publish(const std::string& topic, const std::string& payload,
                         int qos = 1, bool retain = false) = 0;
    virtual void onMessage(std::function<void(const std::string&, const std::string&)> cb) = 0;
};