#ifndef WIFI_ITEMS_H
#define WIFI_ITEMS_H

#include <IPAddress.h>
#include <string>
#include <vector>

struct WiFiItems {
    std::string ssid;
    std::string password;
    std::string hostname;
    bool dhcpFlag;
    IPAddress ip;
    IPAddress gateway;
    IPAddress subnet;
    bool configLoaded;
    int connectionStatus;
    int power;

    WiFiItems();
    void reset();
};

enum class WiFiLog { DISABLE = 0, ENABLE };

#endif // WIFI_ITEMS_H