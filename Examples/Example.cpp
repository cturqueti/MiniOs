#include "WiFiCredentialsJSON.h"

void setup()
{
    Serial.begin(115200);
    LittleFS.begin();

    WiFiCredentialsJSON jsonManager(WiFiLog::ENABLE);

    // 1. Criar configuração padrão
    jsonManager.createDefaultConfig("/wifi_config.json");

    // 2. Carregar configuração existente
    WiFiItems config = jsonManager.loadCredentials("/wifi_config.json");

    // 3. Modificar configuração
    config.ssid = "MinhaRedeWiFi";
    config.password = "SenhaSegura123";
    config.dhcpFlag = false;
    config.ip = {192, 168, 1, 100};
    config.gateway = {192, 168, 1, 1};
    config.subnet = {255, 255, 255, 0};
    config.configLoaded = true;

    jsonManager.saveCredentials(config, "/wifi_config.json");

    // 4. Verificar se existe
    if (jsonManager.configExists("/wifi_config.json"))
    {
        Serial.println("Configuração existe!");
    }

    // 5. Modificar apenas alguns campos
    WiFiItems newConfig = jsonManager.loadCredentials("/wifi_config.json");
    newConfig.password = "NovaSenha456";
    jsonManager.modifyCredentials("/wifi_config.json", newConfig);

    // 6. Apagar configuração (se necessário)
    // jsonManager.deleteCredentials("/wifi_config.json");
}

void loop()
{
    // Seu código principal aqui
}