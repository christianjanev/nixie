#include "server.h"
#include "control.h"

static EventGroupHandle_t wifi_event_group;

static void wifi_event_handler(void* arg, esp_event_base_t event_base, int32_t event_id, void* event_data)
{
    if (event_base == WIFI_EVENT)
    {
        switch (event_id)
        {
            case WIFI_EVENT_STA_START:
                printf("Connecting to %s\n", SSID);
                esp_wifi_connect();
                break;
            case WIFI_EVENT_STA_DISCONNECTED:
                printf("Disconnected...Attemping to reconnect to %s.\n", SSID);
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
                start_ntp();
                xEventGroupSetBits(wifi_event_group, WIFI_CONNECTED_BIT);
                break;
            }
        }
    }

}

long read_file(char* file_path, char** buffer)
{
    long file_size = -1;
    
    FILE* file = fopen(file_path, "r");
    
    if (file != NULL)
    {
        fseek(file, 0, SEEK_END);
        file_size = ftell(file);
        rewind(file);

        *buffer = (char*)malloc(file_size + 1);
        printf("Reading file\n");
        fread(*buffer, file_size, 1, file);
    }
    else
    {
        printf("Failed to read file: %s\n", file_path);
    }

    fclose(file);

    return file_size;
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

esp_err_t root_uri_handler(httpd_req_t* req)
{   
    char* index_buffer = "";
    long index_size = read_file("/server/index.html", &index_buffer);
    if (index_size == -1) printf("Failed to read index.html\n");
    ESP_ERROR_CHECK(httpd_resp_send(req, index_buffer, index_size));

    return ESP_OK;
}

esp_err_t css_uri_handler(httpd_req_t* req)
{   
    char* css_buffer = "";
    const char* uri = req->uri;
    int uri_len = strlen(uri);
    if (strcmp(&uri[uri_len - 5], ".css"))
    {
        int file_path_len = strlen("/server") + uri_len + 1;
        char file_path[file_path_len];
        printf("uri = %s, uri_len = %d\n", uri, uri_len);
        snprintf(file_path, file_path_len, "/server%s", uri);
        long css_size = read_file(file_path, &css_buffer);
        if (css_size == -1) printf("Failed to read %s\n", file_path);

        ESP_ERROR_CHECK(httpd_resp_set_type(req, "text/css"));
        ESP_ERROR_CHECK(httpd_resp_send(req, css_buffer, css_size));
    }
    return ESP_OK;
}

esp_err_t js_uri_handler(httpd_req_t* req)
{   
    char* js_buffer = "";
    const char* uri = req->uri;
    int uri_len = strlen(uri);
    if (strcmp(&uri[uri_len - 4], ".js"))
    {
        int file_path_len = strlen("/server") + uri_len + 1;
        char file_path[file_path_len];
        printf("uri = %s, uri_len = %d\n", uri, uri_len);
        snprintf(file_path, file_path_len, "/server%s", uri);
        long js_size = read_file(file_path, &js_buffer);
        if (js_size == -1) printf("Failed to read %s\n", file_path);

        ESP_ERROR_CHECK(httpd_resp_set_type(req, "text/javascript"));
        ESP_ERROR_CHECK(httpd_resp_send(req, js_buffer, js_size));
    }
    return ESP_OK;
}

esp_err_t activate_led_uri_handler(httpd_req_t* req)
{
    toggle_led();

    return ESP_OK;
}

void register_uri_handlers(httpd_handle_t server)
{
    httpd_uri_t root_uri = {
        .uri = "/",
        .method = HTTP_GET,
        .handler = root_uri_handler,
        .user_ctx = NULL
    };

    httpd_uri_t css_uri = {
        .uri = "/css/*",
        .method = HTTP_GET,
        .handler = css_uri_handler,
        .user_ctx = NULL
    };

    httpd_uri_t js_uri = {
        .uri = "/scripts/*",
        .method = HTTP_GET,
        .handler = js_uri_handler,
        .user_ctx = NULL
    };

    httpd_uri_t activate_led_uri = {
        .uri = "/activate",
        .method = HTTP_POST,
        .handler = activate_led_uri_handler,
        .user_ctx = NULL
    };

    ESP_ERROR_CHECK(httpd_register_uri_handler(server, &root_uri));
    ESP_ERROR_CHECK(httpd_register_uri_handler(server, &css_uri));
    ESP_ERROR_CHECK(httpd_register_uri_handler(server, &js_uri));
    ESP_ERROR_CHECK(httpd_register_uri_handler(server, &activate_led_uri));
}

httpd_handle_t start_server()
{
    httpd_config_t config = HTTPD_DEFAULT_CONFIG();
    config.max_uri_handlers = 20;
    config.uri_match_fn = httpd_uri_match_wildcard;

    httpd_handle_t server = NULL;
    ESP_ERROR_CHECK(httpd_start(&server, &config));

    register_uri_handlers(server);

    return server;
}

esp_err_t start_ntp()
{
    esp_setenv("TZ", "CST6CDT,M3.2.0,M11.1.0", 1); // America/Chicago
    tzset();

    sntp_config_t config = ESP_NETIF_SNTP_DEFAULT_CONFIG("time.nist.gov");
    esp_netif_sntp_init(&config);
    
    return esp_netif_sntp_start();
}
