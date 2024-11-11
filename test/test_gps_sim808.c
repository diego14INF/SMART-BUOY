#include <stdio.h>
#include <string.h>
#include "unity.h"       // Framework de pruebas unitarias
#include "gps_sim808.h"      // Módulo que estamos probando
#include "gps_data.h"      // Incluimos el gps_data para pruebas de datos GPS

void setUp(void) {
    // Inicializar el módulo SIM808 antes de cada prueba
    sim808_init();
}

void tearDown(void) {
    // Opcional: si necesitas limpiar algo después de cada prueba
}

// Prueba 1: Verificar inicialización del módulo SIM808
void test_sim808_init(void) {
    int init_result = sim808_init();
    TEST_ASSERT_TRUE_MESSAGE(init_result, "SIM808 initialized");  // Simplificada como ejemplo
}

// Prueba 2: Verificar la respuesta del módulo SIM808
void test_sim808_send_command_and_read_response(void) {
    // Enviar comando AT y leer la respuesta
    sim808_send_command("AT\r\n");

    char response[64];
    sim808_read_response(response, sizeof(response));

    // Verificar que la respuesta contenga "OK"
    TEST_ASSERT_NOT_NULL(strstr(response, "OK"));
}

// Prueba 3: Verificar el análisis de datos GPS
void test_parse_gps_data(void) {
    // Ejemplo de respuesta del módulo SIM808 que incluye coordenadas GPS
    const char *mock_response = "+CGNSINF: 1,1,20231207083015.000,40.712776,-74.005974,30.0,0.0,0.0,0.0,0.0,0.0,0.0,0.0,0.0";

    GPSData gps_data;
    int parse_result = parse_gps_data(mock_response, &gps_data);

    // Verificar que el análisis fue exitoso y que las coordenadas son correctas
    TEST_ASSERT_TRUE(parse_result);
    TEST_ASSERT_FLOAT_WITHIN(0.0001, 40.712776, gps_data.latitude);
    TEST_ASSERT_FLOAT_WITHIN(0.0001, -74.005974, gps_data.longitude);
    
}