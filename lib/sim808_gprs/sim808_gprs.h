#ifndef SIM808_GPRS_H
#define SIM808_GPRS_H

#include "sim808_gps.h"

// Configuración de conexión GPRS
#define APN "telefonica.es"          // APN de tu proveedor de servicios móviles
#define GPRS_USER "your_user"   // Usuario (si es necesario)
#define GPRS_PASS "8495"   // Contraseña (si es necesario)
#define GPRS_SERVER "your_server_ip"  // IP o URL del servidor al que enviar los datos
#define GPRS_PORT 80            // Puerto (puede ser 80 para HTTP)

// Prototipos de funciones
int sim808_gprs_connect(void);
void sim808_gprs_send_data(const GPSData *gps_data);
int sim808_gprs_disconnect(void);

#endif // SIM808_GPRS_H