#ifndef SIM808_H
#define SIM808_H

#include <stddef.h>

int sim808_init();
void sim808_send_command(const char *command);
void sim808_read_response(char *buffer, size_t buffer_size);
void monitorizar_vbat();

#endif // SIM808_H