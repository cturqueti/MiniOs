#ifndef CORE_BASE64M_H_
#define CORE_BASE64M_H_

#include <Arduino.h>

class base64
{
public:
    static String encode(const uint8_t *data, size_t length);
    static String encode(const String &text);
    static String decode(const String &encoded);

private:
    static inline bool isBase64(char c);
    static const char _base64_alphabet[];
};

#endif /* CORE_BASE64M_H_ */