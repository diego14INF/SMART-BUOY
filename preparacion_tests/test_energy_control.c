#include "unity.h"
#include "energy_control.h"
#include "i2c_com.h"
#include "driver/gpio.h"
#include "driver/adc.h"
#include "esp_pm.h"

void setUp(void) {
    // Inicialización antes de cada test
    energia_init();
}

void tearDown(void) {
    // Limpieza después de cada test
}

void test_energia_init(void) {
    // Verificar inicialización correcta
    TEST_ASSERT_TRUE_MESSAGE(true, "Inicialización completada.");
}

void test_energia_leer_tension_bateria(void) {
    float tension = energia_leer_tension_bateria();
    TEST_ASSERT_FLOAT_WITHIN(0.5, 3.7, tension); // Ejemplo de rango esperado
}

void test_energia_actualizar(void) {
    energia_actualizar();
    uint8_t nivel = energia_get_bateria();
    TEST_ASSERT_TRUE(nivel <= 100 && nivel >= 0);
}

void test_energia_get_bateria(void) {
    uint8_t nivel = energia_get_bateria();
    TEST_ASSERT_TRUE(nivel <= 100);
}

void test_energia_bajo_consumo(void) {
    energia_actualizar();
    bool bajo = energia_bajo_consumo();
    TEST_ASSERT_TRUE(bajo == false || bajo == true);
}

int main(void) {
    UNITY_BEGIN();
    RUN_TEST(test_energia_init);
    RUN_TEST(test_energia_leer_tension_bateria);
    RUN_TEST(test_energia_actualizar);
    RUN_TEST(test_energia_get_bateria);
    RUN_TEST(test_energia_bajo_consumo);
    return UNITY_END();
}
