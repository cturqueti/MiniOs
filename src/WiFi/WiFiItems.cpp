#include "WiFiItems.h"

WiFiItems::WiFiItems() : ip(4),
                         gateway(4),
                         subnet(4),
                         configLoaded(false),
                         connectionStatus(0),
                         power(0)
{
}

void WiFiItems::reset()
{
    ssid.clear();
    password.clear();
    configLoaded = false;
    connectionStatus = 0;
}