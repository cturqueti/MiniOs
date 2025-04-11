#include "WiFiCredentialsJSON.h"

bool WiFiCredentialsJSON::saveCredentials(const WiFiItems &config)
{
    if (_log == WiFiLog::ENABLE)
    {
        LOG_DEBUG("[WIFI] Salvando configurações no arquivo: %s", defaultFilePath);
    }

    JsonDocument doc = createJsonFromConfig(config);
    return writeJsonToFile(doc);
}

WiFiItems WiFiCredentialsJSON::loadCredentials()
{
    WiFiItems wifi;
    if (_log == WiFiLog::ENABLE)
    {
        LOG_DEBUG("Carregando configuração...");
    }

    if (!LittleFS.exists(defaultFilePath))
    {
        if (_log == WiFiLog::ENABLE)
        {
            LOG_ERROR("[WIFI] Arquivo de configuração não encontrado: %s", defaultFilePath);
        }
        wifi.configLoaded = false;
        return wifi; // Retorna objeto vazio
    }

    File file = LittleFS.open(defaultFilePath, "r");
    if (!file)
    {
        if (_log == WiFiLog::ENABLE)
        {
            if (_log == WiFiLog::ENABLE)
            {
                LOG_ERROR("[WIFI] Erro ao abrir %s", defaultFilePath);
            }
        }

        return WiFiItems(); // Defina um código de erro apropriado
    }

    size_t size = file.size();
    if (size == 0)
    {
        if (_log == WiFiLog::ENABLE)
        {
            LOG_ERROR("[WIFI] Arquivo vazio: %s", defaultFilePath);
        }
        _wifi.reset();
        return WiFiItems(); // Retorna nullptr se o arquivo estiver vazio
    }

    // Aloca o buffer com o tamanho do arquivo
    std::unique_ptr<char[]> buf(new char[size + 1]); // +1 para garantir espaço para o terminador '\0'
    file.readBytes(buf.get(), size);
    buf[size] = '\0'; // Adiciona o terminador de string
    file.close();

    JsonDocument doc; // Ajuste o tamanho conforme necessário
    DeserializationError error = deserializeJson(doc, buf.get());

    if (error)
    {
        if (_log == WiFiLog::ENABLE)
        {
            LOG_ERROR("[WIFI] Falha ao analisar JSON: %s", error.c_str());
        }
        // _wifi.reset();
        return WiFiItems(); // Defina um código de erro apropriado
    }

    if (doc["ssid"].is<const char *>())
    {
        _wifi.ssid = doc["ssid"].as<std::string>();
        if (_wifi.ssid.empty())
        {
            _wifi.configLoaded = false;
            return _wifi;
        }
    }
    else
    {
        _wifi.configLoaded = false;
        return wifi;
    }
    if (doc["password"].is<const char *>())
    {
        _wifi.password = doc["password"].as<std::string>();
    }
    else
    {
        _wifi.configLoaded = false;
        return wifi;
    }

    _wifi.dhcpFlag = doc["dhcp"] | false;

    if (!_wifi.dhcpFlag)
    {
        if (_log == WiFiLog::ENABLE)
        {
            LOG_INFO("[WIFI] Rading fixed IP configurations");
        }
        if (doc["ip"].is<JsonArray>() && doc["ip"].size() == 4)
        {
            for (int i = 0; i < 4; i++)
            {
                _wifi.ip[i] = doc["ip"][i];
                if (_wifi.ip[i] < 0 || _wifi.ip[i] > 255)
                {
                    if (_log == WiFiLog::ENABLE)
                    {
                        LOG_ERROR("[WIFI] IP address invalid");
                    }
                    _wifi.configLoaded = false;
                    return wifi;
                }
            }
        }
        else
        {
            if (_log == WiFiLog::ENABLE)
            {
                LOG_ERROR("[WIFI] Chave 'ip' ausente ou inválida.");
            }
            _wifi.configLoaded = false;
            return wifi;
        }

        if (doc["gateway"].is<JsonArray>() && doc["gateway"].size() == 4)
        {
            for (int i = 0; i < 4; i++)
            {
                _wifi.gateway[i] = doc["gateway"][i];
                if (_wifi.gateway[i] < 0 || _wifi.gateway[i] > 255)
                {
                    if (_log == WiFiLog::ENABLE)
                    {
                        LOG_ERROR("[WIFI] Gateway address invalid");
                    }
                    _wifi.configLoaded = false;
                    return wifi;
                }
            }
        }
        else
        {
            if (_log == WiFiLog::ENABLE)
            {
                LOG_ERROR("[WIFI] Chave 'gateway' ausente ou inválida.");
            }
            _wifi.configLoaded = false;
            return wifi;
        }

        if (doc["subnet"].is<JsonArray>() && doc["subnet"].size() == 4)
        {
            for (int i = 0; i < 4; i++)
            {
                _wifi.subnet[i] = doc["subnet"][i];
                if (_wifi.subnet[i] < 0 || _wifi.subnet[i] > 255)
                {
                    if (_log == WiFiLog::ENABLE)
                    {
                        LOG_ERROR("[WIFI] Subnet Mask Invalid");
                    }
                    _wifi.configLoaded = false;
                    return wifi;
                }
            }
        }
        else
        {
            if (_log == WiFiLog::ENABLE)
            {
                LOG_ERROR("[WIFI] Chave 'subnet' ausente ou inválida.");
            }
            _wifi.configLoaded = false;
            return wifi;
        }
    }
    else
    {
        if (_log == WiFiLog::ENABLE)
        {
            LOG_INFO("[WIFI] Selected DHCP");
        }
    }
    _wifi.configLoaded = true;
    if (_log == WiFiLog::ENABLE)
    {
        LOG_INFO("[WIFI] Configurações carregadas com sucesso.");
    }

    return _wifi;
}

bool WiFiCredentialsJSON::deleteCredentials()
{
    if (!LittleFS.exists(defaultFilePath))
    {
        if (_log == WiFiLog::ENABLE)
        {
            LOG_WARN("[WIFI] Arquivo não existe: %s", defaultFilePath);
        }
        return false;
    }

    bool success = LittleFS.remove(defaultFilePath);
    if (_log == WiFiLog::ENABLE)
    {
        if (success)
        {
            LOG_INFO("[WIFI] Arquivo removido: %s", defaultFilePath);
        }
        else
        {
            LOG_ERROR("[WIFI] Falha ao remover arquivo: %s", defaultFilePath);
        }
    }
    return success;
}

bool WiFiCredentialsJSON::createDefaultConfig()
{
    WiFiItems defaultConfig;
    defaultConfig.ssid = "";
    defaultConfig.password = "";
    defaultConfig.dhcpFlag = true;
    defaultConfig.configLoaded = false;

    return saveCredentials(defaultConfig);
}

bool WiFiCredentialsJSON::modifyCredentials(const WiFiItems &newConfig)
{
    if (!configExists())
    {
        if (_log == WiFiLog::ENABLE)
        {
            LOG_ERROR("[WIFI] Arquivo não existe para modificação: %s", defaultFilePath);
        }
        return false;
    }

    return saveCredentials(newConfig);
}

bool WiFiCredentialsJSON::configExists() const
{
    return LittleFS.exists(defaultFilePath);
}

// Métodos privados
bool WiFiCredentialsJSON::writeJsonToFile(const JsonDocument &doc)
{
    File file = LittleFS.open(defaultFilePath, "w");
    if (!file)
    {
        if (_log == WiFiLog::ENABLE)
        {
            LOG_ERROR("[WIFI] Falha ao criar arquivo: %s", defaultFilePath);
        }
        return false;
    }

    size_t bytesWritten = serializeJson(doc, file);
    file.close();

    if (bytesWritten == 0)
    {
        if (_log == WiFiLog::ENABLE)
        {
            LOG_ERROR("[WIFI] Falha ao escrever no arquivo: %s", defaultFilePath);
        }
        return false;
    }

    if (_log == WiFiLog::ENABLE)
    {
        LOG_INFO("[WIFI] Configurações salvas com sucesso em: %s (%d bytes)", defaultFilePath, bytesWritten);
    }
    return true;
}

JsonDocument WiFiCredentialsJSON::createJsonFromConfig(const WiFiItems &config)
{
    JsonDocument doc;

    doc["ssid"] = config.ssid;
    doc["password"] = config.password;
    doc["dhcp"] = config.dhcpFlag;

    if (!config.dhcpFlag)
    {
        JsonArray ip = doc["ip"].to<JsonArray>();
        for (const auto &octet : config.ip)
        {
            ip.add(octet);
        }

        JsonArray gateway = doc["gateway"].to<JsonArray>();
        for (const auto &octet : config.gateway)
        {
            gateway.add(octet);
        }

        JsonArray subnet = doc["subnet"].to<JsonArray>();
        for (const auto &octet : config.subnet)
        {
            subnet.add(octet);
        }
    }

    return doc;
}