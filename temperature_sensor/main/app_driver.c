/*  Temperature Sensor demo implementation using RGB LED and timer

   This example code is in the Public Domain (or CC0 licensed, at your option.)

   Unless required by applicable law or agreed to in writing, this
   software is distributed on an "AS IS" BASIS, WITHOUT WARRANTIES OR
   CONDITIONS OF ANY KIND, either express or implied.
*/

#include <sdkconfig.h>
#include <esp_rmaker_core.h>
#include <esp_rmaker_standard_types.h> 
#include <esp_rmaker_standard_params.h> 

#include <app_reset.h>
#include <ws2812_led.h>
#include "app_priv.h"

#include "dht.h"
#include <esp_log.h>


static const char *TAG = "app_driver";

/* This is the button that is used for toggling the power */
#define BUTTON_GPIO          0
#define BUTTON_ACTIVE_LEVEL  0
/* This is the GPIO on which the power will be set */
#define OUTPUT_GPIO    19

static esp_timer_handle_t sensor_timer ;

#define DEFAULT_SATURATION  100
#define DEFAULT_BRIGHTNESS  50

#define WIFI_RESET_BUTTON_TIMEOUT       3
#define FACTORY_RESET_BUTTON_TIMEOUT    10

//static uint16_t g_hue;
//static uint16_t g_saturation = DEFAULT_SATURATION;
//static uint16_t g_value = DEFAULT_BRIGHTNESS;
static float g_temperature ;

static void app_sensor_update(void *priv)
{
    /*
    static float delta = 0.5;
    g_temperature += delta;
    if (g_temperature > 99) {
        delta = -0.5;
    } else if (g_temperature < 1) {
        delta = 0.5;
    }
    g_hue = (100 - g_temperature) * 2;
    ws2812_led_set_hsv(g_hue, g_saturation, g_value);
    esp_rmaker_param_update_and_report(
            esp_rmaker_device_get_param_by_type(temp_sensor_device, ESP_RMAKER_PARAM_TEMPERATURE),
            esp_rmaker_float(g_temperature));
            */
    float humidity = 0 ;
    float temperature = 0 ;
    //struct dht11_reading data ;

    ESP_LOGI(TAG, "app_sensor_update() called\n") ;

    if (dht_read_float_data( DHT_TYPE_AM2301, GPIO_DHT2,
        &humidity, &temperature) ==ESP_OK )
    {
        g_temperature       =   temperature ;
        ESP_LOGI(TAG, "Read temp %5.2f\n", temperature ) ;
        esp_rmaker_param_update_and_report(
                esp_rmaker_device_get_param_by_type(temp_sensor_device, ESP_RMAKER_PARAM_TEMPERATURE),
                esp_rmaker_float(g_temperature));
    }
    else
    {
        ESP_LOGE(TAG, "Error read from DHT!\n" ) ;
    }

    
}

float app_get_current_temperature()
{
    return (g_temperature) ;
}

esp_err_t app_sensor_init(void)
{
    //DHT11_init(GPIO_NUM_4) ;
    /*
    esp_err_t err = ws2812_led_init();
    if (err != ESP_OK) {
        return err;
    }
    */
    g_temperature = DEFAULT_TEMPERATURE;
    esp_timer_create_args_t sensor_timer_conf = {
        .callback = app_sensor_update,
        .dispatch_method = ESP_TIMER_TASK,
        .name = "app_sensor_update_tm"
    } ;

    if (esp_timer_create(&sensor_timer_conf, &sensor_timer) == ESP_OK) {
        esp_timer_start_periodic(sensor_timer, REPORTING_PERIOD * 1000000U) ;
        /*g_hue = (100 - g_temperature) * 2;
        ws2812_led_set_hsv(g_hue, g_saturation, g_value);
        */
        return (ESP_OK) ;
    }
    return (ESP_FAIL) ;
}

void app_driver_init()
{

    app_sensor_init() ;

    /*
    app_reset_button_register(app_reset_button_create(BUTTON_GPIO, BUTTON_ACTIVE_LEVEL),
                WIFI_RESET_BUTTON_TIMEOUT, FACTORY_RESET_BUTTON_TIMEOUT) ;
                */

    gpio_set_direction( GPIO_LED,      GPIO_MODE_OUTPUT ) ;

    gpio_set_direction( GPIO_ROOM_PIN,      GPIO_MODE_OUTPUT ) ;
    gpio_set_direction( GPIO_KITCHEN_PIN,   GPIO_MODE_OUTPUT ) ;
    gpio_set_direction( GPIO_FLOOR2_PIN,    GPIO_MODE_OUTPUT ) ;
    gpio_set_direction( GPIO_SMALL_ROOM_PIN,  GPIO_MODE_OUTPUT ) ;
    gpio_set_direction( GPIO_TERASSE_PIN,    GPIO_MODE_OUTPUT ) ;

}
