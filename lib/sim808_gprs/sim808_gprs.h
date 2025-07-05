#ifndef SIM808_GPRS_H
#define SIM808_GPRS_H

#include "sim808_gps.h"

// Configuración de conexión GPRS
#define APN "telefonica.es"          // APN de tu proveedor de servicios móviles
#define GPRS_USER "telefonica"   // Usuario (si es necesario)
#define GPRS_PASS "8495"   // Contraseña (si es necesario)
#define GPRS_SERVER "gps-data-server.glitch.me"  // IP o URL del servidor al que enviar los datos
#define HTTP_PORT 80           // Puerto
#define HTTPS_PORT 443
#define GLITCH_PORT 80

#define STRINGIFY(x) #x
#define TOSTRING(x) STRINGIFY(x)

//Comandos AT de comprobación y control
#define AT_GPRS_CTRL_SIGNAL "AT+CSQ\r\n"    //Para comprobar valores de la señal gsm, los valores deben entre 10 y 30 son buenos
#define AT_GPRS_CTRL_GPRS_ATTACHMENT "AT+CGATT?\r\n"      // Comprobar estado de la conexión GPRS
#define AT_GPRS_CTRL_IP "AT+CIFSR\r\n" //Devuelve IP


// Comandos AT para conexión TCP (En orden)
#define AT_GPRS_INIT "AT+CGATT=1\r\n"       // Conectar a la red GPRS
#define AT_GPRS_APN "AT+CSTT=\"" APN "\"\r\n" // Configuración APN
#define AT_GPRS_WIRELESS_CONNECT "AT+CIICR\r\n" //Iniciar conexion inalambrica
#define AT_TCP_SOCKET "AT+CIPSTART=\"TCP\",\"" GPRS_SERVER "\",\"" TOSTRING(HTTP_PORT) "\"\r\n" // Conectar a servidor TCP

//Comandos para comunicación SMS/TLF
#define SMS_CONFIG_COMAND "AT+CMGF=1\r\n" // Configura para enviar SMS
#define SMS_PHONE_NUMBER_COMAND "AT+CMGS=\"+34674080561\"\r\n" //Configura el número de destino


#define RESPONSE_BUFFER_SIZE 512
#define RESPONSE_TIMEOUT_MS 5000

// Funciones de configuracion
int sim808_config_sim(void);
int sim808_gprs_connect_init();
int sim808_gprs_connect_apn();
int sim808_gprs_wireless_activate();
int sim808_gprs_tcp_connect();
int sim808_gprs_send_data(char *shipping_buffer);
int sim808_gprs_disconnect(void);

//No necesario por ahora
int sim808_gprs_establish_ppp();


//Funciones de comprobacion
int sim808_check_network_status();
int sim808_check_gprs_attachment(void);
int sim808_check_apn_present(void);
int sim808_check_ppp_status(void);
int sim808_gprs_get_ip();
int sim808_full_reset(void);
int sim808_check_registration_status(void);
int sim808_check_functionality_status(void); 
int sim808_check_signal_strength(void);

int sim808_send_sos_sms(GPSData *data);

#endif // SIM808_GPRS_H