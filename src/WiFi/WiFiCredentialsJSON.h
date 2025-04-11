#ifndef WIFI_CREDENTIALS_JSON_H
#define WIFI_CREDENTIALS_JSON_H

#include "WiFiItems.h"
#include "WiFiStorageInterface.h"
#include <ArduinoJson.h>
#include <LittleFS.h>
#include <LogLibrary.h>
#include <WiFi.h>

class WiFiCredentialsJSON : public WiFiStorageInterface
{
public:
    explicit WiFiCredentialsJSON(WiFiLog log = WiFiLog::ENABLE) : _log(log) {} // desabilitar depois

    // Operações básicas
    bool saveCredentials(const WiFiItems &config) override;
    WiFiItems loadCredentials() override;
    bool deleteCredentials() override;
    bool configExists() const override;

    // Operações avançadas
    bool createDefaultConfig();
    bool modifyCredentials(const WiFiItems &newConfig);
    // bool loadFile();

private:
    bool writeJsonToFile(const JsonDocument &doc);
    JsonDocument createJsonFromConfig(const WiFiItems &config);
    static constexpr const char *defaultFilePath = "/configWi.json";

    WiFiLog _log;
    WiFiItems _wifi;
};

#endif // WIFI_JSON_H