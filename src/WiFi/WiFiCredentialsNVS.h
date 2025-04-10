#ifndef WIFI_CREDENTIALS_NVS_H
#define WIFI_CREDENTIALS_NVS_H

#include "WiFiItems.h"
#include <Preferences.h>

class WiFiCredentialsNVS
{
private:
    Preferences preferences;
    const char *nvs_namespace = "wifi_creds";

public:
    WiFiCredentialsNVS();
    void begin();
    void end();
    bool saveCredentials(const WiFiItems &config);
    WiFiItems loadCredentials();
    bool updateCredentials(const WiFiItems &config);
    bool clearCredentials();
    bool hasCredentials();
};
#endif // WIFI_CREDENTIALS_NVS_H