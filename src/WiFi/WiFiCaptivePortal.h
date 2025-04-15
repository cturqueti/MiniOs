#pragma once
// #include "WiFiStorageInterface.h"
#include "WiFiItems.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include <DNSServer.h>
#include <WebServer.h>
#include <WiFi.h>
#include <LogLibrary.h>
#include <Preferences.h>
#include "ErrorLib.h"
#include <LittleFS.h>

#define CAPTIVE_PORTAL_SSID "ESP32-Captive-Portal"
#define CAPTIVE_PORTAL_DNS_PORT 53
#define CAPTIVE_PORTAL_TASK_STACK_SIZE 4096

class WiFiCaptivePortal
{
public:
    // WiFiCaptivePortal(WiFiStorageInterface &storage);
    static constexpr std::string_view nvs_namespace = "wifi_config";
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
    bool _loadFromLittleFS(const String& path);
    void _sendFile(const String& path);
    String _getContentType(const String& filename);
    static void _serverTask(void* pvParameters);

    bool _beginCredentials();
    bool _saveCredentials(WiFiItems wifi);

    DNSServer _dnsServer;
    WiFiLog _log;
    WebServer _server;
    bool _isRunning;
    TaskHandle_t _serverTaskHandle;
    Preferences _preferences;
    // WiFiStorageInterface &_storage;
};