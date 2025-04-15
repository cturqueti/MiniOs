#pragma once
// #include "WiFiStorageInterface.h"
#include "ErrorLib.h"
#include "WiFiItems.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include <ArduinoJson.h>
#include <DNSServer.h>
#include <LittleFS.h>
#include <LogLibrary.h>
#include <Preferences.h>
#include <WebServer.h>
#include <WiFi.h>

#define CAPTIVE_PORTAL_SSID "ESP32-Captive-Portal"
#define CAPTIVE_PORTAL_DNS_PORT 53
#define CAPTIVE_PORTAL_TASK_STACK_SIZE 4096

class WiFiCaptivePortal {
  public:
    // WiFiCaptivePortal(WiFiStorageInterface &storage);
    static constexpr std::string_view nvs_namespace = "wifi_config";
    static constexpr std::string_view captivePortalFolder = "/CaptivePortal";
    WiFiCaptivePortal(WiFiLog log = WiFiLog::ENABLE);
    ~WiFiCaptivePortal();

    void begin();
    void end();
    bool isRunning() const;

  private:
    void _startAP();
    void _setupDNS();
    void _setupServer();
    void _handleClient();
    String _loadFromLittleFS(const String &path);
    String _getContentType(const String &filename);
    static void _serverTask(void *pvParameters);

    bool _beginCredentials();
    bool _saveCredentials(WiFiItems wifi);

    void _handleRoot();
    void _handleConfig();

    DNSServer _dnsServer;
    WiFiLog _log;
    WebServer _server;
    bool _isRunning;
    TaskHandle_t _serverTaskHandle;
    Preferences _preferences;
    // WiFiStorageInterface &_storage;
};