#ifndef SIM808_GPRS_H
#define SIM808_GPRS_H

#include "sim808_gps.h"

// Configuración de conexión GPRS
#define APN "telefonica.es"          // APN de tu proveedor de servicios móviles
#define GPRS_USER "telefonica"   // Usuario (si es necesario)
#define GPRS_PASS "8495"   // Contraseña (si es necesario)
#define GPRS_SERVER "https://gps-data-server.glitch.me"  // IP o URL del servidor al que enviar los datos
#define GPRS_PORT 3000            // Puerto

// Comandos AT para conexión GPRS
#define AT_GPRS_INIT "AT+CGATT=1\r\n"       // Conectar a la red GPRS
#define AT_GPRS_CTRL_SIGNAL "AT+CSQ\r\n"    //Para comprobar valores de la señal gsm, los valores deben entre 10 y 30 son buenos
#define AT_GPRS_APN "AT+CGDCONT=1,\"IP\",\"" APN "\"\r\n" // Configuración APN
#define AT_GPRS_STATUS "AT+CGACT?\r\n"      // Comprobar estado de la conexión GPRS
#define AT_TCP_CONNECT "AT+CIICR\r\n"       // Activar GPRS
#define AT_TCP_SOCKET "AT+CIPSTART=\"TCP\",\"" GPRS_SERVER "\",\"" #define GPRS_PORT "\"\r\n" // Conectar a servidor TCP


// Prototipos de funciones
int sim808_config_sim(void);
int sim808_check_network_status();
int sim808_gprs_connect_init();
int sim808_gprs_connect_apn();
int sim808_gprs_activate_data();
int sim808_gprs_establish_ppp();
int sim808_gprs_get_ip();
int sim808_gprs_tcp_connect();
void sim808_gprs_send_data(char *shipping_buffer);
int sim808_gprs_disconnect(void);

#endif // SIM808_GPRS_H