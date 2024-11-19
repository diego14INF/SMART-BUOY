#ifndef GSM_DATA_H
#define GSM_DATA_H

#include "data_storage.h"

// Funciones para preparar y enviar datos por GSM
int gsm_prepare_data(char *buffer, size_t buffer_size);
int gsm_send_data(const char *data);

#endif // GSM_COMM_H