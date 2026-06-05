#include "md5.h"

#if defined(ARDUINO_ARCH_ESP8266)
#include <MD5Builder.h>
#elif defined(ARDUINO_ARCH_ESP32)
#include <mbedtls/md5.h>
#endif

String MD5Module::calculateSign(const String& msgId, const String& deviceKey, unsigned long ts) {
#if defined(ARDUINO_ARCH_ESP8266)
    MD5Builder md5;
    md5.begin();
    md5.add(msgId + deviceKey + String(ts));
    md5.calculate();
    return md5.toString();
#elif defined(ARDUINO_ARCH_ESP32)
    const String input = msgId + deviceKey + String(ts);
    unsigned char hash[16] = {0};
    mbedtls_md5_context context;
    mbedtls_md5_init(&context);
    mbedtls_md5_starts_ret(&context);
    mbedtls_md5_update_ret(&context, reinterpret_cast<const unsigned char*>(input.c_str()), input.length());
    mbedtls_md5_finish_ret(&context, hash);
    mbedtls_md5_free(&context);

    char hex[33];
    for (int i = 0; i < 16; ++i) {
        snprintf(&hex[i * 2], 3, "%02x", hash[i]);
    }
    hex[32] = '\0';
    return String(hex);
#else
    return "";
#endif
}