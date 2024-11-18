
#include <stdio.h>
#include "sim808.h" // Archivo de encabezado que contiene las declaraciones de las funciones y estructuras
#include <esp_rom_sys.h>  // Necesario para `esp_rom_delay_us()`

void app_main(void) {
    printf("Iniciando programa con SIM808...\n");

    // Inicializar el módulo SIM808
    if (sim808_init()==0){
        printf("Error al inicializar el SIM808.\n");
        //return; // Salir si no se inicializa correctamente
    }
    // Reiniciar el GPS en modo "Cold Start" (por ejemplo)
    sim808_gps_reset_mode(2); // 3 representa "Cold Start" en nuestro programa

    // Obtener estado actual del GPS
    sim808_gps_get_status();

    // Estructura para almacenar datos del GPS
    GPSData gps_data;

    // Variables para monitorear la batería
    //int battery_voltage;

    // Ciclo principal
    while (1) {

        // Obtener datos de geolocalización
        if (sim808_get_gps_data(&gps_data)) {
            printf("Datos de GPS obtenidos:\n");
            printf("Latitud: %.6f, Longitud: %.6f, Altitud: %.2f m, Velocidad: %.2f km/h, Curso: %.2f°\n",
                   gps_data.latitude, gps_data.longitude, gps_data.altitude,
                   gps_data.speed, gps_data.course);
        }

        // Monitorear la batería
        //sim808_get_battery_status(&battery_voltage);

        // Enviar datos a través de GSM
        //printf("Enviando datos mediante GSM...\n");
        //sim808_send_data_over_gsm(&gps_data);

        // Retraso de 1 segundo
        esp_rom_delay_us(2000000);   // 1000 ms = 1000000 us
    }
}