idf_component_register(SRCS "wifi.c"
                    INCLUDE_DIRS "include"
                    REQUIRES Device_Info esp_wifi esp_event esp_netif)

