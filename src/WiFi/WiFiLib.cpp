#include "WiFiLib.h"

WiFiLib::WiFiLib(WiFiLog log)
{
    _log = log;
    _beginCredentials();
}
WiFiLib::~WiFiLib()
{
}

void WiFiLib::begin()
{
    if (_wifi.configLoaded)
    {
        if (isSsid())
        {
            // Start Wifi
            connectToWiFi(_wifi);
            // modify WiFiIcon Wifiservice
        }
        else
        {
            _captivePortal.begin();
        }
    }
}

bool WiFiLib::isCredentials()
{
    if (!_wifi.configLoaded)
        return false;
    if (_wifi.ssid.empty())
        return false;
    if (_wifi.password.empty())
        return false;
    // Pode adicionar outras verificações como tamanho mínimo da senha, etc.
    return true;
}

void WiFiLib::connectToWiFi(WiFiItems wifi)
{
    if (_log == WiFiLog::ENABLE)
    {
        LOG_DEBUG("[WIFI] Connecting to Wi-Fi...");
        LOG_DEBUG("[WIFI] SSID: %s", wifi.ssid.c_str());
    }
    WiFi.mode(WIFI_STA);
    if (!wifi.dhcpFlag)
    {
        IPAddress staticIP(wifi.ip[0], wifi.ip[1], wifi.ip[2], wifi.ip[3]);
        IPAddress gateway(wifi.gateway[0], wifi.gateway[1], wifi.gateway[2], wifi.gateway[3]);
        IPAddress subnet(wifi.subnet[0], wifi.subnet[1], wifi.subnet[2], wifi.subnet[3]);
        WiFi.config(staticIP, gateway, subnet);
    }

    WiFi.onEvent([this](WiFiEvent_t event, WiFiEventInfo_t info)
                 { this->WiFiEvent(event); });

    WiFi.begin(wifi.ssid.c_str(), wifi.password.c_str());

    int attempts = 0;
    const int maxAttempts = 50;
    printf("Conectando.");
    while (WiFi.status() != WL_CONNECTED && attempts < maxAttempts)
    {
        vTaskDelay(pdMS_TO_TICKS(200));
        printf(".");
        attempts++;
    }

    if (WiFi.status() == WL_CONNECTED)
    {
        if (_log == WiFiLog::ENABLE)
        {
            LOG_INFO("[WIFI] Connected to Wi-Fi!");
            LOG_INFO("[WIFI] IP Address: %s", WiFi.localIP().toString().c_str());
        }
    }
    else
    {
        if (_log == WiFiLog::ENABLE)
        {
            LOG_WARN("[WIFI] Failed to connect to Wi-Fi.");
        }
        WiFi.disconnect(true);
        WiFi.mode(WIFI_OFF); // Reinicia completamente a interface Wi-Fi
    }
    return;
}

void WiFiLib::WiFiEvent(WiFiEvent_t event)
{
    switch (event)
    {
    case ARDUINO_EVENT_WIFI_SCAN_DONE:
        if (_log == WiFiLog::ENABLE)
        {
            LOG_INFO("[WIFI] Wi-Fi scan done");
        }
        break;

    case ARDUINO_EVENT_WIFI_STA_START:
        if (_log == WiFiLog::ENABLE)
        {
            LOG_INFO("[WIFI] Wi-Fi STA started");
        }
        break;

    case ARDUINO_EVENT_WIFI_STA_STOP:
        if (_log == WiFiLog::ENABLE)
        {
            LOG_INFO("[WIFI] Wi-Fi STA stopped");
        }
        break;

    case ARDUINO_EVENT_WIFI_STA_CONNECTED:
        if (_log == WiFiLog::ENABLE)
        {
            LOG_INFO("[WIFI] Wi-Fi STA connected");
        }
        _wifi.connectionStatus = ARDUINO_EVENT_WIFI_STA_CONNECTED;
        // display.modifyIconOnTray("WiFi", wifiConnectedIcon, sizeof(wifiConnectedIcon));
        break;

    case ARDUINO_EVENT_WIFI_STA_DISCONNECTED:
        if (_log == WiFiLog::ENABLE)
        {
            LOG_INFO("[WIFI] Wi-Fi STA disconnected");
        }
        _wifi.connectionStatus = ARDUINO_EVENT_WIFI_STA_DISCONNECTED;
        // display.modifyIconOnTray("WiFi", wifiDisconnectedIcon, sizeof(wifiDisconnectedIcon));
        break;

    default:
        if (_log == WiFiLog::ENABLE)
        {
            LOG_INFO("[WIFI] Unhandled Wi-Fi event: %d", event);
        }
        break;
    }
}

void WiFiLib::startAP()
{
    // _captivePortal.begin();
}

bool WiFiLib::_beginCredentials()
{
    if (_preferences.begin(nvs_namespace.data(), true))
    {
        return true;
    }
    if (_log == WiFiLog::ENABLE)
    {
        LOG_ERROR("[WiFi] Error on load NVS");
    }
    ERRORS_LIST.addError(ErrorCode::NVS_BEGIN_ERROR);
    return false;
}

bool WiFiLib::_loadCredentials()
{
    _beginCredentials();
    _wifiConfig.ssid = _preferences.getString("ssid", "").c_str();
    _wifiConfig.password = _preferences.getString("password", "").c_str();
    _preferences.end();
    if (_wifiConfig.ssid != "")
    {
        if (_log == WiFiLog::ENABLE)
        {
            LOG_INFO("[WiFi] Loaded SSID: %s", _wifiConfig.ssid);
        }
        return true;
    }
    if (_log == WiFiLog::ENABLE)
    {
        LOG_ERROR("[WiFi] Don't have SSID");
    }
    ERRORS_LIST.addError(ErrorCode::SSID_NOT_FOUND);

    return false;
}
