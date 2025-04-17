#include "WiFiItems.h"

WiFiItems::WiFiItems()
    : ssid(""), password(""), dhcp(""), mDns(), ip(), gateway(), subnet(), configLoaded(false), connectionStatus(0),
      power(0) {}

void WiFiItems::clear() {
    ssid.clear();
    password.clear();
    dhcp.clear();
    ip = IPAddress();
    gateway = IPAddress();
    subnet = IPAddress();
    mDns.clear();
    configLoaded = false;
    connectionStatus = 0;
    power = 0;
}