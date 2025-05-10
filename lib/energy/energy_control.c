#include "energy_control.h"
#include "i2c_com.h"  // Asumiendo librería para el sensor INA219
#include "driver/gpio.h"
#include "driver/adc.h"
#include "esp_pm.h"
#include "sim808_gps.h"

#define UMBRAL_BATERIA_ZUMBADOR 60
#define UMBRAL_BATERIA_BALIZA 40
#define UMBRAL_BATERIA_BAJO_CONSUMO 20

#define BATERIA_ADC_CHANNEL ADC_CHANNEL_6 // GPIO34 por ejemplo
#define ADC_WIDTH ADC_WIDTH_BIT_12
#define ADC_ATTEN ADC_ATTEN_DB_11
#define TENSION_MAXIMA 4.2f  // Tensión máxima de la batería
#define TENSION_MINIMA 3.0f  // Tensión mínima operativa

static uint8_t nivel_bateria = 100;
static bool bajo_consumo = false;

void energia_init(void) {
    // Inicialización de sensores INA219
    //ina219_init();

    // Configuración del ADC para la batería
    adc1_config_width(ADC_WIDTH);
    adc1_config_channel_atten(BATERIA_ADC_CHANNEL, ADC_ATTEN);
}

float energia_leer_tension_bateria(void) {
    int adc_reading = adc1_get_raw(BATERIA_ADC_CHANNEL);
    float tension = ((float)adc_reading / 4095.0f) * TENSION_MAXIMA;
    return tension;
}


void sim808_control_energia(int a){
    char response[128];
    if(a==0){
        //Desactivamos las funciones GPS
         sim808_send_command("AT+CGNSPWR=0\r\n");
         sim808_wait_for_response(response, sizeof(response), 10000); // Espera hasta 5s

        //Desactivamos la funcionalidad Bluetooth
        sim808_send_command("AT+BTPOWER=0\r\n");
        sim808_wait_for_response(response, sizeof(response), 10000); // Espera hasta 5s

        //Activamos el modo de bajo consumo (Power Saving Mode - PSM)
        // sim808_send_command("AT+CSCLK=1\r\n");
        // sim808_wait_for_response(response, sizeof(response), 10000); // Espera hasta 5s

        //Reducimos el intervalo de busqueda de la red GSM a intervalos menos frecuentes
        sim808_send_command("AT+CFUN=4\r\n");
        sim808_wait_for_response(response, sizeof(response), 10000); // Espera hasta 5s
    }

}

// void energia_actualizar(void) {
//     // Leer corriente
//     float corriente_zumbador = ina219_leer_corriente(ZUMBADOR);
//     float corriente_baliza = ina219_leer_corriente(BALIZA);
//     float corriente_sim808 = ina219_leer_corriente(SIM808);

//     // Leer tensión de la batería
//     float tension_bateria = energia_leer_tension_bateria();

//     // Calcular nivel de batería
//     if (tension_bateria < TENSION_MINIMA) tension_bateria = TENSION_MINIMA;
//     nivel_bateria = (uint8_t)(((tension_bateria - TENSION_MINIMA) / (TENSION_MAXIMA - TENSION_MINIMA)) * 100);

//     // Control según prioridad
//     if (nivel_bateria < UMBRAL_BATERIA_ZUMBADOR) {
//         gpio_set_level(ZUMBADOR_GPIO, 0);
//     }
//     if (nivel_bateria < UMBRAL_BATERIA_BALIZA) {
//         gpio_set_level(BALIZA_GPIO, 0);
//     }
//     if (nivel_bateria < UMBRAL_BATERIA_BAJO_CONSUMO && !bajo_consumo) {
//         esp_pm_configure(&(esp_pm_config_esp32s3_t){
//             .max_freq_mhz = 80,
//             .min_freq_mhz = 20,
//             .light_sleep_enable = true
//         });
//         bajo_consumo = true;
//     }
// }

uint8_t energia_get_bateria(void) {
    return nivel_bateria;
}

bool energia_bajo_consumo(void) {
    return bajo_consumo;
}