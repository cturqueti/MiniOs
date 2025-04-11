#ifndef WIFI_CREDENTIALS_NVS_H
#define WIFI_CREDENTIALS_NVS_H

#include "WiFiItems.h"
#include "WiFiStorageInterface.h"
#include <Preferences.h>

class WiFiCredentialsNVS final : public WiFiStorageInterface
{
public:
    WiFiCredentialsNVS();

    // Interface implementation
    bool saveCredentials(const WiFiItems &config) override;
    WiFiItems loadCredentials() override;
    bool deleteCredentials() override;
    bool configExists() const override;

    // Additional methods
    bool updateCredentials(const WiFiItems &config);
    bool clearCredentials();
    bool hasCredentials() const;
    bool initialize(); // Novo método público
    void terminate();  // Novo método público

private:
    bool begin(bool readOnly = false) const;
    void end() const;
    bool getBool(const char *key, bool defaultValue = false) const;
    mutable Preferences preferences;
    static constexpr const char *nvs_namespace = "wifi_credentials";
};
#endif // WIFI_CREDENTIALS_NVS_H