#ifndef CONTROL_H
#define CONTROL_H

#include <driver/gpio.h>
#include <freertos/FreeRTOS.h>
#include <esp_system.h>
#include <esp_littlefs.h>
#include <stdio.h>

int toggle_led();

#endif // CONTROL_H
