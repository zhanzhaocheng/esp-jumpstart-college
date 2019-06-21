/*
   This example code is in the Public Domain (or CC0 licensed, at your option.)

   Unless required by applicable law or agreed to in writing, this
   software is distributed on an "AS IS" BASIS, WITHOUT WARRANTIES OR
   CONDITIONS OF ANY KIND, either express or implied.
*/
#include <string.h>
#include <freertos/FreeRTOS.h>
#include <freertos/semphr.h>
#include <freertos/task.h>
#include <iot_button.h>
#include <nvs_flash.h>
#include "esp_system.h"

#include "app_priv.h"
#include JUMPSTART_BOARD

//以下六个函数为各个LED初始化和各个LED线程任务
void init_rled(void)
{
    gpio_pad_select_gpio(RLED_GPIO);
    gpio_set_direction(RLED_GPIO, GPIO_MODE_OUTPUT);     
}

void init_gled(void)
{
    gpio_pad_select_gpio(GLED_GPIO);
    gpio_set_direction(GLED_GPIO, GPIO_MODE_OUTPUT);  
}

void init_bled(void)
{
    gpio_pad_select_gpio(BLED_GPIO);
    gpio_set_direction(BLED_GPIO, GPIO_MODE_OUTPUT);     
}

void rled_task(void *pvParameter)
{
    while(1) {
        /* Blink off (output low) */
        gpio_set_level(RLED_GPIO, 0);
        vTaskDelay(500 / portTICK_PERIOD_MS);
        /* Blink on (output high) */
        gpio_set_level(RLED_GPIO, 1);
        vTaskDelay(500 / portTICK_PERIOD_MS);
    }
}

void gled_task(void *pvParameter)
{
    while(1) {
        /* Blink off (output low) */
        gpio_set_level(GLED_GPIO, 0);
        vTaskDelay(1000 / portTICK_PERIOD_MS);
        /* Blink on (output high) */
        gpio_set_level(GLED_GPIO, 1);
        vTaskDelay(1000 / portTICK_PERIOD_MS);
    }
}

void bled_task(void *pvParameter)
{
    while(1) {
        /* Blink off (output low) */
        gpio_set_level(BLED_GPIO, 0);
        vTaskDelay(100 / portTICK_PERIOD_MS);
        /* Blink on (output high) */
        gpio_set_level(BLED_GPIO, 1);
        vTaskDelay(100 / portTICK_PERIOD_MS);
    }
}

void led_show(char *rxbuff)
{
    if(!strcmp(rxbuff, "all_on"))
    {
        gpio_set_level(RLED_GPIO, 1); 
        gpio_set_level(GLED_GPIO, 1);
        gpio_set_level(BLED_GPIO, 1);     
    }
    if(!strcmp(rxbuff, "all_off"))
    {
        gpio_set_level(RLED_GPIO, 0); 
        gpio_set_level(GLED_GPIO, 0);
        gpio_set_level(BLED_GPIO, 0);         
    }
    if(!strcmp(rxbuff, "r_on"))
    {
        gpio_set_level(RLED_GPIO, 1);         
    }  
    if(!strcmp(rxbuff, "r_off"))
    {
        gpio_set_level(RLED_GPIO, 0);          
    } 
    if(!strcmp(rxbuff, "g_on"))
    {
        gpio_set_level(GLED_GPIO, 1);         
    }  
    if(!strcmp(rxbuff, "g_off"))
    {
        gpio_set_level(GLED_GPIO, 0);          
    } 
    if(!strcmp(rxbuff, "b_on"))
    {
        gpio_set_level(BLED_GPIO, 1);         
    }  
    if(!strcmp(rxbuff, "b_off"))
    {
        gpio_set_level(BLED_GPIO, 0);          
    }     
}

#define DEBOUNCE_TIME  30

static bool g_output_state;
static void push_btn_cb(void *arg)
{
    static uint64_t previous;
    uint64_t current = xTaskGetTickCount();
    if ((current - previous) > DEBOUNCE_TIME) {
        previous = current;
        app_driver_set_state(!g_output_state);
    }
}

static void button_press_3sec_cb(void *arg)
{
    nvs_flash_erase();
    esp_restart();
}

static void configure_push_button(int gpio_num, void (*btn_cb)(void *))
{
    button_handle_t btn_handle = iot_button_create(JUMPSTART_BOARD_BUTTON_GPIO, JUMPSTART_BOARD_BUTTON_ACTIVE_LEVEL);
    if (btn_handle) {
        iot_button_set_evt_cb(btn_handle, BUTTON_CB_RELEASE, btn_cb, "RELEASE");
        iot_button_add_on_press_cb(btn_handle, 3, button_press_3sec_cb, NULL);
    }
}

static void set_output_state(bool target)
{
    gpio_set_level(JUMPSTART_BOARD_OUTPUT_GPIO, target);
}

void app_driver_init()
{
    configure_push_button(JUMPSTART_BOARD_BUTTON_GPIO, push_btn_cb);

    /* Configure output */
    gpio_config_t io_conf = {
        .mode = GPIO_MODE_OUTPUT,
        .pull_up_en = 1,
    };
    io_conf.pin_bit_mask = ((uint64_t)1 << JUMPSTART_BOARD_OUTPUT_GPIO);
    /* Configure the GPIO */
    gpio_config(&io_conf);
}

int IRAM_ATTR app_driver_set_state(bool state)
{
    if(g_output_state != state) {
        g_output_state = state;
        set_output_state(g_output_state);
    }
    return ESP_OK;
}

bool app_driver_get_state(void)
{
    return g_output_state;
}
