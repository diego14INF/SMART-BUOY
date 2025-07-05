#include "bluetooth_link.h"
#include "esp_log.h"
#include "nvs_flash.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "driver/gpio.h"
#include "esp_http_client.h"
#include <string.h>
#include "data_storage.h"
// #include "esp_bt.h"
// #include "esp_bt_main.h"
// #include "esp_gap_bt_api.h"
// #include "esp_bt_device.h"
// #include "esp_spp_api.h"

// static const char *TAG = "Bluetooth Link";
// static bool linking_mode = false;
// static esp_bd_addr_t remote_bda = {0};
// static long long remote_mmsi = 0;

// // Callback para eventos GAP (descubrimiento)
// static void gap_callback(esp_bt_gap_cb_event_t event, esp_bt_gap_cb_param_t *param) {
//     if (event == ESP_BT_GAP_DISC_RES_EVT && linking_mode) {
//         // Tomamos la primera dirección encontrada
//         memcpy(remote_bda, param->disc_res.bda, ESP_BD_ADDR_LEN);
//         ESP_LOGI(TAG, "Boya encontrada: %02x:%02x:%02x:%02x:%02x:%02x",
//                  remote_bda[0], remote_bda[1], remote_bda[2],
//                  remote_bda[3], remote_bda[4], remote_bda[5]);

//         // Detener escaneo
//         esp_bt_gap_cancel_discovery();
//         linking_mode = false;

//         // Aquí podrías, por ejemplo, leer SDP o un nombre que contenga el MMSI remoto...
//         // Por simplicidad vamos a usar un valor fijo o bien asumimos que ya conoces remote_mmsi
//         remote_mmsi = 123456789LL; // Debes sustituirlo o extraerlo realmente

//         enlazar_boya(remote_mmsi);
//     }
// }

// // Inicializa el Bluetooth Classic
// esp_err_t bluetooth_init(void) {
//     ESP_ERROR_CHECK(nvs_flash_init());
//     ESP_ERROR_CHECK(esp_bt_controller_mem_release(ESP_BT_MODE_BLE));
//     esp_bt_controller_config_t bt_cfg = BT_CONTROLLER_INIT_CONFIG_DEFAULT();
//     ESP_ERROR_CHECK(esp_bt_controller_init(&bt_cfg));
//     ESP_ERROR_CHECK(esp_bt_controller_enable(ESP_BT_MODE_CLASSIC_BT));
//     ESP_ERROR_CHECK(esp_bluedroid_init());
//     ESP_ERROR_CHECK(esp_bluedroid_enable());

//     // Registrar GAP callback
//     ESP_ERROR_CHECK(esp_bt_gap_register_callback(gap_callback));
//     ESP_LOGI(TAG, "Bluetooth inicializado OK");
//     return ESP_OK;
// }

// // Escanea boyas cercanas
// void escanear_boyas_cercanas(void) {
//     if (!linking_mode) {
//         linking_mode = true;
//         ESP_LOGI(TAG, "Iniciando escaneo de boyas...");
//         esp_bt_gap_start_discovery(ESP_BT_INQ_MODE_GENERAL_INQUIRY, 10, 0);
//     }
// }

// // Empareja con la boya encontrada, compartiendo ambos MMSI
// void enlazar_boya(long long int mmsi_remoto) {
//     // Supón que tienes una función para enviar por SPP:
//     // send_spp_data(buffer, length);
//     char payload[64];
//     snprintf(payload, sizeof(payload),
//              "{\"self_mmsi\":%lli,\"remote_mmsi\":%lli}\n",
//              (long long)MMSI_UNICO, mmsi_remoto);
//     ESP_LOGI(TAG, "Enlazando boyas: %s", payload);

    // Inicia conexión SPP con remote_bda, luego enviar payload...
    // esp_spp_connect(..., remote_bda, ...);
    // send_spp_data((uint8_t*)payload, strlen(payload));
//}
