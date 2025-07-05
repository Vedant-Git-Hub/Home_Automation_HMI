idf_component_register(SRCS "LCD.c"
                    INCLUDE_DIRS "include"
                    REQUIRES lvgl__lvgl esp_lcd espressif__esp_lcd_touch espressif__esp_lcd_touch_gt911 Ui)

