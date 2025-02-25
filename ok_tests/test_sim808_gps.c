#include "unity.h"
#include "sim808_gps.h"
#include <string.h>

GPSData test_data;

void setUp(void) {
    // Inicialización antes de cada prueba
}

void tearDown(void) {
    // Limpieza después de cada prueba
}

void test_sim808_init(void) {
    TEST_ASSERT_EQUAL_INT(1, sim808_init());
}

void test_sim808_send_and_read_command(void) {
    char buffer[128];
    sim808_send_command("AT\r\n");   
    sim808_read_response(buffer, sizeof(buffer));
    // Verifica si el buffer contiene algún dato (no es NULL)
    TEST_ASSERT_NOT_NULL(buffer);

    // Verifica si el buffer contiene al menos un caracter (no es una cadena vacía)
    TEST_ASSERT_NOT_EQUAL(0, strlen(buffer));

    // Verifica si el buffer contiene "OK" (o la respuesta esperada)
    TEST_ASSERT_NOT_NULL(strstr(buffer, "OK"));
}

void test_sim808_gps_reset_mode(void) {
    sim808_gps_reset_mode(0); // Hot reset
    // Validación manual si es necesario
}

void test_sim808_gps_get_status(void) {
    int status = sim808_gps_get_status();
    TEST_ASSERT_TRUE(status >= 1 && status <= 3);

}

void test_sim808_get_gps_data(void) {
    int result = sim808_get_gps_data(&test_data);
    TEST_ASSERT_EQUAL_INT(1, result);
    TEST_ASSERT_FLOAT_WITHIN(0.01, 42.21, test_data.latitude); // Valores reales
    TEST_ASSERT_FLOAT_WITHIN(0.01, -8.72, test_data.longitude);
 
}


void test_sim808_get_battery_status(void) {
    int result = sim808_get_battery_status(&test_data);
    TEST_ASSERT_EQUAL_INT(1, result);
    TEST_ASSERT_TRUE(test_data.battery_voltage > 0);
   
}

int app_main(void) {
    UNITY_BEGIN();

    RUN_TEST(test_sim808_init);
    RUN_TEST(test_sim808_send_and_read_command);
    RUN_TEST(test_sim808_gps_reset_mode);
    RUN_TEST(test_sim808_gps_get_status);
    RUN_TEST(test_sim808_get_gps_data);
    RUN_TEST(test_sim808_get_battery_status);

    return UNITY_END();
}
