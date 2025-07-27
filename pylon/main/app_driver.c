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
//#include "dht.h"
#include <esp_log.h>
#include "wifi_utils.h"


static const char *TAG = "app_driver";
DRAM_ATTR TaskHandle_t btnTaskHandle ;
volatile SemaphoreHandle_t btnBinarySemaphore ;


/* This is the button that is used for toggling the power */
#define BUTTON_GPIO          0
#define BUTTON_ACTIVE_LEVEL  0
/* This is the GPIO on which the power will be set */
#define OUTPUT_GPIO    19

//static esp_timer_handle_t sensor_timer ;

#define DEFAULT_SATURATION  100
#define DEFAULT_BRIGHTNESS  50

#define WIFI_RESET_BUTTON_TIMEOUT       3
#define FACTORY_RESET_BUTTON_TIMEOUT    10


// Post control support
static const gpio_num_t POST_BUTTON_ON     = GPIO_NUM_18 ;         // 
static const gpio_num_t POST_BUTTON_OFF    = GPIO_NUM_19 ;         // 


//static uint16_t g_hue;
//static uint16_t g_saturation = DEFAULT_SATURATION;
//static uint16_t g_value = DEFAULT_BRIGHTNESS;
//static float g_temperature ;

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
    float RSSI = 0 ;
    //struct dht11_reading data ;

    ESP_LOGI(TAG, "app_sensor_update() called\n") ;
    /*
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
    */

    RSSI = getRSSI() ;
    ESP_LOGI(TAG, "RSSI: %5.2fdbm\n", RSSI ) ;
    esp_rmaker_param_update_and_report(
            esp_rmaker_device_get_param_by_type(rssi_monitor_device, ESP_RMAKER_PARAM_TEMPERATURE),
            esp_rmaker_float(RSSI)) ;
    
}

/*
float app_get_current_temperature()
{
    return (g_temperature) ;
}
    */

esp_err_t app_sensor_init(void)
{
    //DHT11_init(GPIO_NUM_4) ;
    /*
    esp_err_t err = ws2812_led_init();
    if (err != ESP_OK) {
        return err;
    }
    */
   /*
    g_temperature = DEFAULT_TEMPERATURE;
    esp_timer_create_args_t sensor_timer_conf = {
        .callback = app_sensor_update,
        .dispatch_method = ESP_TIMER_TASK,
        .name = "app_sensor_update_tm"
    } ;
     */
    /*
    if (esp_timer_create(&sensor_timer_conf, &sensor_timer) == ESP_OK) {
        esp_timer_start_periodic(sensor_timer, REPORTING_PERIOD * 1000000U) ;
        return (ESP_OK) ;
    }
    */
    return (ESP_OK) ;
}

IRAM_ATTR static void btn_gpio_isr_handler(void* arg)
{
    //type_drdy_isr_context* pContext = (type_drdy_isr_context*) arg ;
    static BaseType_t high_task_wakeup = pdFALSE ;

    xSemaphoreGiveFromISR( btnBinarySemaphore, &high_task_wakeup ) ;

    //vTaskNotifyGiveFromISR( btnTaskHandle, &high_task_wakeup ) ;

    /* If high_task_wakeup was set to true you
    should yield.  The actual macro used here is
    port specific. */
    if ( high_task_wakeup!=pdFALSE )
    {
        portYIELD_FROM_ISR( ) ;
    }
}

int pump_state = false ;

IRAM_ATTR void btn_task( void* pvParameter )
{
    const TickType_t msToWait = 60000*5 ;
    TimeOut_t xTimeOut ;
    TickType_t xTicksToWait = pdMS_TO_TICKS(msToWait) ;

    ESP_LOGI(TAG,"btn_task started at CpuCore%1d\n", xPortGetCoreID() ) ;

    // Initialize water pump state to Off
    pump_state = false ;
    gpio_set_level(WATER_PUMP_PIN, (pump_state)?1:0 ) ;
    //gpio_set_level(GPIO_LED, (pump_state)?1:0 ) ;

    // Continuous capture the data
    for( ;; )
    {
        xSemaphoreTake( btnBinarySemaphore, portMAX_DELAY ) ;
        //ESP_LOGI(TAG,"btn_task: got btnBinarySemaphore\n" ) ;
        // check for off button
        if (gpio_get_level(POST_BUTTON_OFF)==1 && pump_state)
        {
            ESP_LOGI(TAG,"btn_task: got POST_BUTTON_OFF\n" ) ;
            // Debounce
            vTaskDelay( pdMS_TO_TICKS( 100 ) ) ;
            if (gpio_get_level(POST_BUTTON_OFF)==1)
            {
                // esp_rmaker_param_update_and_report(
                //         esp_rmaker_device_get_param_by_type(water_pump_device, ESP_RMAKER_DEVICE_SWITCH),
                //         esp_rmaker_bool(false)) ;
                ESP_LOGI(TAG,"btn_task: POST_BUTTON_OFF updated\n" ) ;
                pump_state = false ;
                gpio_set_level(WATER_PUMP_PIN, (pump_state)?1:0 ) ;
                //gpio_set_level(GPIO_LED, (pump_state)?1:0 ) ;
            }
        }
        else if (gpio_get_level(POST_BUTTON_ON)==0 && !pump_state)
        {
            ESP_LOGI(TAG,"btn_task: got POST_BUTTON_ON\n" ) ;
            // Debounce
            vTaskDelay( pdMS_TO_TICKS( 100 ) ) ;
            if (gpio_get_level(POST_BUTTON_ON)==0)
            {                
                // esp_rmaker_param_update_and_report(
                //         esp_rmaker_device_get_param_by_type(water_pump_device, ESP_RMAKER_DEVICE_SWITCH),
                //         esp_rmaker_bool(true)) ;                
                ESP_LOGI(TAG,"btn_task: POST_BUTTON_ON updated\n" ) ;
                // Switch pump On
                pump_state = true ;
                gpio_set_level(WATER_PUMP_PIN, (pump_state)?1:0 ) ;
                //gpio_set_level(GPIO_LED, (pump_state)?1:0 ) ;
                // Start measure timeout
                vTaskSetTimeOutState( &xTimeOut ) ;
                xTicksToWait = pdMS_TO_TICKS(msToWait) ;
                // wait for user release ON button
                for(;gpio_get_level(POST_BUTTON_ON)==0;)
                {
                    xSemaphoreTake( btnBinarySemaphore, pdMS_TO_TICKS(100) ) ; 
                }
                // Wait for timeout
                for(;;)
                {
                    // Try to wait timout, but check for stop button
                    if (xSemaphoreTake( btnBinarySemaphore, pdMS_TO_TICKS(5000) )==pdTRUE)
                    {
                        // If stop button pressed
                        if (gpio_get_level(POST_BUTTON_OFF)==1)
                        {
                            vTaskDelay( pdMS_TO_TICKS( 50 ) ) ;
                            if (gpio_get_level(POST_BUTTON_OFF)==1)
                            {
                                ESP_LOGI(TAG,"btn_task: timeout cancelled (POST_BUTTON_OFF)\n" ) ;
                                break ;
                            } 
                        }
                    }
                    if( xTaskCheckForTimeOut( &xTimeOut, &xTicksToWait ) != pdFALSE )
                    {
                        ESP_LOGI(TAG,"btn_task: pump timeout expired - switched off\n" ) ;
                        break ;
                    }
                }
                pump_state = false ;
                gpio_set_level(WATER_PUMP_PIN, (pump_state)?1:0 ) ;
                //gpio_set_level(GPIO_LED, (pump_state)?1:0 ) ;
            }
        }
    }
}


void app_driver_init()
{

    btnBinarySemaphore = xSemaphoreCreateBinary() ;

    // Start button task
    TaskHandle_t btnTaskHandle ;
    xTaskCreate( &btn_task, "btn_task", 4096, NULL, tskIDLE_PRIORITY+5, &btnTaskHandle ) ;
    configASSERT( btnTaskHandle ) ;

    app_sensor_init() ;

    /*
    app_reset_button_register(app_reset_button_create(BUTTON_GPIO, BUTTON_ACTIVE_LEVEL),
                WIFI_RESET_BUTTON_TIMEOUT, FACTORY_RESET_BUTTON_TIMEOUT) ;
                */

    gpio_set_direction( GPIO_LED,       GPIO_MODE_OUTPUT ) ;
    gpio_set_direction( WATER_PUMP_PIN, GPIO_MODE_OUTPUT ) ;

    gpio_reset_pin( POST_BUTTON_ON ) ;
    gpio_set_direction( POST_BUTTON_ON, GPIO_MODE_INPUT ) ;
    gpio_set_intr_type( POST_BUTTON_ON, GPIO_INTR_NEGEDGE ) ;
    gpio_intr_enable( POST_BUTTON_ON ) ;

    gpio_reset_pin( POST_BUTTON_OFF ) ;
    gpio_set_direction( POST_BUTTON_OFF, GPIO_MODE_INPUT ) ;
    gpio_set_intr_type( POST_BUTTON_OFF, GPIO_INTR_POSEDGE ) ;
    gpio_intr_enable( POST_BUTTON_OFF ) ;

    gpio_install_isr_service(0) ;
    gpio_isr_handler_add( POST_BUTTON_ON, btn_gpio_isr_handler, NULL ) ;
    gpio_isr_handler_add( POST_BUTTON_OFF, btn_gpio_isr_handler, NULL ) ;

}

