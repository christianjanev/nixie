#include "freertos/idf_additions.h"
#include "hal/gpio_types.h"
#include "portmacro.h"
#include "soc/gpio_num.h"
#include <freertos/FreeRTOS.h>
#include <driver/gpio.h>
#include <esp_system.h>
#include "server.h"

void app_main(void)
{
    gpio_set_direction(GPIO_NUM_2, GPIO_MODE_OUTPUT);
    gpio_set_level(GPIO_NUM_2, 0);
    vTaskDelay(1000 / portTICK_PERIOD_MS);

    httpd_handle_t server = start_server();

    esp_restart();
}
