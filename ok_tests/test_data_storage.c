#include "unity.h"
#include "data_storage.h"
#include <time.h>

void setUp(void) {
    data_storage_init();
}

void tearDown(void) {
    // No se requiere limpieza después de cada prueba
}

void test_data_storage_init(void) {
    data_storage_init();
    TEST_ASSERT_EQUAL_INT(0, data_storage_get_count());
}

void test_data_storage_save(void) {
    GPSData data = { .latitude = 40.4168, .longitude = -3.7038, .altitude = 667.0 };
    data_storage_save(&data);
    TEST_ASSERT_EQUAL_INT(1, data_storage_get_count());

    DataEntry* entries = data_storage_get_all();
    TEST_ASSERT_EQUAL_FLOAT(40.4168, entries[0].gps_data.latitude);
    TEST_ASSERT_EQUAL_FLOAT(-3.7038, entries[0].gps_data.longitude);
    TEST_ASSERT_EQUAL_FLOAT(667.0, entries[0].gps_data.altitude);
}

void test_data_storage_overflow(void) {
/* Esta prueba verifica que el sistema de almacenamiento de datos maneje correctamente el 
desbordamiento, es decir, que no se exceda la capacidad máxima de almacenamiento y que los 
datos más recientes sobrescriban los más antiguos.*/
    GPSData data = { .latitude = 0.0, .longitude = 0.0, .altitude = 0.0 };
    for (int i = 0; i < STORAGE_SIZE + 5; i++) {
        data.latitude = i;
        data_storage_save(&data);
    }

    TEST_ASSERT_EQUAL_INT(STORAGE_SIZE, data_storage_get_count());
    DataEntry* entries = data_storage_get_all();
    TEST_ASSERT_EQUAL_FLOAT(5.0, entries[0].gps_data.latitude);
}

void test_data_storage_timestamps(void) {
    GPSData data = { .latitude = 51.5074, .longitude = -0.1278, .altitude = 35.0 };
    data_storage_save(&data);

    DataEntry* entries = data_storage_get_all();
    TEST_ASSERT_TRUE(entries[0].timestamp > 0);
}

int app_main(void) {
    UNITY_BEGIN();
    RUN_TEST(test_data_storage_init);
    RUN_TEST(test_data_storage_save);
    RUN_TEST(test_data_storage_overflow);
    RUN_TEST(test_data_storage_timestamps);
    return UNITY_END();
}