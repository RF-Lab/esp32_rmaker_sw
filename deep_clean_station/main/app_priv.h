/*
   This example code is in the Public Domain (or CC0 licensed, at your option.)

   Unless required by applicable law or agreed to in writing, this
   software is distributed on an "AS IS" BASIS, WITHOUT WARRANTIES OR
   CONDITIONS OF ANY KIND, either express or implied.
*/
#pragma once
#include <stdint.h>
#include <stdbool.h>
#include "driver/gpio.h"

#define DEFAULT_TEMPERATURE 25.0
#define REPORTING_PERIOD    15 /* Seconds */

extern esp_rmaker_device_t *temp_sensor_device ;
extern esp_rmaker_device_t *rssi_monitor_device ;

void app_driver_init(void);
float app_get_current_temperature();

#define  GPIO_LED                            GPIO_NUM_2

#define  AIR_PUMP_PIN                        GPIO_NUM_27
#define  WATER_PUMP_PIN                      GPIO_NUM_26
#define  HEAT_CABLE_PIN                      GPIO_NUM_25

#define  GPIO_DHT2                           GPIO_NUM_22
