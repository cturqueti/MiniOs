#include "WiFiItems.h"

WiFiItems::WiFiItems()
    : ssid(""), password(""), hostname(""), dhcpFlag(true), ip(), gateway(), subnet(), configLoaded(false),
      connectionStatus(0), power(0) {}

void WiFiItems::reset() {
    ssid.clear();
    password.clear();
    hostname.clear();
    dhcpFlag = true;
    ip = IPAddress();
    gateway = IPAddress();
    subnet = IPAddress();
    configLoaded = false;
    connectionStatus = 0;
    power = 0;
}