// bluetooth_link.h
#ifndef BLUETOOTH_LINK_H
#define BLUETOOTH_LINK_H

#include "esp_err.h"

#define BLUETOOTH_DEVICE_NAME "BOYA_SMART"
#define SERVER_URL "gps-data-server.glitch.me"


//bluetooth
esp_err_t bluetooth_init(void);
void escanear_boyas_cercanas(void);
void enlazar_boyas(long long int *mmsi);


#endif // BLUETOOTH_LINK_H