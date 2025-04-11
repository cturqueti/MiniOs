#ifndef WIFI_ITEMS_H
#define WIFI_ITEMS_H

#include <string>
#include <vector>

struct WiFiItems {
    std::string ssid;
    std::string password;
    bool dhcpFlag;
    std::vector<uint8_t> ip, gateway, subnet;
    bool configLoaded;
    int connectionStatus;
    int power;
    
    WiFiItems();
    void reset();
};

enum class WiFiLog
{
    DISABLE = 0,
    ENABLE
};

#endif // WIFI_ITEMS_H