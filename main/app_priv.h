/*
   This example code is in the Public Domain (or CC0 licensed, at your option.)

   Unless required by applicable law or agreed to in writing, this
   software is distributed on an "AS IS" BASIS, WITHOUT WARRANTIES OR
   CONDITIONS OF ANY KIND, either express or implied.
*/
#pragma once

enum eLed_state{

    PRIV_NONE,
    PRIV_WAIT,
    PRIV_ING,
    PRIV_DONE,
    PRIV_FAIL,
    TX_ING
};
void app_driver_init(void);
void set_led_state(enum eLed_state state);
int app_driver_set_state(bool state);
bool app_driver_get_state(void);

