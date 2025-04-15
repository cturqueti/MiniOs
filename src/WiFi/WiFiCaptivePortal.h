#pragma once
// #include "WiFiStorageInterface.h"
#include "WiFiItems.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include <DNSServer.h>
#include <WebServer.h>
#include <WiFi.h>

#define CAPTIVE_PORTAL_SSID "ESP32-Captive-Portal"
#define CAPTIVE_PORTAL_DNS_PORT 53
#define CAPTIVE_PORTAL_TASK_STACK_SIZE 4096

class WiFiCaptivePortal
{
public:
    // WiFiCaptivePortal(WiFiStorageInterface &storage);
    WiFiCaptivePortal();
    ~WiFiCaptivePortal();

    void begin();
    void end();
    bool isRunning() const;

private:
    void _startAP();
    void _setupDNS();
    void _setupServer();
    void _handleClient();
    bool _loadFromLittleFS(const String &path);
    void _sendFile(const String &path);
    String _getContentType(const String &filename);
    static void _serverTask(void *pvParameters);

    DNSServer _dnsServer;
    WebServer _server;
    bool _isRunning;
    TaskHandle_t _serverTaskHandle;
    // WiFiStorageInterface &_storage;
};