#include "unity.h"
#include "i2c_com.h"
#include "driver/i2c.h"

void setUp(void) {}
void tearDown(void) {}

void test_i2c_master_init(void) {
    esp_err_t ret = i2c_master_init();
    TEST_ASSERT_EQUAL(ESP_OK, ret);
}

void test_ina219_read_register(void) {
    uint16_t data;
    esp_err_t ret = ina219_read_register(INA219_REG_BUS_VOLTAGE, &data);
    TEST_ASSERT_EQUAL(ESP_OK, ret);
    TEST_ASSERT_NOT_EQUAL(0, data);
}

void test_ina219_get_shunt_voltage(void) {
    float shunt_voltage = ina219_get_shunt_voltage();
    TEST_ASSERT_TRUE(shunt_voltage >= -320.0f && shunt_voltage <= 320.0f);
}

void test_ina219_get_bus_voltage(void) {
    float bus_voltage = ina219_get_bus_voltage();
    TEST_ASSERT_TRUE(bus_voltage >= 0.0f && bus_voltage <= 32.0f);
}

void test_ina219_get_current(void) {
    float current = ina219_get_current();
    TEST_ASSERT_TRUE(current >= -3.2f && current <= 3.2f);
}

void test_ina219_get_power(void) {
    float power = ina219_get_power();
    TEST_ASSERT_TRUE(power >= 0.0f);
}

void app_main(void) {
    UNITY_BEGIN();
    RUN_TEST(test_i2c_master_init);
    RUN_TEST(test_ina219_read_register);
    RUN_TEST(test_ina219_get_shunt_voltage);
    RUN_TEST(test_ina219_get_bus_voltage);
    RUN_TEST(test_ina219_get_current);
    RUN_TEST(test_ina219_get_power);
    UNITY_END();
}
