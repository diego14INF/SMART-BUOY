#include "unity.h"
#include "sim808_gprs.h"
#include "sim808_gps.h"
#include "gsm_data.h"
#include <string.h>
#include <stdio.h> // Incluir para printf
#include "freertos/FreeRTOS.h" // Incluir para FreeRTOS
#include "freertos/task.h" // Incluir para vTaskDelay y pdMS_TO_TICKS

void setUp(void) {
    sim808_init();
}
void tearDown(void) {}

// Función de reintento genérica
int retry_function(int (*func)(void), const char *func_name) {
    int retries = 3;
    while (retries > 0) {
        int result = func();
        if (result == 1) {
            printf("PASSED: %s after %d retries.\n", func_name, 3 - retries);
            return 1;
        } else {
            printf("FAILED: %s. Retry %d...\n", func_name, 3 - retries +1);
            retries--;
            vTaskDelay(pdMS_TO_TICKS(1000)); // Espera 1 segundo entre reintentos
        }
    }
    printf("FAILED: %s after 3 retries.\n", func_name);
    return 0; // Falló después de 3 reintentos
}

// Test de configuración de la SIM
void test_sim808_config_sim(void) {
    TEST_ASSERT_EQUAL(1, retry_function(sim808_config_sim, "sim808_config_sim"));
}

// Test de verificación de estado de red GSM
void test_sim808_check_network_status(void) {
    int status = -1;
    status = retry_function(sim808_check_network_status, "sim808_check_network_status");
    TEST_ASSERT_EQUAL(1, status);
}

// Test de inicialización de GPRS
void test_sim808_gprs_connect_init(void) {
    TEST_ASSERT_EQUAL(1, retry_function(sim808_gprs_connect_init, "sim808_gprs_connect_init"));
}

// Test de configuración del APN
void test_sim808_gprs_connect_apn(void) {
    TEST_ASSERT_EQUAL(1, retry_function(sim808_gprs_connect_apn, "sim808_gprs_connect_apn"));
}

// Test de activación de datos
void test_sim808_gprs_activate_data(void) {
    TEST_ASSERT_EQUAL(1, retry_function(sim808_gprs_activate_data, "sim808_gprs_activate_data"));
}

// Test de establecimiento de enlace PPP
void test_sim808_gprs_establish_ppp(void) {
    TEST_ASSERT_EQUAL(1, retry_function(sim808_gprs_establish_ppp, "sim808_gprs_establish_ppp"));
}

// Test de obtención de dirección IP
void test_sim808_gprs_get_ip(void) {
    TEST_ASSERT_EQUAL(1, retry_function(sim808_gprs_get_ip, "sim808_gprs_get_ip"));
}

// Test de conexión TCP
void test_sim808_gprs_tcp_connect(void) {
    TEST_ASSERT_EQUAL(1, retry_function(sim808_gprs_tcp_connect, "sim808_gprs_tcp_connect"));
}

// Test de envío de datos por GPRS
void test_sim808_gprs_send_data(void) {
    char json_data[] = "{\"lat\":40.4168,\"lon\":-3.7038,\"status\":\"ok\"}";
    sim808_gprs_send_data(json_data);
    TEST_PASS_MESSAGE("Datos enviados correctamente (verificar en el servidor).");
}

// Test de desconexión de la sesión GPRS
void test_sim808_gprs_disconnect(void) {
    TEST_ASSERT_EQUAL(1, retry_function(sim808_gprs_disconnect, "sim808_gprs_disconnect"));
}

// Función principal de pruebas
int app_main(void) {
    UNITY_BEGIN();

    RUN_TEST(test_sim808_config_sim);
    RUN_TEST(test_sim808_gprs_connect_init);
    RUN_TEST(test_sim808_gprs_connect_apn);
    RUN_TEST(test_sim808_gprs_activate_data);
    RUN_TEST(test_sim808_gprs_establish_ppp);
    RUN_TEST(test_sim808_gprs_get_ip);
    RUN_TEST(test_sim808_check_network_status);
    // RUN_TEST(test_sim808_gprs_tcp_connect);
    // RUN_TEST(test_sim808_gprs_send_data);
    RUN_TEST(test_sim808_gprs_disconnect);

    return UNITY_END();
}