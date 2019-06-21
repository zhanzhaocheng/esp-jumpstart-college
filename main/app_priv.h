/*
   This example code is in the Public Domain (or CC0 licensed, at your option.)

   Unless required by applicable law or agreed to in writing, this
   software is distributed on an "AS IS" BASIS, WITHOUT WARRANTIES OR
   CONDITIONS OF ANY KIND, either express or implied.
*/
#pragma once

#define CONFIG_RLED_GPIO 25
#define CONFIG_GLED_GPIO 26
#define CONFIG_BLED_GPIO 27
#define RLED_GPIO CONFIG_RLED_GPIO
#define GLED_GPIO CONFIG_GLED_GPIO
#define BLED_GPIO CONFIG_BLED_GPIO

void init_rled(void);
void init_gled(void);
void init_bled(void);
void rled_task(void *pvParameter);
void gled_task(void *pvParameter);
void bled_task(void *pvParameter);
void led_show(char *rxbuff);
void app_driver_init(void);
int app_driver_set_state(bool state);
bool app_driver_get_state(void);

