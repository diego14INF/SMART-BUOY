#include "sim808_gps.h"
#include "data_storage.h"
#include "gsm_data.h"
#include <assert.h>
#include "sim808_gprs.h"
#include "unity.h"

void test_gps_data_adquisition() {
    GPSData gps_data;
    int result = sim808_get_gps_data(&gps_data);
    assert(result == 1);
}

void test_battery_status() {
    GPSData gps_data;
    int result = sim808_get_battery_status(&gps_data);
    assert(result == 1);
}

void test_data_storage() {
    data_storage_init();
    GPSData gps_data = {20241119183045.000, 42.210323, -8.722028, 30.0, 0.1, 359.0, 100, 4200};
    data_storage_save(&gps_data);
    assert(data_storage_get_count() == 1);
}

void test_gsm_data_preparation() {
    char buffer[1024];
    int result = gsm_prepare_data(buffer, sizeof(buffer));
    assert(result == 1);
    printf("Prepared data:\n%s\n", buffer);
}

// void test_gsm_init(){
//     int result= sim808_gprs_connect();
//     assert(result==1);
// }


void app_main()
{
  UNITY_BEGIN();
    test_gps_data_adquisition();
    test_battery_status();
    test_data_storage();
    //test_gsm_data_preparation();
    //test_gsm_init();
   
    UNITY_END();
}