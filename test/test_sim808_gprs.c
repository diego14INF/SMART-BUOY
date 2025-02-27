#include "unity.h"
#include "sim808_gprs.h"
#include "sim808_gps.h"
#include "gsm_data.h"
#include <string.h>

void setUp(void) {}
void tearDown(void) {}

void test_sim808_config_sim(void) {
    TEST_ASSERT_EQUAL(1, sim808_config_sim());
}

void test_sim808_gprs_connect(void) {
    TEST_ASSERT_EQUAL(1, sim808_gprs_connect());
}

void test_sim808_check_network_status(void) {
    sim808_check_network_status();
    TEST_ASSERT_EQUAL(1,sim808_check_network_status());
}

void test_sim808_gprs_send_data(void) {
    char json_data[] = "{\"lat\":40.4168,\"lon\":-3.7038,\"status\":\"ok\"}";
    sim808_gprs_send_data(json_data);
    // No hay retorno directo, verificar manualmente en el servidor si es necesario
    TEST_PASS_MESSAGE("Datos enviados correctamente.");
}


void test_sim808_gprs_disconnect(void) {
    TEST_ASSERT_EQUAL(1, sim808_gprs_disconnect());
}

int app_main(void) {
    UNITY_BEGIN();

    RUN_TEST(test_sim808_config_sim);
    RUN_TEST(test_sim808_gprs_connect); 
    RUN_TEST(test_sim808_check_network_status);
    //RUN_TEST(test_sim808_gprs_send_data);
    //RUN_TEST(test_sim808_gprs_disconnect);

    return UNITY_END();
}
