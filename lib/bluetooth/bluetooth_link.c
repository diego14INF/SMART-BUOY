#include "bluetooth_link.h"
#include "esp_log.h"
#include "nvs_flash.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "driver/gpio.h"
#include "esp_http_client.h"
#include <string.h>
//#include "esp_gap_bt.h"

static const char *TAG = "Bluetooth Link";
static bool linking_mode = false;

// // Callback para eventos GAP
// static void gap_callback(esp_bt_gap_cb_event_t event, esp_bt_gap_cb_param_t *param) {
//     switch (event) {
//         case ESP_BT_GAP_DISC_RES_EVT:
//             ESP_LOGI(TAG, "Dispositivo encontrado en escaneo.");
//             break;
//         case ESP_BT_GAP_DISC_STATE_CHANGED:
//             ESP_LOGI(TAG, "Estado de escaneo cambiado.");
//             break;
//         default:
//             break;
//     }
// }

// // Inicializa el Bluetooth
// esp_err_t bluetooth_init(void) {
//     ESP_ERROR_CHECK(nvs_flash_init());
//     ESP_ERROR_CHECK(esp_bt_controller_mem_release(ESP_BT_MODE_BLE));
//     esp_bt_controller_config_t bt_cfg = BT_CONTROLLER_INIT_CONFIG_DEFAULT();
//     ESP_ERROR_CHECK(esp_bt_controller_init(&bt_cfg));
//     ESP_ERROR_CHECK(esp_bt_controller_enable(ESP_BT_MODE_CLASSIC_BT));
//     ESP_ERROR_CHECK(esp_bluedroid_init());
//     ESP_ERROR_CHECK(esp_bluedroid_enable());
//     ESP_ERROR_CHECK(esp_bt_gap_register_callback(gap_callback));
//     ESP_ERROR_CHECK(esp_bt_gap_start_discovery(ESP_BT_INQ_MODE_GENERAL_INQUIRY, 10, 0));
//     ESP_LOGI(TAG, "Bluetooth inicializado correctamente.");
//     return ESP_OK;
// }

// // Verifica si el botón está presionado por 10 segundos
// void check_button_press(void) {
//     gpio_pad_select_gpio(BUTTON_GPIO);
//     gpio_set_direction(BUTTON_GPIO, GPIO_MODE_INPUT);

//     while (1) {
//         if (gpio_get_level(BUTTON_GPIO) == 0) {
//             vTaskDelay(BUTTON_HOLD_TIME / portTICK_PERIOD_MS);
//             if (gpio_get_level(BUTTON_GPIO) == 0) {
//                 ESP_LOGI(TAG, "Modo enlace activado.");
//                 linking_mode = true;
//                 scan_nearby_buoys();
//             }
//         }
//         vTaskDelay(100 / portTICK_PERIOD_MS);
//     }
// }

// // Escanea boyas cercanas
// void scan_nearby_buoys(void) {
//     if (linking_mode) {
//         ESP_LOGI(TAG, "Escaneando boyas cercanas...");
//         esp_bt_gap_start_discovery(ESP_BT_INQ_MODE_GENERAL_INQUIRY, 10, 0);
//     }
// }

// // Inicia emparejamiento con otra boya
// void start_pairing(const char *remote_id) {
//     ESP_LOGI(TAG, "Emparejando con boya ID: %s", remote_id);
//     exchange_data_with_server(remote_id);
// }

// Intercambio de datos con el servidor
// esp_err_t exchange_data_with_server(const char *buoy_id) {
//     ESP_LOGI(TAG, "Enviando ID %s al servidor.", buoy_id);
//     esp_http_client_config_t config = {
//         .url = SERVER_URL,
//         .method = HTTP_METHOD_POST,
//     };
//     esp_http_client_handle_t client = esp_http_client_init(&config);
//     esp_http_client_perform(client);
//     esp_http_client_cleanup(client);
//     return ESP_OK;
// }
