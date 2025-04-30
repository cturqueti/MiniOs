#include "base64m.h"

const char base64::_base64_alphabet[] = "ABCDEFGHIJKLMNOPQRSTUVWXYZ"
                                        "abcdefghijklmnopqrstuvwxyz"
                                        "0123456789+/";

String base64::encode(const uint8_t *data, size_t length)
{
    String result;
    int i = 0;
    int j = 0;
    uint8_t char_array_3[3];
    uint8_t char_array_4[4];

    while (length--)
    {
        char_array_3[i++] = *(data++);
        if (i == 3)
        {
            char_array_4[0] = (char_array_3[0] & 0xfc) >> 2;
            char_array_4[1] = ((char_array_3[0] & 0x03) << 4) + ((char_array_3[1] & 0xf0) >> 4);
            char_array_4[2] = ((char_array_3[1] & 0x0f) << 2) + ((char_array_3[2] & 0xc0) >> 6);
            char_array_4[3] = char_array_3[2] & 0x3f;

            for (i = 0; i < 4; i++)
            {
                result += _base64_alphabet[char_array_4[i]];
            }
            i = 0;
        }
    }

    if (i)
    {
        for (j = i; j < 3; j++)
        {
            char_array_3[j] = '\0';
        }

        char_array_4[0] = (char_array_3[0] & 0xfc) >> 2;
        char_array_4[1] = ((char_array_3[0] & 0x03) << 4) + ((char_array_3[1] & 0xf0) >> 4);
        char_array_4[2] = ((char_array_3[1] & 0x0f) << 2) + ((char_array_3[2] & 0xc0) >> 6);

        for (j = 0; j < i + 1; j++)
        {
            result += _base64_alphabet[char_array_4[j]];
        }

        while (i++ < 3)
        {
            result += '=';
        }
    }

    return result;
}

String base64::encode(const String &text)
{
    return encode((const uint8_t *)text.c_str(), text.length());
}

String base64::decode(const String &encoded)
{
    int in_len = encoded.length();
    int i = 0;
    int j = 0;
    int in_ = 0;
    uint8_t char_array_4[4], char_array_3[3];
    String result;

    while (in_len-- && (encoded[in_] != '=') && isBase64(encoded[in_]))
    {
        char_array_4[i++] = encoded[in_];
        in_++;
        if (i == 4)
        {
            for (i = 0; i < 4; i++)
            {
                char_array_4[i] = strchr(_base64_alphabet, char_array_4[i]) - _base64_alphabet;
            }

            char_array_3[0] = (char_array_4[0] << 2) + ((char_array_4[1] & 0x30) >> 4);
            char_array_3[1] = ((char_array_4[1] & 0xf) << 4) + ((char_array_4[2] & 0x3c) >> 2);
            char_array_3[2] = ((char_array_4[2] & 0x3) << 6) + char_array_4[3];

            for (i = 0; i < 3; i++)
            {
                result += (char)char_array_3[i];
            }
            i = 0;
        }
    }

    if (i)
    {
        for (j = 0; j < i; j++)
        {
            char_array_4[j] = strchr(_base64_alphabet, char_array_4[j]) - _base64_alphabet;
        }

        char_array_3[0] = (char_array_4[0] << 2) + ((char_array_4[1] & 0x30) >> 4);
        char_array_3[1] = ((char_array_4[1] & 0xf) << 4) + ((char_array_4[2] & 0x3c) >> 2);

        for (j = 0; j < i - 1; j++)
        {
            result += (char)char_array_3[j];
        }
    }

    return result;
}

bool base64::isBase64(char c)
{
    return (isalnum(c) || (c == '+') || (c == '/'));
}