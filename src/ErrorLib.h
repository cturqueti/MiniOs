#ifndef ERROR_LIB_H
#define ERROR_LIB_H

#include <Arduino.h>
#include <LogLibrary.h>
#include <vector>

enum ErrorCode
{
    NONE,
    SSID_NOT_FOUND,
    NVS_BEGIN_ERROR,
    WARNING,
    INFO,
    SENSOR_FAIL,
    WIFI_DISCONNECTED,
    INVALID_DATA,
    ERROR_TYPE_COUNT
};

class ErrorLib
{
public:
    ErrorLib();
    ~ErrorLib();
    void addError(ErrorCode error);
    void clearErrors();
    void clearError(ErrorCode error);
    bool findError(ErrorCode error);
    void printErrors();

    std::vector<ErrorCode> _errors;

private:
};

extern ErrorLib ERRORS_LIST;

#endif // ERROR_LIB_H