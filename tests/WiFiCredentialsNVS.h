#ifndef WIFI_CREDENTIALS_NVS_H
#define WIFI_CREDENTIALS_NVS_H

#include "WiFiItems.h"
// #include "WiFiStorageInterface.h"
#include <Preferences.h>

class WiFiCredentialsNVS final
{
public:
    WiFiCredentialsNVS();

    // Interface implementation
    bool saveCredentials(const WiFiItems &config);
    WiFiItems loadCredentials();
    bool deleteCredentials();
    bool configExists();

    // Additional methods
    bool updateCredentials(const WiFiItems &config);
    bool clearCredentials();
    bool hasCredentials();
    bool initialize(); // Novo método público
    void terminate();  // Novo método público

private:
    bool begin(bool readOnly = false);
    void end();
    bool getBool(const char *key, bool defaultValue = false);
    mutable Preferences preferences;
    static constexpr const char *nvs_namespace = "wifi_credentials";
};
#endif // WIFI_CREDENTIALS_NVS_H