#include "energy_control.h"
#include "i2c_com.h"  // Asumiendo librería para el sensor INA219
#include "driver/gpio.h"
#include "driver/adc.h"
#include "esp_pm.h"
#include "sim808_gps.h"


//#define UMBRAL_VOLT_BATERIA_MAXIMA 12.6  // Tensión máxima de la batería (4.2V × 3).
#define UMBRAL_VOLT_BATERIA_BAJA 9.5  // Tensión mínima operativa (UMBRAL MINIMO QUE NO SE DEBERIA SUPERAR 8.4-9 V). El voltaje mínimo seguro es 2.8V - 3.0V por celda para evitar una descarga profunda.
#define UMBRAL_VOLT_SIM808_MAX 4.3
#define UMBRAL_CORRIENTE_SIM808_MAX 5
#define UMBRAL_VOLT_LEDS_MAX 5
#define UMBRAL_CORRIENTE_LEDS_MAX 5
#define UMBRAL_VOLT_SIRENA_MAX 5
#define UMBRAL_CORRIENTE_SIRENA_MAX 5




//// Función para comprobar si la batería está por debajo del umbral
bool verificacion_nivel_bateria() {
    float voltaje = ina219_leer_voltaje_bus(INA219_ADDRESS_BATERIA);
    return voltaje < UMBRAL_VOLT_BATERIA_BAJA;
}


bool verificacion_estado_modulo(uint8_t ina219_addr) {
    float voltaje = ina219_leer_voltaje_bus(ina219_addr);
    float corriente = ina219_leer_corriente(ina219_addr);

    // Variables para almacenar los umbrales según el módulo
    float umbral_voltaje = 0.0;
    float umbral_corriente = 0.0;

    // Asignar los umbrales según la dirección I2C del módulo
    switch (ina219_addr) {
        case INA219_ADDRESS_SIM808:
            umbral_voltaje = UMBRAL_VOLT_SIM808_MAX;
            umbral_corriente = UMBRAL_CORRIENTE_SIM808_MAX;
            break;
        case INA219_ADDRESS_LEDS:
            umbral_voltaje = UMBRAL_VOLT_LEDS_MAX;
            umbral_corriente = UMBRAL_CORRIENTE_LEDS_MAX;
            break;
        case INA219_ADDRESS_SIRENA:
            umbral_voltaje = UMBRAL_VOLT_SIRENA_MAX;
            umbral_corriente = UMBRAL_CORRIENTE_SIRENA_MAX;
            break;
        default:
            return false; // Dirección no reconocida, no realizar verificación
    }

    // Comprobar si el módulo está consumiendo menos de lo esperado
    if (voltaje < umbral_voltaje || corriente < umbral_corriente) {
        return true;  // Indicar que hay un problema en el módulo
    }

    return false; // Todo en orden
}

void sim808_bateria_baja(){
    char response[128];
        
        //Desactivamos la funcionalidad Bluetooth
        sim808_send_command("AT+BTPOWER=0\r\n");
        sim808_wait_for_response(response, sizeof(response), 10000); // Espera hasta 5s

        //Reducimos el intervalo de busqueda de la red GSM a intervalos menos frecuentes
        sim808_send_command("AT+CFUN=4\r\n");
        sim808_wait_for_response(response, sizeof(response), 10000); // Espera hasta 5s

         //Activamos el modo de bajo consumo (Power Saving Mode - PSM)
        // sim808_send_command("AT+CSCLK=1\r\n");
        // sim808_wait_for_response(response, sizeof(response), 10000); // Espera hasta 5s


}