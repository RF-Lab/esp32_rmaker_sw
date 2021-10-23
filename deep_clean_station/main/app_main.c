/* Temperature Sensor Example

   This example code is in the Public Domain (or CC0 licensed, at your option.)

   Unless required by applicable law or agreed to in writing, this
   software is distributed on an "AS IS" BASIS, WITHOUT WARRANTIES OR
   CONDITIONS OF ANY KIND, either express or implied.
*/

// DHT drivers is here: https://github.com/UncleRus/esp-idf-lib

#include <string.h>
#include <freertos/FreeRTOS.h>
#include <freertos/task.h>
#include <esp_log.h>
#include <nvs_flash.h>

#include <esp_rmaker_core.h>
#include <esp_rmaker_standard_params.h>
#include <esp_rmaker_standard_devices.h>

#include <app_wifi.h>

#include "app_priv.h"

#include <wifi_provisioning/manager.h>

static const char *TAG = "app_main";

#define TEMP_SENSOR_TAG         "Температура"
#define RSSI_MONITOR_TAG        "RSSI"
#define AIR_PUMP_TAG            "Воздушный компрессор"
#define WATER_PUMP_TAG          "Насос"
#define HEAT_CABLE_TAG          "Подогрев трубы"


esp_rmaker_device_t *temp_sensor_device ;
esp_rmaker_device_t *rssi_monitor_device ;
esp_rmaker_device_t *air_pump_device ;
esp_rmaker_device_t *water_pump_device ;
esp_rmaker_device_t *heat_cable_device ;

/* Callback to handle commands received from the RainMaker cloud */
static esp_err_t write_cb(const esp_rmaker_device_t *device, const esp_rmaker_param_t *param,
            const esp_rmaker_param_val_t val, void *priv_data, esp_rmaker_write_ctx_t *ctx)
{
    const char *device_name = esp_rmaker_device_get_name(device) ;
    const char *param_name = esp_rmaker_param_get_name(param) ;
    ESP_LOGI(TAG, "write_cb: device_name=[%s] param_name=[%s] val.val.b=[%d]", device_name, param_name, val.val.b ) ;
    if (strcmp(device_name, AIR_PUMP_TAG ) == 0) 
    {
        gpio_set_level(GPIO_LED, (val.val.b)?1:0 ) ;
        gpio_set_level(AIR_PUMP_PIN, (val.val.b)?1:0 ) ;                        
    } 
    else if (strcmp(device_name, WATER_PUMP_TAG ) == 0)
    {
        gpio_set_level(WATER_PUMP_PIN, (val.val.b)?1:0 ) ;
    }
    else if (strcmp(device_name, HEAT_CABLE_TAG ) == 0)
    {
        gpio_set_level(HEAT_CABLE_PIN, (val.val.b)?1:0 ) ;
    }
    else
    {
        ESP_LOGI(TAG, "write_cb: Unknown device name" ) ;
        return (ESP_OK) ;
    }
    
    esp_rmaker_param_update_and_report(param, val) ;
    
    return (ESP_OK) ;

}

void app_main()
{
    /* Initialize Application specific hardware drivers and
     * set initial state.
     */
    app_driver_init() ;

    /* Initialize NVS. */
    esp_err_t err = nvs_flash_init() ;
    if (err == ESP_ERR_NVS_NO_FREE_PAGES || err == ESP_ERR_NVS_NEW_VERSION_FOUND) {
        ESP_ERROR_CHECK(nvs_flash_erase());
        err = nvs_flash_init();
    }
    ESP_ERROR_CHECK( err ) ;

    /* Initialize Wi-Fi. Note that, this should be called before esp_rmaker_init()
     */
    app_wifi_init() ;
    
    /* Initialize the ESP RainMaker Agent.
     * Note that this should be called after app_wifi_init() but before app_wifi_start()
     * */
    esp_rmaker_config_t rainmaker_cfg = {
        .enable_time_sync = false,
    };
    esp_rmaker_node_t *node = esp_rmaker_node_init(&rainmaker_cfg, "Управление септиком", "Септик") ;
    if (!node) 
    {
        ESP_LOGE(TAG, "Could not initialise node. Aborting!!!") ;
        vTaskDelay(5000/portTICK_PERIOD_MS) ;
        abort() ;
    }

    /* Create a device and add the relevant parameters to it */
    temp_sensor_device = esp_rmaker_temp_sensor_device_create(TEMP_SENSOR_TAG, NULL, app_get_current_temperature()) ;
    esp_rmaker_node_add_device(node, temp_sensor_device) ;

    rssi_monitor_device = esp_rmaker_temp_sensor_device_create(RSSI_MONITOR_TAG, NULL, 0 ) ;
    esp_rmaker_node_add_device(node, rssi_monitor_device) ;

    /* create switch */
    air_pump_device = esp_rmaker_switch_device_create(AIR_PUMP_TAG, NULL, false ) ;
    esp_rmaker_device_add_cb(air_pump_device, write_cb, NULL) ;
    esp_rmaker_node_add_device( node, air_pump_device ) ;

    water_pump_device = esp_rmaker_switch_device_create(WATER_PUMP_TAG, NULL, false ) ;
    esp_rmaker_device_add_cb(water_pump_device, write_cb, NULL) ;
    esp_rmaker_node_add_device(node, water_pump_device ) ;

    heat_cable_device = esp_rmaker_switch_device_create(HEAT_CABLE_TAG, NULL, false ) ;
    esp_rmaker_device_add_cb(heat_cable_device, write_cb, NULL) ;
    esp_rmaker_node_add_device(node, heat_cable_device ) ;

    /* Start the ESP RainMaker Agent */
    esp_rmaker_start() ;

    // Call it to erase wifi credentials
    //wifi_prov_mgr_reset_provisioning() ;

    /* Start the Wi-Fi.
     * If the node is provisioned, it will start connection attempts,
     * else, it will start Wi-Fi provisioning. The function will return
     * after a connection has been successfully established
     */
    err = app_wifi_start(POP_TYPE_RANDOM) ;
    if (err != ESP_OK) 
    {
        ESP_LOGE(TAG, "Could not start Wifi. Aborting!!!") ;
        vTaskDelay(5000/portTICK_PERIOD_MS) ;
        abort() ;
    }
}
