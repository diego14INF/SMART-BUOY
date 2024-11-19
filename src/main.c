
#include <stdio.h>
#include "sim808_gps.h" // Archivo de encabezado que contiene las declaraciones de las funciones y estructuras
#include <esp_rom_sys.h>  // Necesario para `esp_rom_delay_us()`
#include "data_storage.h"
#include "sim808_gprs.h"

void app_main(void) {
    printf("Iniciando programa con SIM808...\n");
    
    data_storage_init();

    // Inicializar el módulo SIM808
    if (sim808_init()==0){
        printf("Error al inicializar el SIM808.\n");
        //return; // Salir si no se inicializa correctamente
    }

     //Conectar a la red GPRS
    if (sim808_gprs_connect()) {
        printf("Conexión GPRS exitosa.\n");
    }
    // Reiniciar el GPS en modo "Cold Start" (por ejemplo)
    sim808_gps_reset_mode(2); // 3 representa "Cold Start" en nuestro programa

    // Obtener estado actual del GPS
    sim808_gps_get_status();

    //Estructura para almacenar datos del GPS
    GPSData gps_data;

    // Ciclo principal
    while (1) {

        // Obtener datos de geolocalización
        sim808_get_gps_data(&gps_data);

        // Monitorear la batería
        sim808_get_battery_status(&gps_data);

        data_storage_save(&gps_data);

        // Obtener estado actual del GPS
        //sim808_gps_get_status();

        // Enviar datos a través de GSM
        //printf("Enviando datos mediante GSM...\n");
        //sim808_send_data_over_gsm(&gps_data);

        // Retraso de 1 segundo
        esp_rom_delay_us(2000000);   // 1000 ms = 1000000 us

        int num_storage;
        num_storage=data_storage_get_count();
        printf("Total de entradas a la memoria: %d\n", num_storage);

    }
}