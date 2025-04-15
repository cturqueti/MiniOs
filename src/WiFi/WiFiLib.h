#ifndef WIFILIB_H
#define WIFILIB_H
// #include "WiFiCredentialsNVS.h"
#include "WiFiItems.h"
#include <Arduino.h>
// #include <ArduinoJson.h>
// #include <LittleFS.h>
// #include "WiFiCaptivePortal.h"
#include <LogLibrary.h>
#include <Preferences.h>
#include <vector>

// #include "Utils.h"
#include <WiFi.h>
// #include "WiFiAP.h"

class WiFiLib
{
public:
    static constexpr std::string_view nvs_namespace = "wifi_config";

    static WiFiLib &getInstance(WiFiLog log = WiFiLog::ENABLE)
    {
        static WiFiLib instance(log);
        return instance;
    }
    ~WiFiLib();

    void begin(WiFiItems wifi);
    void begin(WiFiItems wifi, WiFiLog log);

    bool isCredentials();

    inline bool isConfigLoaded() { return _wifi.configLoaded; }
    inline bool isDhcp() { return _wifi.dhcpFlag; }
    inline bool isSsid() { return _wifi.ssid.size() > 0; }

private:
    WiFiLib(WiFiLog log);                         // Construtor privado
    WiFiLib(const WiFiLib &) = delete;            // Previne cópia
    WiFiLib &operator=(const WiFiLib &) = delete; // Previne atribuição

    void connectToWiFi(WiFiItems wifi);
    void WiFiEvent(WiFiEvent_t event);
    void startAP();
    bool _beginCredentials();
    bool _loadCredentials();

    static const char *TAG;
    WiFiItems _wifi;
    WiFiLog _log;
    // WiFiCredentialsNVS nvs;
    WiFiItems _wifiConfig;
    // WiFiCaptivePortal _captivePortal;
    mutable Preferences _preferences;
};

#endif // WiFiLib_H