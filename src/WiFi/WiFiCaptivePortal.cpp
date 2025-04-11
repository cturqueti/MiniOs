#include "WiFiCaptivePortal.h"
#include <LittleFS.h>

WiFiCaptivePortal::WiFiCaptivePortal(WiFiStorageInterface &storage)
    : _server(80), _isRunning(false), _serverTaskHandle(NULL), _storage(storage)
{
}

WiFiCaptivePortal::~WiFiCaptivePortal()
{
    end();
}

void WiFiCaptivePortal::begin()
{
    if (_isRunning)
        return;

    _startAP();
    _setupDNS();
    _setupServer();

    xTaskCreate(
        _serverTask,
        "CaptivePortalTask",
        CAPTIVE_PORTAL_TASK_STACK_SIZE,
        this,
        1,
        &_serverTaskHandle);

    _isRunning = true;
}

void WiFiCaptivePortal::end()
{
    if (!_isRunning)
        return;

    _server.stop();
    _dnsServer.stop();
    WiFi.softAPdisconnect(true);

    if (_serverTaskHandle != NULL)
    {
        vTaskDelete(_serverTaskHandle);
        _serverTaskHandle = NULL;
    }

    _isRunning = false;
}

bool WiFiCaptivePortal::isRunning() const
{
    return _isRunning;
}

void WiFiCaptivePortal::_startAP()
{
    WiFi.softAP(CAPTIVE_PORTAL_SSID);
    IPAddress apIP(192, 168, 4, 1);
    WiFi.softAPConfig(apIP, apIP, IPAddress(255, 255, 255, 0));
    Serial.print("AP IP address: ");
    Serial.println(apIP);
}

void WiFiCaptivePortal::_setupDNS()
{
    _dnsServer.start(CAPTIVE_PORTAL_DNS_PORT, "*", WiFi.softAPIP());
}

void WiFiCaptivePortal::_setupServer()
{
    _server.on("/", HTTP_GET, [this]()
               { _sendFile("/index.html"); });

    _server.on("/scan", HTTP_GET, [this]()
               {
        int n = WiFi.scanNetworks();
        String json = "[";
        for (int i = 0; i < n; ++i) {
            if (i) json += ",";
            json += "{";
            json += "\"ssid\":\"" + WiFi.SSID(i) + "\",";
            json += "\"rssi\":" + WiFi.RSSI(i);
            json += "}";
        }
        json += "]";
        _server.send(200, "application/json", json); });

    _server.on("/styles.css", HTTP_GET, [this]()
               { _sendFile("/styles.css"); });
    _server.on("/script.js", HTTP_GET, [this]()
               { _sendFile("/script.js"); });

    _server.on("/connect", HTTP_POST, [this]()
               {
        WiFiItems config;
        config.ssid = _server.arg("ssid").c_str();
        config.password = _server.arg("password").c_str();
        config.dhcpFlag = true; // Ou pegar do formulário se adicionar essa opção
        
        if (_storage.saveCredentials(config)) {
            _server.send(200, "application/json", "{\"status\":\"success\",\"message\":\"Credenciais salvas com sucesso\"}");
        } else {
            _server.send(500, "application/json", "{\"status\":\"error\",\"message\":\"Falha ao salvar credenciais\"}");
        }
        
        // Opcional: encerrar o portal após conexão
        delay(1000);
        end(); });

    _server.onNotFound([this]()
                       { _sendFile("/index.html"); });

    _server.begin();
}

void WiFiCaptivePortal::_handleClient()
{
    _server.handleClient();
    _dnsServer.processNextRequest();
}

void WiFiCaptivePortal::_serverTask(void *pvParameters)
{
    WiFiCaptivePortal *portal = (WiFiCaptivePortal *)pvParameters;
    while (true)
    {
        portal->_handleClient();
        vTaskDelay(10 / portTICK_PERIOD_MS);
    }
    vTaskDelete(NULL);
}

bool WiFiCaptivePortal::_loadFromLittleFS(const String &path)
{
    String contentType = _getContentType(path);
    File file = LittleFS.open(path, "r");
    if (!file)
        return false;

    _server.streamFile(file, contentType);
    file.close();
    return true;
}

void WiFiCaptivePortal::_sendFile(const String &path)
{
    if (!_loadFromLittleFS(path))
    {
        String message = "File not found\n\n";
        message += "URI: ";
        message += _server.uri();
        message += "\nMethod: ";
        message += (_server.method() == HTTP_GET) ? "GET" : "POST";
        message += "\nArguments: ";
        message += _server.args();
        message += "\n";

        for (uint8_t i = 0; i < _server.args(); i++)
        {
            message += " " + _server.argName(i) + ": " + _server.arg(i) + "\n";
        }

        _server.send(404, "text/plain", message);
    }
}

String WiFiCaptivePortal::_getContentType(const String &filename)
{
    if (filename.endsWith(".html"))
        return "text/html";
    else if (filename.endsWith(".css"))
        return "text/css";
    else if (filename.endsWith(".js"))
        return "application/javascript";
    else if (filename.endsWith(".ico"))
        return "image/x-icon";
    else if (filename.endsWith(".png"))
        return "image/png";
    else if (filename.endsWith(".jpg") || filename.endsWith(".jpeg"))
        return "image/jpeg";
    else if (filename.endsWith(".gif"))
        return "image/gif";
    else if (filename.endsWith(".svg"))
        return "image/svg+xml";
    else if (filename.endsWith(".json"))
        return "application/json";
    else if (filename.endsWith(".txt"))
        return "text/plain";
    else if (filename.endsWith(".xml"))
        return "text/xml";
    return "application/octet-stream";
}