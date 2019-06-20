/*
   This example code is in the Public Domain (or CC0 licensed, at your option.)

   Unless required by applicable law or agreed to in writing, this
   software is distributed on an "AS IS" BASIS, WITHOUT WARRANTIES OR
   CONDITIONS OF ANY KIND, either express or implied.
*/

#include <freertos/FreeRTOS.h>
#include <freertos/semphr.h>
#include <freertos/task.h>
#include <iot_button.h>
#include <nvs_flash.h>
#include "esp_system.h"

#include "app_priv.h"
#include JUMPSTART_BOARD

#define DEBOUNCE_TIME  30

volatile enum eLed_state led_state_current=PRIV_NONE;

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
        //iot_button_set_evt_cb(btn_handle, BUTTON_CB_RELEASE, btn_cb, "RELEASE");
        iot_button_add_on_press_cb(btn_handle, 3, button_press_3sec_cb, NULL);
    }
}

static void set_output_state(bool target)
{
    gpio_set_level(JUMPSTART_BOARD_OUTPUT_RED, target);
}

void set_led_state(enum eLed_state state)
{
    led_state_current=state;
}

void led_state_task(void *pvParameter)
{
    /* 
        GPIO should be configured before
        Waiting for Provisioning : blink red
        Provisioning :blink blue
        Provision done : solid green
        Provision fail : solid red
    */
   static bool red_last=0;
  // static bool green_last=0;
   static bool blue_last=0;
    while(1) {
        switch(led_state_current){
            case PRIV_NONE:
            break;
            case PRIV_WAIT:
             red_last= !red_last; 
             gpio_set_level(JUMPSTART_BOARD_OUTPUT_RED,red_last);
             gpio_set_level(JUMPSTART_BOARD_OUTPUT_GREEN,0);
             gpio_set_level(JUMPSTART_BOARD_OUTPUT_BLUE,0);
            break;
            case PRIV_ING:
             blue_last= !blue_last; 
             gpio_set_level(JUMPSTART_BOARD_OUTPUT_RED,0);
             gpio_set_level(JUMPSTART_BOARD_OUTPUT_GREEN,0);
             gpio_set_level(JUMPSTART_BOARD_OUTPUT_BLUE,blue_last);
            break;
            case PRIV_DONE:
             gpio_set_level(JUMPSTART_BOARD_OUTPUT_RED,0);
             gpio_set_level(JUMPSTART_BOARD_OUTPUT_GREEN,1);
             gpio_set_level(JUMPSTART_BOARD_OUTPUT_BLUE,0);
            break;
            case PRIV_FAIL:
             gpio_set_level(JUMPSTART_BOARD_OUTPUT_RED,1);
             gpio_set_level(JUMPSTART_BOARD_OUTPUT_GREEN,0);
             gpio_set_level(JUMPSTART_BOARD_OUTPUT_BLUE,0);
            break;
        }
        vTaskDelay(500 / portTICK_PERIOD_MS);
    }
}

void app_driver_init()
{
    configure_push_button(JUMPSTART_BOARD_BUTTON_GPIO, push_btn_cb);

    /* Configure output */
    gpio_config_t io_conf = {
        .mode = GPIO_MODE_OUTPUT,
        .pull_up_en = 1,
    };
    io_conf.pin_bit_mask = (((uint64_t)1 << JUMPSTART_BOARD_OUTPUT_RED)|((uint64_t)1 <<JUMPSTART_BOARD_OUTPUT_GREEN)|((uint64_t)1 <<JUMPSTART_BOARD_OUTPUT_BLUE)); //配置指示灯端口
    /* Configure the GPIO */
    gpio_config(&io_conf);
    xTaskCreate(&led_state_task, "led_state_task", configMINIMAL_STACK_SIZE, NULL, 5, NULL);

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
