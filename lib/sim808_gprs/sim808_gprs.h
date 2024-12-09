#ifndef SIM808_GPRS_H
#define SIM808_GPRS_H

#include "sim808_gps.h"

// Configuración de conexión GPRS
#define APN "telefonica.es"          // APN de tu proveedor de servicios móviles
#define GPRS_USER "your_user"   // Usuario (si es necesario)
#define GPRS_PASS "8495"   // Contraseña (si es necesario)
#define GPRS_SERVER "https://gps-data-server.glitch.me"  // IP o URL del servidor al que enviar los datos
#define GPRS_PORT 3000            // Puerto

// Prototipos de funciones
int sim808_config_sim(void);
void sim808_check_network_status();
int sim808_gprs_connect(void);
void sim808_gprs_send_data(char *shipping_buffer);
int sim808_gprs_disconnect(void);

#endif // SIM808_GPRS_H