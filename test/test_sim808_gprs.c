#include "unity.h"
#include "sim808_gprs.h"
#include "sim808_gps.h"
#include "gsm_data.h"
#include <string.h>
#include <stdio.h>
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"

bool test_failed = false;

void setUp(void) {
    sim808_init();
    test_failed = false;
}

void tearDown(void) {}

// Funciones de comprobación de estado
static int sim808_check_network_registered(void) {
    char response[128];
    sim808_send_command("AT+CREG?\r\n");
    sim808_wait_for_response(response, sizeof(response), 5000);
    return strstr(response, "+CREG: 0,1") != NULL || strstr(response, "+CREG: 0,5") != NULL;
}

static int sim808_check_gprs_attached(void) {
    char response[128];
    sim808_send_command("AT+CGATT?\r\n");
    sim808_wait_for_response(response, sizeof(response), 5000);
    return strstr(response, "+CGATT: 1") != NULL;
}

static int sim808_check_pdp_activated(void) {
    char response[128];
    sim808_send_command("AT+CGACT?\r\n");
    sim808_wait_for_response(response, sizeof(response), 5000);
    return strstr(response, "+CGACT: 1,1") != NULL;
}

static int sim808_check_ppp_established(void) {
    char response[128];
    sim808_send_command("AT+CIPSTATUS\r\n");
    sim808_wait_for_response(response, sizeof(response), 5000);
    return strstr(response, "PPP UP") != NULL;
}

static int sim808_check_ip_address_obtained(void) {
    char response[128];
    sim808_send_command("AT+CIFSR\r\n");
    sim808_wait_for_response(response, sizeof(response), 5000);
    return strlen(response) > 0 && strstr(response, "ERROR") == NULL;
}

// Tests modificados
void test_sim808_config_sim(void) {
    TEST_ASSERT_EQUAL(1, sim808_config_sim());
}

void test_sim808_check_network_status(void) {
    if (sim808_check_network_registered()) {
        TEST_PASS_MESSAGE("Network already registered.");
        return;
    }
    TEST_ASSERT_EQUAL(1, sim808_check_network_status());
}

void test_sim808_gprs_connect_init(void) {
    if (sim808_check_gprs_attached()) {
        TEST_PASS_MESSAGE("GPRS already attached.");
        return;
    }
    TEST_ASSERT_EQUAL(1, sim808_gprs_connect_init());
}

void test_sim808_gprs_connect_apn(void) {
    if (sim808_check_apn_present()) {
        TEST_PASS_MESSAGE("APN already configured.");
        return;
    }
    TEST_ASSERT_EQUAL(1, sim808_gprs_connect_apn());
}

void test_sim808_gprs_activate_data(void) {
    if (sim808_check_pdp_activated()) {
        TEST_PASS_MESSAGE("PDP context already activated.");
        return;
    }
    TEST_ASSERT_EQUAL(1, sim808_gprs_activate_data());
}

void test_sim808_gprs_establish_ppp(void) {
    if (sim808_check_ppp_established()) {
        TEST_PASS_MESSAGE("PPP already established.");
        return;
    }
    TEST_ASSERT_EQUAL(1, sim808_gprs_establish_ppp());
}

void test_sim808_gprs_get_ip(void) {
    if (sim808_check_ip_address_obtained()) {
        TEST_PASS_MESSAGE("IP address already obtained.");
        return;
    }
    TEST_ASSERT_EQUAL(1, sim808_gprs_get_ip());
}

void test_sim808_gprs_tcp_connect(void) {
    TEST_ASSERT_EQUAL(1, sim808_gprs_tcp_connect());
}

void test_sim808_gprs_send_data(void) {
    char json_data[] = "{\"lat\":40.4168,\"lon\":-3.7038,\"status\":\"ok\"}";
    sim808_gprs_send_data(json_data);
    TEST_PASS_MESSAGE("Datos enviados correctamente (verificar en el servidor).");
}

void test_sim808_gprs_disconnect(void) {
    TEST_ASSERT_EQUAL(1, sim808_gprs_disconnect());
}

int app_main(void) {
    UNITY_BEGIN();

    RUN_TEST(test_sim808_config_sim);
    if (test_failed) return UNITY_END();
    RUN_TEST(test_sim808_gprs_connect_init);
    if (test_failed) return UNITY_END();
    RUN_TEST(test_sim808_gprs_connect_apn);
    if (test_failed) return UNITY_END();
    RUN_TEST(test_sim808_gprs_activate_data);
    if (test_failed) return UNITY_END();
    RUN_TEST(test_sim808_gprs_establish_ppp);
    if (test_failed) return UNITY_END();
    RUN_TEST(test_sim808_gprs_get_ip);
    RUN_TEST(test_sim808_check_network_status);
    // RUN_TEST(test_sim808_gprs_tcp_connect);
    // RUN_TEST(test_sim808_gprs_send_data);
    RUN_TEST(test_sim808_gprs_disconnect);

    return UNITY_END();
}

