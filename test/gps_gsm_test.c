#include "unity.h"
#include "gps_state_machine.h"
#include "sim808_gps.h"
#include "timers.h"
#include "data_storage.h"
#include "sim808_gprs.h"

// // Variables para mocks
// static int mock_gps_status = 0;
// static int mock_save_result = 1;
// static GPSData mock_gps_data;

// // Funciones simuladas
// int sim808_gps_get_status() {
//     return mock_gps_status;
// }

// void sim808_gps_reset_mode(int reset_type) {
//     printf("Simulado: Reiniciando GPS con tipo %d\n", reset_type);
// }

// bool sim808_get_gps_data(GPSData *data) {
//     *data = mock_gps_data;
//     return true; // Simula éxito siempre
// }

// bool sim808_get_battery_status(GPSData *data) {
//     data->battery_voltage = 80; // Simula un nivel de batería válido
//     return true; // Simula éxito siempre
// }

// bool data_storage_save(const GPSData *data) {
//     return mock_save_result;
// }

// int data_storage_get_count() {
//     return 10; // Simula 10 entradas guardadas
// }

// void data_storage_init() {
//     printf("Simulado: Inicializando almacenamiento de datos.\n");
// }

// void init_timer(int timer_interval_sec) {
//     printf("Simulado: Iniciando temporizador por %d segundos.\n", timer_interval_sec);
// }

// bool is_timer_finished() {
//     return true; // Simula que siempre finalizó el temporizador
// }

// // Setup para pruebas
// void setUp(void) {
//     gps_state_machine_init();
//     mock_gps_status = 0;
//     mock_save_result = 1;
//     mock_gps_data.timestamp = 123456789;
//     mock_gps_data.latitude = 10.123456;
//     mock_gps_data.longitude = -20.123456;
//     mock_gps_data.altitude = 100.0;
//     mock_gps_data.speed = 50.0;
//     mock_gps_data.course = 180.0;
// }

// // Tests
// void test_verify_gps_ok(void) {
//     mock_gps_status = 1; // Simula GPS OK
//     gps_state_machine_run();
//     TEST_ASSERT_EQUAL(STATE_ACQUIRE_DATA, current_state);
// }

// void test_verify_gps_retry(void) {
//     mock_gps_status = 0; // Simula GPS no OK
//     gps_state_machine_run();
//     TEST_ASSERT_EQUAL(STATE_VERIFY_GPS, current_state);
//     TEST_ASSERT_EQUAL(1, gps_retry_count); // Incrementa el contador
// }

// void test_verify_gps_max_retries(void) {
//     mock_gps_status = 0; // Simula GPS no OK
//     for (int i = 0; i <= MAX_GPS_RETRIES; i++) {
//         gps_state_machine_run();
//     }
//     TEST_ASSERT_EQUAL(STATE_ERROR, current_state); // Debería entrar en error
// }

// void test_acquire_data_success(void) {
//     mock_gps_status = 1; // GPS OK
//     gps_state_machine_run(); // Transición a ACQUIRE_DATA
//     gps_state_machine_run(); // Simula adquisición exitosa
//     TEST_ASSERT_EQUAL(STATE_STORE_DATA, current_state);
// }

// void test_acquire_data_fail(void) {
//     mock_gps_status = 1; // GPS OK
//     gps_state_machine_run(); // Transición a ACQUIRE_DATA
//     for (int i = 0; i <= MAX_ACQUISITION_RETRIES; i++) {
//         mock_save_result = 0; // Simula fallo en adquisición
//         gps_state_machine_run();
//     }
//     TEST_ASSERT_EQUAL(STATE_ERROR, current_state);
// }

// void test_store_data_success(void) {
//     mock_gps_status = 1; // GPS OK
//     gps_state_machine_run(); // Transición a ACQUIRE_DATA
//     gps_state_machine_run(); // Simula adquisición exitosa
//     gps_state_machine_run(); // Simula almacenamiento exitoso
//     TEST_ASSERT_EQUAL(STATE_VERIFY_GPS, current_state); // Reinicia flujo
// }

// void test_store_data_fail(void) {
//     mock_gps_status = 1; // GPS OK
//     gps_state_machine_run(); // Transición a ACQUIRE_DATA
//     gps_state_machine_run(); // Simula adquisición exitosa
//     mock_save_result = 0; // Simula fallo en almacenamiento
//     gps_state_machine_run();
//     TEST_ASSERT_EQUAL(STATE_ERROR, current_state);
// }


//Prueba de conexión GPRS

void test_sim808_gprs_innit(void){
     char response[64];
    sim808_send_command("AT+CGATT=1\r\n" );
    sim808_read_response(response, sizeof(response));
    TEST_ASSERT_EQUAL(0, strcmp("OK", response));
}

void test_sim808_gprs_config_APN(void){
     char response[64];
    sim808_send_command("AT+CGDCONT=1,\"IP\",\"" APN "\"\r\n" );
    sim808_read_response(response, sizeof(response));
    TEST_ASSERT_EQUAL(0, strcmp("OK", response));
}

void test_sim808_gprs_status_pdp(void){ //Packet data control
  char response[256];
sim808_send_command("AT+CGACT?\r\n");
sim808_read_response(response, sizeof(response)); //Respuesta típica +CGACT: <cid>,<state> OK

}


// Main de pruebas
void app_main(void) {
    UNITY_BEGIN();

    // RUN_TEST(test_verify_gps_ok);
    // RUN_TEST(test_verify_gps_retry);
    // RUN_TEST(test_verify_gps_max_retries);
    // RUN_TEST(test_acquire_data_success);
    // RUN_TEST(test_acquire_data_fail);
    // RUN_TEST(test_store_data_success);
    // RUN_TEST(test_store_data_fail);
    RUN_TEST(test_sim808_gprs_innit);
    RUN_TEST(test_sim808_gprs_config_APN);



    UNITY_END();
}
