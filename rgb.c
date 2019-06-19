#include "rgb.h"

void blink_delay(){
    for(long long i=0;i<900000000;i++)
    {}
};
void RGB_init(){
    gpio_pad_select_gpio(GPIO_RED);
    gpio_pad_select_gpio(GPIO_GREEN);
    gpio_pad_select_gpio(GPIO_BLUE);

    gpio_set_direction(GPIO_RED, GPIO_MODE_OUTPUT);
    gpio_set_direction(GPIO_GREEN, GPIO_MODE_OUTPUT);
    gpio_set_direction(GPIO_BLUE, GPIO_MODE_OUTPUT);
}
void BLINK_RED(){
    gpio_set_level(GPIO_RED, 0);
    blink_delay();
    gpio_set_level(GPIO_RED, 1);
    blink_delay();
}
void BLINK_GREEN(){
    gpio_set_level(GPIO_GREEN, 0);
    blink_delay();
    gpio_set_level(GPIO_GREEN, 1);
    blink_delay();
}
void BLINK_BLUE(){
    gpio_set_level(GPIO_BLUE, 0);
    blink_delay();
    gpio_set_level(GPIO_BLUE, 1);
    blink_delay();
}
void ONPEN_RED(){
    gpio_set_level(GPIO_RED, 1);
}
void CLOSE_RED(){
    gpio_set_level(GPIO_RED, 0);
}
void ONPEN_GREEN(){
    gpio_set_level(GPIO_GREEN, 1);
}
void CLOSE_GREEN(){
    gpio_set_level(GPIO_GREEN, 0);
}
void ONPEN_BLUE(){
    gpio_set_level(GPIO_BLUE, 1);
}
void CLOSE_BLUE(){
    gpio_set_level(GPIO_BLUE, 0);
}