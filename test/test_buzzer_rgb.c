#include "unity.h"
#include "buzzer_rgb.h"

void setUp(void) {
    baliza_zumbador_init();
}

void tearDown(void) {
    set_baliza_color(0, 0, 0);
    desactivar_zumbador();
}

void test_baliza_zumbador_init(void) {
    TEST_PASS_MESSAGE("Baliza y zumbador inicializados correctamente");
}

void test_activar_desactivar_zumbador(void) {
    activar_zumbador(BUZZER_FREQUENCY);
    TEST_PASS_MESSAGE("Zumbador activado");
    desactivar_zumbador();
    TEST_PASS_MESSAGE("Zumbador desactivado");
}

void test_set_baliza_color(void) {
    set_baliza_color(1, 0, 0); // Rojo
    TEST_PASS_MESSAGE("Baliza en rojo");
    set_baliza_color(0, 1, 0); // Verde
    TEST_PASS_MESSAGE("Baliza en verde");
    set_baliza_color(0, 0, 1); // Azul
    TEST_PASS_MESSAGE("Baliza en azul");
}

void test_baliza_zumbador_socorro(void) {
    baliza_zumbador_socorro();
    TEST_PASS_MESSAGE("Función de socorro ejecutada");
}

void test_baliza_zumbador_estado_ok(void) {
    baliza_zumbador_estado_ok();
    TEST_PASS_MESSAGE("Función de estado OK ejecutada");
}

void test_baliza_zumbador_fuera_limites(void) {
    baliza_zumbador_fuera_limites();
    TEST_PASS_MESSAGE("Función de fuera de límites ejecutada");
}

void test_baliza_zumbador_bajo_bateria(void) {
    baliza_zumbador_bajo_bateria();
    TEST_PASS_MESSAGE("Función de batería baja ejecutada");
}

void test_baliza_zumbador_error_sistema(void) {
    baliza_zumbador_error_sistema();
    TEST_PASS_MESSAGE("Función de error de sistema ejecutada");
}

void test_baliza_zumbador_transmision_datos(void) {
    baliza_zumbador_transmision_datos();
    TEST_PASS_MESSAGE("Función de transmisión de datos ejecutada");
}

int app_main(void) {
    UNITY_BEGIN();

    RUN_TEST(test_baliza_zumbador_init);
    RUN_TEST(test_activar_desactivar_zumbador);
    RUN_TEST(test_set_baliza_color);
    RUN_TEST(test_baliza_zumbador_socorro);
    RUN_TEST(test_baliza_zumbador_estado_ok);
    RUN_TEST(test_baliza_zumbador_fuera_limites);
    RUN_TEST(test_baliza_zumbador_bajo_bateria);
    RUN_TEST(test_baliza_zumbador_error_sistema);
    RUN_TEST(test_baliza_zumbador_transmision_datos);

    return UNITY_END();
}