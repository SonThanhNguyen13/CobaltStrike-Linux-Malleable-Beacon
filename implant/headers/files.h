#ifndef FILES_H_
#define FILES_H_

#include <stdint.h>
#include <stddef.h>

void command_download(const uint8_t *data, size_t data_len);
void command_download_stop(const uint8_t *data, size_t data_len);
void download_poll();

#endif
