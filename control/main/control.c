#include <freertos/FreeRTOS.h>
#include <esp_system.h>
#include <driver/gpio.h>
#include "server.h"
#include <stdio.h>

void app_main(void)
{
    gpio_set_direction(GPIO_NUM_2, GPIO_MODE_OUTPUT);

    ESP_ERROR_CHECK(nvs_flash_init());

    esp_err_t err;
    esp_netif_t* network_interface;

    if ((err = initialize_wifi(&network_interface)) == ESP_OK)
    {
        gpio_set_level(GPIO_NUM_2, 1);
        start_server();
    }
    else
    {
        printf("Failed to connect to WIFI\n");  
    }

    for (;;)
    {
        vTaskDelay(pdMS_TO_TICKS(10));
    }

    esp_restart();
}
