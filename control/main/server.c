#include "server.h"

static EventGroupHandle_t wifi_event_group;

static void wifi_event_handler(void* arg, esp_event_base_t event_base, int32_t event_id, void* event_data)
{
    if (event_base == WIFI_EVENT)
    {
        switch (event_id)
        {
            case WIFI_EVENT_STA_START:
                esp_wifi_connect();
                break;
            case WIFI_EVENT_STA_DISCONNECTED:
                printf("Disconnected...Attemping to reconnect.\n");
                esp_wifi_connect();
                break;
        }
    }
    else if (event_base == IP_EVENT)
    {
        switch (event_id)
        {
            case IP_EVENT_STA_GOT_IP:
            {
                ip_event_got_ip_t* event = (ip_event_got_ip_t*)event_data;
                printf("Obtained IP=%d.%d.%d.%d\n", IP2STR(&event->ip_info.ip));

                xEventGroupSetBits(wifi_event_group, WIFI_CONNECTED_BIT);
                break;
            }
        }
    }

}

esp_err_t initialize_wifi(esp_netif_t** netif)
{
    wifi_event_group = xEventGroupCreate();

    if (wifi_event_group == NULL)
        return ESP_ERR_NO_MEM;

    ESP_ERROR_CHECK(esp_netif_init());

    ESP_ERROR_CHECK(esp_event_loop_create_default());

    esp_netif_t* network_interface = esp_netif_create_default_wifi_sta();
    *netif = network_interface;

    wifi_init_config_t config = WIFI_INIT_CONFIG_DEFAULT();
    ESP_ERROR_CHECK(esp_wifi_init(&config));

    ESP_ERROR_CHECK(esp_event_handler_register(WIFI_EVENT, ESP_EVENT_ANY_ID, &wifi_event_handler, NULL));
    ESP_ERROR_CHECK(esp_event_handler_register(IP_EVENT, IP_EVENT_STA_GOT_IP, &wifi_event_handler, NULL));

    ESP_ERROR_CHECK(esp_wifi_set_mode(WIFI_MODE_STA));

    wifi_config_t station_config = {
        .sta = {
            .ssid = SSID,
            .password = PSK
        },
    };

    ESP_ERROR_CHECK(esp_wifi_set_config(WIFI_IF_STA, &station_config));

    ESP_ERROR_CHECK(esp_wifi_start());

    EventBits_t bits = xEventGroupWaitBits(wifi_event_group, WIFI_CONNECTED_BIT, pdFALSE, pdFALSE, portMAX_DELAY);

    vEventGroupDelete(wifi_event_group);

    if (bits & WIFI_CONNECTED_BIT)
        return ESP_OK;
    else return ESP_FAIL;
}

httpd_handle_t start_server()
{
    httpd_config_t config = HTTPD_DEFAULT_CONFIG();
    httpd_handle_t server = NULL;
    ESP_ERROR_CHECK(httpd_start(&server, &config));

    return server;
}