// WiFiStorageInterface.h
#pragma once
#include "WiFiItems.h"

class WiFiStorageInterface
{
public:
    virtual ~WiFiStorageInterface() = default;
    virtual bool saveCredentials(const WiFiItems &config) = 0;
    virtual WiFiItems loadCredentials() = 0;
    virtual bool deleteCredentials() = 0;
    virtual bool configExists() const = 0;
};