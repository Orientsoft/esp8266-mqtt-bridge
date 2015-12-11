#define SSID "ESP_UDP"
#define SSID_PASSWORD "udpesp12"

/// Put format strings in flash rather than needing to load them into program memory
#define USE_OPTIMIZE_PRINTF
/// Add missing prototype of os_printf_plus
extern int os_printf_plus(const char * format, ...) __attribute__ ((format (printf, 1, 2)));
