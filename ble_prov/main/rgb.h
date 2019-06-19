#include <stdio.h>
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "driver/gpio.h"

#define GPIO_RED (21)
#define GPIO_GREEN (22)
#define GPIO_BLUE (23)

void blink_delay();
void RGB_init();
void BLINK_RED();
void BLINK_GREEN();
void BLINK_BLUE();
void ONPEN_RED();
void CLOSE_RED();
void ONPEN_GREEN();
void CLOSE_GREEN();
void ONPEN_BLUE();
void CLOSE_BLUE();