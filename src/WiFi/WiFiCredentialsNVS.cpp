#include "WiFiCredentialsNVS.h"

// Construtor
WiFiCredentialsNVS::WiFiCredentialsNVS() {}

// Inicializa o NVS
void WiFiCredentialsNVS::begin()
{
    preferences.begin(nvs_namespace, false);
}

// Fecha o NVS
void WiFiCredentialsNVS::end()
{
    preferences.end();
}

// Salva credenciais no NVS
bool WiFiCredentialsNVS::saveCredentials(const WiFiItems &config)
{
    begin();

    bool success = true;
    success &= preferences.putString("ssid", config.ssid.c_str());
    success &= preferences.putString("password", config.password.c_str());
    success &= preferences.putBool("dhcpFlag", config.dhcpFlag);

    if (!config.dhcpFlag)
    {
        uint8_t ip_arr[4] = {config.ip[0], config.ip[1], config.ip[2], config.ip[3]};
        uint8_t gateway_arr[4] = {config.gateway[0], config.gateway[1], config.gateway[2], config.gateway[3]};
        uint8_t subnet_arr[4] = {config.subnet[0], config.subnet[1], config.subnet[2], config.subnet[3]};

        success &= preferences.putBytes("ip", ip_arr, 4);
        success &= preferences.putBytes("gateway", gateway_arr, 4);
        success &= preferences.putBytes("subnet", subnet_arr, 4);
    }

    success &= preferences.putBool("configLoaded", config.configLoaded);

    end();
    return success;
}

// Carrega credenciais do NVS
WiFiItems WiFiCredentialsNVS::loadCredentials()
{
    begin();

    WiFiItems config;
    config.ssid = preferences.getString("ssid", "").c_str();
    config.password = preferences.getString("password", "").c_str();
    config.dhcpFlag = preferences.getBool("dhcpFlag", true);

    if (!config.dhcpFlag)
    {
        uint8_t ip_arr[4], gateway_arr[4], subnet_arr[4];

        if (preferences.getBytes("ip", ip_arr, 4) == 4)
        {
            config.ip.assign(ip_arr, ip_arr + 4);
        }

        if (preferences.getBytes("gateway", gateway_arr, 4) == 4)
        {
            config.gateway.assign(gateway_arr, gateway_arr + 4);
        }

        if (preferences.getBytes("subnet", subnet_arr, 4) == 4)
        {
            config.subnet.assign(subnet_arr, subnet_arr + 4);
        }
    }

    config.configLoaded = preferences.getBool("configLoaded", false);

    end();
    return config;
}

// Modifica credenciais existentes
bool WiFiCredentialsNVS::updateCredentials(const WiFiItems &config)
{
    // A atualização é igual a salvar (o NVS sobrescreve)
    return saveCredentials(config);
}

// Apaga todas as credenciais
bool WiFiCredentialsNVS::clearCredentials()
{
    begin();
    bool success = preferences.clear();
    end();
    return success;
}

// Verifica se existem credenciais salvas
bool WiFiCredentialsNVS::hasCredentials()
{
    begin();
    bool hasCreds = preferences.getBool("configLoaded", false);
    end();
    return hasCreds;
}
