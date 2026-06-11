#ifndef DEBUG_H
#define DEBUG_H

#include <stdio.h>

// Debug printing macros
// Compile with -DDEBUG to enable debug output
// Example: gcc -DDEBUG myfile.c -o myprogram

#ifdef DEBUG
    // Debug print with automatic [DEBUG] prefix
    #define DEBUG_PRINT(fmt, ...) printf("[DEBUG] " fmt, ##__VA_ARGS__)
    
    // Debug print for hex dumps
    #define DEBUG_HEX(label, data, len) do { \
        printf("[DEBUG] %s: ", label); \
        for(size_t _i = 0; _i < (len); _i++) { \
            printf("%02x", ((uint8_t*)(data))[_i]); \
            if ((_i + 1) % 32 == 0 && (_i + 1) < (len)) { \
                printf("\n[DEBUG]                  "); \
            } \
        } \
        printf("\n"); \
    } while(0)
    
    // Debug print for hex dumps with spaces
    #define DEBUG_HEX_SPACED(label, data, len) do { \
        printf("[DEBUG] %s: ", label); \
        for(size_t _i = 0; _i < (len); _i++) { \
            printf("%02x ", ((uint8_t*)(data))[_i]); \
            if ((_i + 1) % 16 == 0 && (_i + 1) < (len)) { \
                printf("\n[DEBUG]                  "); \
            } \
        } \
        printf("\n"); \
    } while(0)
    
    // Info messages (always printed)
    #define INFO_PRINT(fmt, ...) printf("[*] " fmt, ##__VA_ARGS__)

    // Error messages (always printed)
    #define ERROR_PRINT(fmt, ...) fprintf(stderr, "[!] " fmt, ##__VA_ARGS__)

#else
    // No-op when DEBUG is not defined
    #define DEBUG_PRINT(fmt, ...)
    #define DEBUG_HEX(label, data, len)
    #define DEBUG_HEX_SPACED(label, data, len)
    #define INFO_PRINT(fmt, ...)
    #define ERROR_PRINT(fmt, ...)
#endif



#endif // DEBUG_H
