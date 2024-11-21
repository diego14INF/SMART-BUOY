#ifndef SIM808_H
#define SIM808_H

#include <time.h>

typedef struct {
    double time;
    float latitude;
    float longitude;
    float altitude;
    float speed;
    float course;
    int battery_level;
    int battery_voltage;
} GPSData;

//Funciones para la comunicación UART
void sim808_send_command(const char *command);

void sim808_read_response(char *buffer, size_t buffer_size);

// Funciones para inicializar y obtener datos del SIM808
// Inicializa el módulo SIM808
int sim808_init(void);

// Activa el GPS del SIM808, tal vez lo elimine porque ya le mando este comando dentro de la inicializacion
//void sim808_gps_power_on(void);

// Reinicia el GPS en el modo especificado
void sim808_gps_reset_mode(int mode);

// Obtiene el estado actual del GPS
void sim808_gps_get_status(void);

//Obtiene los datos de geolocalización
int sim808_get_gps_data(GPSData *data);

//Obtiene los valores de la batería del módulo
int sim808_get_battery_status(GPSData *battery_voltage);


#endif // SIM808_H