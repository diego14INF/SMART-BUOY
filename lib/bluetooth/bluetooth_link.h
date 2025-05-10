// bluetooth_link.h
#ifndef BLUETOOTH_LINK_H
#define BLUETOOTH_LINK_H

#include "esp_err.h"

#define BLUETOOTH_DEVICE_NAME "BOYA_SMART"
#define BUTTON_GPIO 0 // Ajustar al GPIO correcto del botón
#define BUTTON_HOLD_TIME 10000 // 10 segundos en milisegundos
#define SERVER_URL "gps-data-server.glitch.me"


//bluetoothesp_err_t bluetooth_init(void);
void check_button_press(void);
void scan_nearby_buoys(void);
void start_pairing(const char *remote_id);

// Maneja el intercambio de datos con el servidor
//esp_err_t exchange_data_with_server(const char *buoy_id); //esto realmente lo metere en la maquina de estados


#endif // BLUETOOTH_LINK_H