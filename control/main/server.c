#include "server.h"

esp_err_t initialize_wifi()
{
    wifi_init_config_t config = WIFI_INIT_CONFIG_DEFAULT();
    esp_wifi_init(&config);
    esp_wifi_start();

    wifi_sta_config_t station_config;

}
httpd_handle_t start_server()
{
    httpd_config_t config = HTTPD_DEFAULT_CONFIG();
    httpd_handle_t server = NULL;
    httpd_start(&server, &config);

    return server;
}