#include "WiFiCaptivePortal.h"
#include <LittleFS.h>

// WiFiCaptivePortal::WiFiCaptivePortal(WiFiStorageInterface &storage)
//     : _server(80), _isRunning(false), _serverTaskHandle(NULL), _storage(storage)
// {
// }

WiFiCaptivePortal::WiFiCaptivePortal(WiFiLog log) : _server(80), _isRunning(false), _serverTaskHandle(NULL) {
    _log = log;
    if (!LittleFS.begin()) {
        if (_log == WiFiLog::ENABLE) {
            LOG_ERROR("[CAPTIVE PORTAL] Failed to mount LittleFS");
        }
        ERRORS_LIST.addError(ErrorCode::LITTLEFS_MOUNT_ERROR);
    }

    if (!LittleFS.exists(captivePortalFolder.data())) {
        if (_log == WiFiLog::ENABLE) {
            LOG_ERROR("[CAPTIVE PORTAL] Pasta %s não encontrada", captivePortalFolder.data());
        }
        ERRORS_LIST.addError(ErrorCode::FILE_NOT_FOUND);
    }
}

WiFiCaptivePortal::~WiFiCaptivePortal() { end(); }

void WiFiCaptivePortal::begin() {
    if (_isRunning) {
        return;
    }
    if (_log == WiFiLog::ENABLE) {
        LOG_INFO("[CAPTIVE PORTAL] Iniciando AP");
    }
    _startAP();
    _setupDNS();
    _setupServer();

    xTaskCreate(_serverTask, "CaptivePortalTask", CAPTIVE_PORTAL_TASK_STACK_SIZE, this, 1, &_serverTaskHandle);

    _isRunning = true;
}

void WiFiCaptivePortal::end() {
    if (!_isRunning)
        return;

    _server.stop();
    _dnsServer.stop();
    WiFi.softAPdisconnect(true);

    if (_serverTaskHandle != NULL) {
        vTaskDelete(_serverTaskHandle);
        _serverTaskHandle = NULL;
    }

    _isRunning = false;
}

bool WiFiCaptivePortal::isRunning() const { return _isRunning; }

void WiFiCaptivePortal::_startAP() {
    WiFi.softAP(CAPTIVE_PORTAL_SSID);
    _wifiCurrentValues.ip = IPAddress(192, 168, 4, 1);
    _wifiCurrentValues.subnet = IPAddress(255, 255, 255, 0);
    WiFi.softAPConfig(_wifiCurrentValues.ip, _wifiCurrentValues.ip, _wifiCurrentValues.subnet);
    if (_log == WiFiLog::ENABLE) {
        LOG_INFO("[CAPTIVE PORTAL] AP IP address: %s", _wifiCurrentValues.ip.toString().c_str());
    }
}

void WiFiCaptivePortal::_setupDNS() {
    _dnsServer.start(CAPTIVE_PORTAL_DNS_PORT, "*", WiFi.softAPIP());

    if (_log == WiFiLog::ENABLE) {
        LOG_INFO("[CAPTIVE PORTAL] DNS server started on %s", WiFi.softAPIP().toString().c_str());
    }
}

void WiFiCaptivePortal::_setupServer() {
    _server.on("/", HTTP_GET, [this]() {
        if (_log == WiFiLog::ENABLE) {
            LOG_INFO("[CAPTIVE PORTAL] Serving index page /");
        }
        _handleRoot();
    });

    _server.on("/index.html", HTTP_GET, [this]() {
        if (_log == WiFiLog::ENABLE) {
            LOG_INFO("[CAPTIVE PORTAL] Serving index page /index.html");
        }
        _handleRoot(); // Redireciona para a raiz
    });

    _server.on("/config.html", HTTP_GET, [this]() {
        if (_log == WiFiLog::ENABLE) {
            LOG_INFO("[CAPTIVE PORTAL] Serving config page /config.html");
        }
        _handleConfig();
    });

    /* ------------- Redireciona para a página do portal ------------ */
    _server.on("/connecttest.txt", HTTP_GET, [this]() {
        if (_log == WiFiLog::ENABLE) {
            LOG_INFO("[CAPTIVE PORTAL] Received connecttest.txt request");
        }
        // _server.send(200, "text/plain", "Microsoft NCSI");
        _server.sendHeader("Location", "/"); // Redireciona para a página do portal
        _server.send(302, "text/plain", "Redirecting to captive portal");
    });

    _server.on("/hotspot-detect.html", HTTP_GET, [this]() {
        if (_log == WiFiLog::ENABLE) {
            LOG_INFO("[CAPTIVE PORTAL] Received hotspot-detect.html request");
        }
        _server.sendHeader("Location", "/");
        _server.send(302, "text/plain", "");
    });

    _server.on("/ncsi.txt", HTTP_GET, [this]() {
        if (_log == WiFiLog::ENABLE) {
            LOG_INFO("[CAPTIVE PORTAL] Received ncsi.txt request");
        }
        // _server.send(200, "text/plain", "Microsoft NCSI");
        _server.sendHeader("Location", "/"); // Redireciona para a página do portal
        _server.send(302, "text/plain", "Redirecting to captive portal");
        // _server.send(200, "text/plain", "OK");
    });

    _server.on("/generate_204", HTTP_GET, [this]() {
        if (_log == WiFiLog::ENABLE) {
            LOG_INFO("[CAPTIVE PORTAL] Received generate_204 request");
        }
        // _server.send(200, "text/plain", "Microsoft NCSI");
        _server.sendHeader("Location", "/"); // Redireciona para a página do portal
        _server.send(302, "text/plain", "Redirecting to captive portal");
        // _server.send(204, "text/plain", "");
    });
    /* ------------- FIM da redirecionamento da página do portal ------------- */
    _server.on("/favicon.ico", HTTP_GET, [this]() {
        if (_log == WiFiLog::ENABLE) {
            LOG_INFO("[CAPTIVE PORTAL] Received favicon.ico request");
        }
        String iconPath = String(captivePortalFolder.data()) + "/icon.png";
        if (LittleFS.exists(iconPath)) {
            File file = LittleFS.open(iconPath, "r");
            _server.sendHeader("Cache-Control", "public, max-age=86400"); // Cache de 1 dia
            _server.send(200, "image/png", file.readString());
            file.close();
        } else {
            _server.send(404, "text/plain", "Favicon not found");
        }
    });

    _server.on("/scan-wifi", HTTP_GET, [this]() {
        this->_handleScanWifi();
        // int n = WiFi.scanNetworks();
        // if (n == WIFI_SCAN_FAILED) {
        //     _server.send(500, "application/json", "{\"error\":\"Scan failed\"}");
        //     return;
        // }
        // JsonDocument doc;

        // JsonArray networks = doc.to<JsonArray>();

        // for (int i = 0; i < n; ++i) {
        //     JsonObject network = networks.add<JsonObject>();
        //     network["ssid"] = WiFi.SSID(i);
        //     network["rssi"] = WiFi.RSSI(i);
        // }

        // String json;
        // serializeJson(doc, json);
        // _server.send(200, "application/json", json);
        // WiFi.scanDelete();
    });

    _server.on("/connect", HTTP_POST, [this]() {
        if (_log == WiFiLog::ENABLE) {
            LOG_INFO("[CAPTIVE PORTAL] Received connect request");
        }
        JsonDocument doc;
        DeserializationError error = deserializeJson(doc, _server.arg("plain"));
        if (error) {
            _server.send(400, "application/json", "{\"success\":false,\"message\":\"JSON inválido\"}");
            if (_log == WiFiLog::ENABLE) {
                LOG_ERROR("[CAPTIVE PORTAL] JSON inválido: %s", error.c_str());
            }
            return;
        }
        WiFiItems config;
        // Obter dados básicos
        String ssid = doc["ssid"].as<String>();
        String password = doc["password"].as<String>();
        String hostname = doc["hostname"].as<String>();
        String ipMode = doc["ipMode"].as<String>();

        // Configuração de IP
        if (ipMode == "static") {
            IPAddress static_ip, gateway, subnet;

            if (!static_ip.fromString(doc["staticIp"].as<String>()) ||
                !gateway.fromString(doc["gateway"].as<String>()) || !subnet.fromString(doc["subnet"].as<String>())) {
                _server.send(400, "application/json", "{\"success\":false,\"message\":\"Endereço IP inválido\"}");
                return;
            }

            WiFi.config(static_ip, gateway, subnet);
        } else {
            WiFi.config(INADDR_NONE, INADDR_NONE, INADDR_NONE);
        }

        // Configurar hostname
        WiFi.setHostname(hostname.c_str());

        // Conectar à rede
        WiFi.begin(ssid.c_str(), password.c_str());

        // Aguardar conexão (com timeout)
        int attempts = 0;
        while (WiFi.status() != WL_CONNECTED && attempts < 20) {
            delay(500);
            attempts++;
        }

        if (WiFi.status() == WL_CONNECTED) {
            // if (_saveCredentials(config)) {
            //     _server.send(200, "application/json",
            //                  "{\"status\":\"success\",\"message\":\"Credenciais salvas com sucesso\"}");
            // } else {
            //     _server.send(500, "application/json",
            //                  "{\"status\":\"error\",\"message\":\"Falha ao salvar credenciais\"}");
            //     ERRORS_LIST.addError(ErrorCode::CREDENTIALS_SAVE_ERROR);
            // }
            if (_log == WiFiLog::ENABLE) {
                LOG_INFO("[CAPTIVE PORTAL] Connected to WiFi");
            }
            // _server.sendHeader("Location", "/success");
            // _server.send(302, "text/plain", "Redirecting to success page");
            // Preparar resposta
            JsonDocument responseDoc;
            responseDoc["success"] = WiFi.status() == WL_CONNECTED;
            responseDoc["message"] = WiFi.status() == WL_CONNECTED ? "Conectado com sucesso" : "Falha na conexão";
            responseDoc["ip"] = WiFi.localIP().toString();

            _saveCredentials(config);

            String response;
            serializeJson(responseDoc, response);
            _server.send(200, "application/json", response);
        }

        // Opcional: encerrar o portal após conexão
        delay(1000);
        end();
    });

    _server.on("/success", HTTP_GET, [this]() {
        if (_log == WiFiLog::ENABLE) {
            LOG_INFO("[CAPTIVE PORTAL] Received success request");
        }
        _server.sendHeader("Location", "/config.html");
        _server.send(302, "text/plain", "Redirecting to config");
    });

    _server.on("/network-settings", HTTP_GET, [this]() {
        JsonDocument doc;

        // Obter configurações atuais
        doc["hostname"] = WiFi.getHostname();
        doc["currentIp"] = WiFi.localIP().toString();

        // Obter modo IP (você precisará armazenar isso nas preferências)
        Preferences preferences;
        preferences.begin("network", true);
        doc["ipMode"] = preferences.getString("ip-mode", "dhcp");

        if (doc["ipMode"] == "static") {
            doc["ip"] = preferences.getString("ip", "");
            doc["gateway"] = preferences.getString("gateway", "");
            doc["subnet"] = preferences.getString("subnet", "");
        }
        preferences.end();

        String json;
        serializeJson(doc, json);
        _server.send(200, "application/json", json);
    });

    _server.on("/save-settings", HTTP_POST, [this]() {
        JsonDocument doc;
        DeserializationError error = deserializeJson(doc, _server.arg("plain"));

        if (error) {
            _server.send(400, "application/json", "{\"success\":false,\"message\":\"JSON inválido\"}");
            return;
        }

        Preferences preferences;
        preferences.begin("network", false);

        // Salvar hostname para mDNS
        String hostname = doc["hostname"].as<String>();
        if (hostname.length() > 0) {
            preferences.putString("hostname", hostname);
            WiFi.setHostname(hostname.c_str());
        }

        // Salvar configurações IP
        String ipMode = doc["ipMode"].as<String>();
        preferences.putString("ip-mode", ipMode);

        if (ipMode == "static") {
            String ip = doc["ip"].as<String>();
            String gateway = doc["gateway"].as<String>();
            String subnet = doc["subnet"].as<String>();

            preferences.putString("ip", ip);
            preferences.putString("gateway", gateway);
            preferences.putString("subnet", subnet);

            // Aplicar configuração estática
            IPAddress staticIP, staticGateway, staticSubnet;
            if (staticIP.fromString(ip) && staticGateway.fromString(gateway) && staticSubnet.fromString(subnet)) {
                WiFi.config(staticIP, staticGateway, staticSubnet);
            }
        } else {
            // Usar DHCP
            WiFi.config(INADDR_NONE, INADDR_NONE, INADDR_NONE);
        }
        preferences.end();

        // Reiniciar conexão para aplicar as configurações
        WiFi.reconnect();

        JsonDocument responseDoc;
        responseDoc["success"] = true;
        responseDoc["message"] = "Configurações salvas! Reconectando...";
        responseDoc["currentIp"] = WiFi.localIP().toString();

        String response;
        serializeJson(responseDoc, response);
        _server.send(200, "application/json", response);
    });

    _server.onNotFound([this]() {
        _handleRoot();
        if (_log == WiFiLog::ENABLE) {
            LOG_INFO("[CAPTIVE PORTAL] Page not found: %s", _server.uri().c_str());
        }
        _server.sendHeader("Location", "/");
        _server.send(302, "text/plain", "Redirecting to /");
    });

    _server.begin();
}

void WiFiCaptivePortal::_handleClient() {
    _server.handleClient();
    _dnsServer.processNextRequest();
}

void WiFiCaptivePortal::_serverTask(void *pvParameters) {
    WiFiCaptivePortal *portal = (WiFiCaptivePortal *)pvParameters;
    while (true) {
        portal->_handleClient();
        vTaskDelay(10 / portTICK_PERIOD_MS);
    }
    vTaskDelete(NULL);
}

String WiFiCaptivePortal::_loadFromLittleFS(const String &path) {
    if (_log == WiFiLog::ENABLE) {
        LOG_DEBUG("[CAPTIVE PORTAL] Loading file: %s", path.c_str());
    }
    File file = LittleFS.open(path, "r");
    if (!file) {
        if (_log == WiFiLog::ENABLE) {
            LOG_ERROR("[CAPTIVE PORTAL] Failed to open file: %s", path.c_str());
        }
        ERRORS_LIST.addError(ErrorCode::FILE_NOT_FOUND);
        return "";
    }

    String content = file.readString();
    file.close();
    return content;
}

String WiFiCaptivePortal::_getContentType(const String &filename) {
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

bool WiFiCaptivePortal::_beginCredentials() {
    if (_preferences.begin(nvs_namespace.data(), false)) {
        return true;
    }
    if (_log == WiFiLog::ENABLE) {
        LOG_ERROR("[WiFi] Error on load NVS");
    }
    ERRORS_LIST.addError(ErrorCode::NVS_BEGIN_ERROR);
    return false;
}

bool WiFiCaptivePortal::_saveCredentials(WiFiItems wifi) {
    bool success = true;
    success &= _preferences.putString("ssid", wifi.ssid.c_str());
    success &= _preferences.putString("password", wifi.password.c_str());
    _preferences.end();
    return true;
}

void WiFiCaptivePortal::_handleRoot() {
    // Define os caminhos dos arquivos
    const String indexPath = String(captivePortalFolder.data()) + "/index.html";
    const String cssPath = String(captivePortalFolder.data()) + "/style.css";
    const String jsPath = String(captivePortalFolder.data()) + "/script.js";
    const String iconPath = String(captivePortalFolder.data()) + "/images/icon.png";

    // Verifica se o arquivo HTML principal existe
    if (!LittleFS.exists(indexPath)) {
        _logError(F("Index file not found"), indexPath, ErrorCode::FILE_NOT_FOUND);
        _server.send(200, "text/html",
                     "<!DOCTYPE html><html><head><title>Erro</title></head>"
                     "<body><h1>Portal Captivo</h1><p>Página não encontrada</p></body></html>");
        return;
    }

    // Carrega o HTML principal
    String html = _loadFromLittleFS(indexPath);
    if (html.isEmpty()) {
        _logError(F("Failed to load index file"), indexPath, ErrorCode::FILE_READ_ERROR);
        _server.send(500, "text/plain", "Erro ao carregar a página");
        return;
    }

    // Processa CSS incorporado
    _embedFileContent(html, cssPath, "<style>", "</style>", "</head>");

    // Processa JavaScript incorporado
    _embedFileContent(html, jsPath, "<script>", "</script>", "</body>");

    // Envia a resposta
    _server.send(200, "text/html", html);
}

// Função auxiliar para registrar erros
void WiFiCaptivePortal::_logError(const __FlashStringHelper *message, const String &path, ErrorCode code) {
    if (_log == WiFiLog::ENABLE) {
        LOG_ERROR("[CAPTIVE PORTAL] %s: %s", message, path.c_str());
    }
    ERRORS_LIST.addError(code);
}

// Função auxiliar para incorporar arquivos
void WiFiCaptivePortal::_embedFileContent(String &html, const String &filePath, const String &prefix,
                                          const String &suffix, const String &insertBefore) {
    if (LittleFS.exists(filePath)) {
        String content = _loadFromLittleFS(filePath);
        if (!content.isEmpty()) {
            html.replace(insertBefore, prefix + content + suffix + insertBefore);
        } else {
            _logError(F("Failed to load file"), filePath, ErrorCode::FILE_READ_ERROR);
        }
    } else {
        _logError(F("File not found"), filePath, ErrorCode::FILE_NOT_FOUND);
    }
}

void WiFiCaptivePortal::_handleScanWifi() {
    if (_log == WiFiLog::ENABLE) {
        LOG_DEBUG("[CAPTIVE PORTAL] Handling WiFi scan request");
    }

    // Realiza o scan das redes WiFi
    int numNetworks = WiFi.scanNetworks(false, true); // scanNetworks(async, showHidden)

    // Cria o documento JSON
    DynamicJsonDocument doc(2048); // Tamanho ajustado conforme necessidade
    JsonArray networks = doc.to<JsonArray>();

    for (int i = 0; i < numNetworks; ++i) {
        JsonObject network = networks.createNestedObject();

        network["ssid"] = WiFi.SSID(i);
        network["rssi"] = WiFi.RSSI(i);
        network["channel"] = WiFi.channel(i);
        network["open"] = (WiFi.encryptionType(i) == WIFI_AUTH_OPEN);

        // Adiciona ícone baseado na segurança (opcional)
        network["icon"] = (WiFi.encryptionType(i) == WIFI_AUTH_OPEN) ? "🔓" : "🔒";

        if (_log == WiFiLog::ENABLE) {
            LOG_DEBUG("[CAPTIVE PORTAL] Found network: %s (%ddBm)", WiFi.SSID(i).c_str(), WiFi.RSSI(i));
        }
    }

    // Serializa o JSON para string
    String response;
    serializeJson(doc, response);

    // Envia a resposta
    _server.send(200, "application/json", response);

    // Limpa os resultados do scan
    WiFi.scanDelete();
}

void WiFiCaptivePortal::_handleConfig() {
    if (_log == WiFiLog::ENABLE) {
        LOG_DEBUG("[CAPTIVE PORTAL] Loading config page...");
    }
    String configPath = String(captivePortalFolder.data()) + "/config.json";
    if (!LittleFS.exists(configPath)) {
        if (_log == WiFiLog::ENABLE) {
            LOG_ERROR("[CAPTIVE PORTAL] %s not found", configPath.c_str());
        }
        ERRORS_LIST.addError(ErrorCode::FILE_NOT_FOUND);
        _server.send(404, "text/plain", "Config page not found");
        return;
    }

    String html = _loadFromLittleFS(configPath);
    _server.send(200, "text/html", html);
    return;
}