#ifndef WIFILIB_H
#define WIFILIB_H
#include "WiFiCredentialsNVS.h"
#include "WiFiItems.h"
#include <Arduino.h>
#include <ArduinoJson.h>
#include <LittleFS.h>
#include <LogLibrary.h>
#include <vector>

// #include "Utils.h"
#include "WiFi.h"
// #include "WiFiAP.h"

class WiFiLib
{
public:
    enum class WiFiLog
    {
        DISABLE = 0,
        ENABLE
    };
    static WiFiLib &getInstance()
    {
        static WiFiLib instance;
        return instance;
    }
    ~WiFiLib();

    void begin(WiFiItems wifi);
    void begin(WiFiItems wifi, WiFiLog log);

    WiFiItems loadConfig(std::string fileAddress);

    bool isCredentials();

    inline bool isConfigLoaded() { return _wifi.configLoaded; }
    inline bool isDhcp() { return _wifi.dhcpFlag; }
    inline bool isSsid() { return _wifi.ssid.size() > 0; }

private:
    WiFiLib();                                    // Construtor privado
    WiFiLib(const WiFiLib &) = delete;            // Previne cópia
    WiFiLib &operator=(const WiFiLib &) = delete; // Previne atribuição

    void connectToWiFi(WiFiItems wifi);
    void WiFiEvent(WiFiEvent_t event);
    void startAP();

    static const char *TAG;
    WiFiItems _wifi;
    WiFiLog _log;
    WiFiCredentialsNVS nvs;
    WiFiItems wifiConfig;
};

#endif // WiFiLib_H