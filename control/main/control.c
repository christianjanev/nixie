#include "server.h"
#include "control.h"

#include <time.h>

int led_on = 0;

int toggle_led()
{
    struct timespec epoch;
    clock_gettime(CLOCK_REALTIME, &epoch);
    
    struct tm* date_time = localtime(&(epoch.tv_sec));
    printf("time = %d:%d:%d\n", date_time->tm_hour, date_time->tm_min, date_time->tm_sec);
    return led_on;
}

void initialize_server_flash()
{
    esp_vfs_littlefs_conf_t vfs_conf = {
        .base_path = "/server",
        .partition_label = "webserver",
        .format_if_mount_failed = 1,
        .dont_mount = 0
    }; 
    ESP_ERROR_CHECK(esp_vfs_littlefs_register(&vfs_conf));

    size_t total = 0, used = 0;
    if (esp_littlefs_info(vfs_conf.partition_label, &total, &used) != ESP_OK)
    {
        esp_littlefs_format(vfs_conf.partition_label);
    }
    else
    {
        printf("Total=%d, Used=%d\n", total, used);
    }
}

void app_main(void)
{
    gpio_set_direction(GPIO_NUM_2, GPIO_MODE_OUTPUT);

    ESP_ERROR_CHECK(nvs_flash_init());
    nvs_flash_erase();
    ESP_ERROR_CHECK(nvs_flash_init());

    initialize_server_flash();

    esp_err_t err;
    esp_netif_t* network_interface;

    if ((err = initialize_wifi(&network_interface)) == ESP_OK)
    {
        gpio_set_level(GPIO_NUM_2, 1);
        led_on = 1;
        start_server();
    }
    else
    {
        printf("Failed to connect to WIFI\n");  
    }
}
