/**
 * @file main.c
 * 
 * @author
 * Angelo Elias Dalzotto (150633@upf.br)
 * Gabriel Boni Vicari (133192@upf.br)
 * GEPID - Grupo de Pesquisa em Cultura Digital (http://gepid.upf.br/)
 * Universidade de Passo Fundo (http://www.upf.br/)
 *
 * @copyright 2018-2019 Angelo Elias Dalzotto, Gabriel Boni Vicari
 *
 * @brief Main file for the MYB project for the ESP-IDF.
 */

#include <stdio.h>
#include <math.h>
#include <string.h>
#include <limits.h>
#include <driver/i2c.h>
#include <freertos/task.h>
#include "esp_log.h"
#include "esp_err.h"
#include "esp_spiffs.h"
#include "mpu6050/mpu6050.h"
#include "max30100/max30100.h"

#define PI              3.14159265358979323846f
#define AVG_BUFF_SIZE   20
#define SAMPLE_SIZE     2000
#define I2C_SDA         15
#define I2C_SCL         4
#define I2C_FREQ        100000
#define I2C_PORT        I2C_NUM_0

float self_test[6] = {0, 0, 0, 0, 0, 0};
float accel_bias[3] = {0, 0, 0};
float gyro_bias[3] = {0, 0, 0};

max30100_config_t max30100 = {};

esp_err_t i2c_master_init()
{
    i2c_config_t conf = {};
    conf.mode = I2C_MODE_MASTER;
    conf.sda_io_num = I2C_SDA;
    conf.scl_io_num = I2C_SCL;
    conf.sda_pullup_en = GPIO_PULLUP_ENABLE;
    conf.scl_pullup_en = GPIO_PULLUP_ENABLE;
    conf.master.clk_speed = I2C_FREQ;
	i2c_param_config(I2C_PORT, &conf);

	return (i2c_driver_install(I2C_PORT, I2C_MODE_MASTER, 0, 0, 0));
}

void bpm_counter(void* param)
{
    max30100_data_t result = {};

    while (true) {
        max30100_update(&max30100, &result);

        if (result.pulse_detected) {
            ESP_LOGI("MAX30100", "BPM: %f", result.heart_bpm);
        }

        vTaskDelay(10 / portTICK_PERIOD_MS);
    }
}

void app_main()
{
    esp_err_t ret;

    ESP_ERROR_CHECK(i2c_master_init());

    ESP_ERROR_CHECK(max30100_init
    (
        &max30100,
        I2C_PORT,
        MAX30100_DEFAULT_OPERATING_MODE,
        MAX30100_DEFAULT_SAMPLING_RATE,
        MAX30100_DEFAULT_LED_PULSE_WIDTH,
        MAX30100_DEFAULT_IR_LED_CURRENT,
        MAX30100_DEFAULT_START_RED_LED_CURRENT,
        MAX30100_DEFAULT_MEAN_FILTER_SIZE,
        1,
        true,
        false
    ));
    ESP_LOGI("MAX30100", "Device initialized.");
    xTaskCreate(bpm_counter, "BPMCounter", 8192, NULL, 1, NULL);
}
