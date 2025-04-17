#ifndef WIFI_ITEMS_H
#define WIFI_ITEMS_H

#include <IPAddress.h>
#include <string>
#include <vector>

struct WiFiItems {
    String ssid;
    String password;
    String dhcp;
    IPAddress ip;
    IPAddress gateway;
    IPAddress subnet;
    String mDns;
    bool configLoaded;
    int connectionStatus;
    int power;

    WiFiItems();
    void clear();
};

struct StaticIPConfig {
    String ip;
    String gateway;
    String subnet;
    String mDns;
};

enum class WiFiLog { DISABLE = 0, ENABLE };

#endif // WIFI_ITEMS_H