#include "LCD.h"


#include "display/lv_display.h"
#include "esp_err.h"
#include "esp_timer.h"
#include "freertos/idf_additions.h"
#include "indev/lv_indev.h"
#include "portmacro.h"
#include "soc/clk_tree_defs.h"
#include <stdbool.h>
#include <sys/unistd.h>
#include <unistd.h>
#include <esp_lcd_panel_rgb.h>
#include <esp_lcd_panel_ops.h>
#include <esp_heap_caps.h>
#include <driver/gpio.h>
#include <driver/i2c.h>
#include <esp_lcd_types.h>
#include <esp_lcd_touch_gt911.h>
#include <esp_lcd_touch.h>

#include <lvgl.h>

#include "Ui.h"


#define TOUCH_I2C_NUM I2C_NUM_0

#define SCREEN_WIDTH 800
#define SCREEN_HEIGHT 480

#define LCD_CLK_SRC			LCD_CLK_SRC_DEFAULT
#define LCD_PCLK_CLK		10000000UL

#define LCD_GPIO_HSYNC		39
#define LCD_GPIO_VSYNC		40
#define LCD_GPIO_PCLK		0
#define LCD_GPIO_DE			41
#define LCD_GPIO_DISP		-1
/*BLUE*/
#define LCD_GPIO_B0			15
#define LCD_GPIO_B1			7
#define LCD_GPIO_B2			6
#define LCD_GPIO_B3			5
#define LCD_GPIO_B4			4
/*GREEN*/
#define LCD_GPIO_G0			9
#define LCD_GPIO_G1			46
#define LCD_GPIO_G2			3
#define LCD_GPIO_G3			8
#define LCD_GPIO_G4			16
#define LCD_GPIO_G5			1
/*RED*/
#define LCD_GPIO_R0			14
#define LCD_GPIO_R1			21
#define LCD_GPIO_R2			47
#define LCD_GPIO_R3			48
#define LCD_GPIO_R4			45
/*LCD Back Light*/
#define LCD_GPIO_BL			2

#define LCD_HSYNC_FPORCH	40
#define LCD_HSYNC_PWIDTH	48
#define LCD_HSYNC_BPORCH	40
#define LCD_VSYNC_FPORCH	1
#define LCD_VSYNC_PWIDTH	31
#define LCD_VSYNC_BPORCH	13

#define BYTES_PER_PIXEL		(LV_COLOR_FORMAT_GET_SIZE(LV_COLOR_FORMAT_RGB565))


static uint32_t my_tick_get_cb(void);
static void my_flush_cb(lv_display_t *display, const lv_area_t *area, uint8_t *px_map);
static void my_input_read(lv_indev_t * indev, lv_indev_data_t * data);
static void lcd_init();
static void gt911_init();


static esp_lcd_panel_handle_t handle;
static esp_lcd_rgb_panel_config_t config;
static esp_lcd_touch_handle_t tp;
static esp_lcd_panel_io_handle_t touch_handle;


void task_LCD(void *vParameters)
{
	/*Initialize LCD*/
	lcd_init();
	/*Initialize touch controller*/
	gt911_init();
	/*Initialize LVGL*/
	lv_init();
	/*100ms delay*/
	vTaskDelay(100/portTICK_PERIOD_MS);
	/*Register tick callback in LVGL*/
	lv_tick_set_cb(my_tick_get_cb);
	/*Create display*/
	lv_display_t *display = lv_display_create(SCREEN_WIDTH, SCREEN_HEIGHT);
	/*Display should not be null*/
	if(display != NULL)
	{
		void *fb1, *fb2;
		/*Get pointers for 2 buffers which are created in lcd_init*/
		esp_lcd_rgb_panel_get_frame_buffer(handle, 2, &fb1, &fb2);
		uint8_t *lcd_buff_ram1 = (uint8_t *)fb1;
		uint8_t *lcd_buff_ram2 = (uint8_t *)fb2;
		uint32_t lcd_buff_size = (SCREEN_WIDTH * SCREEN_HEIGHT * BYTES_PER_PIXEL);
		/*Register these buffers in lvgl*/
		lv_display_set_buffers(display, lcd_buff_ram1, lcd_buff_ram2, lcd_buff_size, LV_DISPLAY_RENDER_MODE_DIRECT);
		/*Register the flush callback*/
		lv_display_set_flush_cb(display, my_flush_cb);
	}

	/* Create input device connected to Default Display. */
	lv_indev_t * indev = lv_indev_create();        
	/*Input device should not be NULL*/
	if(indev != NULL)
	{
		/* Touch pad is a pointer-like device. */
		lv_indev_set_type(indev, LV_INDEV_TYPE_POINTER);   
		/*Register touch callback in LVGL*/
		lv_indev_set_read_cb(indev, my_input_read);
	}

	xTaskCreate(task_UI, "UI", 4096, NULL, 1, NULL);
	
    while (true) {
		/*Run LVGL timely*/
		uint32_t time_to_wait = lv_timer_handler();
		/*Defined delay*/
		vTaskDelay(time_to_wait/portTICK_PERIOD_MS);
    }
}


static uint32_t my_tick_get_cb(void)
{
	return (esp_timer_get_time() / 1000);
}

static void my_flush_cb(lv_display_t *display, const lv_area_t *area, uint8_t *px_map)
{
	int x1 = area->x1;
	int x2 = area->x2;
	int y1 = area->y1;
	int y2 = area->y2;
	
	ESP_ERROR_CHECK(esp_lcd_panel_draw_bitmap(handle, x1, y1, x2, y2, px_map));
	
//	ESP_ERROR_CHECK(esp_lcd_rgb_panel_refresh(handle));	
		
	lv_display_flush_ready(display);
}

static void my_input_read(lv_indev_t * indev, lv_indev_data_t * data)
{
	uint16_t touch_x[1];
    uint16_t touch_y[1];
    uint16_t touch_strength[1];
    uint8_t touch_cnt = 0;
    
    esp_lcd_touch_read_data(tp);
    
	bool touchpad_pressed = esp_lcd_touch_get_coordinates(tp, touch_x, touch_y, touch_strength, &touch_cnt, 1);

    if(touchpad_pressed) 
    {
        data->point.x = (int32_t )touch_x[0];
        data->point.y = (int32_t )touch_y[0];
        data->state = LV_INDEV_STATE_PRESSED;
    } 
    else 
    {
        data->state = LV_INDEV_STATE_RELEASED;
    }
}

static void lcd_init()
{
	config.clk_src = LCD_CLK_SRC;
	config.data_width = 16;
	
	config.hsync_gpio_num = LCD_GPIO_HSYNC;
	config.vsync_gpio_num = LCD_GPIO_VSYNC;
	config.pclk_gpio_num = LCD_GPIO_PCLK;
	config.de_gpio_num = LCD_GPIO_DE;
	config.disp_gpio_num = LCD_GPIO_DISP;
	
	config.data_gpio_nums[0] = LCD_GPIO_B0;
	config.data_gpio_nums[1] = LCD_GPIO_B1;
	config.data_gpio_nums[2] = LCD_GPIO_B2;
	config.data_gpio_nums[3] = LCD_GPIO_B3;
	config.data_gpio_nums[4] = LCD_GPIO_B4;
	config.data_gpio_nums[5] = LCD_GPIO_G0;
	config.data_gpio_nums[6] = LCD_GPIO_G1;
	config.data_gpio_nums[7] = LCD_GPIO_G2;
	config.data_gpio_nums[8] = LCD_GPIO_G3;
	config.data_gpio_nums[9] = LCD_GPIO_G4;
	config.data_gpio_nums[10] = LCD_GPIO_G5;
	config.data_gpio_nums[11] = LCD_GPIO_R0;
	config.data_gpio_nums[12] = LCD_GPIO_R1;
	config.data_gpio_nums[13] = LCD_GPIO_R2;
	config.data_gpio_nums[14] = LCD_GPIO_R3;
	config.data_gpio_nums[15] = LCD_GPIO_R4;
	
	config.num_fbs = 2;
	config.flags.fb_in_psram = true;
	config.flags.refresh_on_demand = false;
	config.timings.h_res = SCREEN_WIDTH;
	config.timings.v_res = SCREEN_HEIGHT;
	config.timings.pclk_hz = LCD_PCLK_CLK;
	config.timings.hsync_front_porch = LCD_HSYNC_FPORCH;
	config.timings.hsync_pulse_width = LCD_HSYNC_PWIDTH;
	config.timings.hsync_back_porch = LCD_HSYNC_BPORCH;
	config.timings.vsync_front_porch = LCD_VSYNC_FPORCH;
	config.timings.vsync_pulse_width = LCD_VSYNC_PWIDTH;
	config.timings.vsync_back_porch = LCD_VSYNC_BPORCH;
	config.timings.flags.hsync_idle_low = true;
	config.timings.flags.vsync_idle_low = true;
	config.timings.flags.pclk_active_neg = true;
	config.timings.flags.de_idle_high = false;
	config.timings.flags.pclk_idle_high = false;	
	
	gpio_set_direction(LCD_GPIO_BL, GPIO_MODE_OUTPUT);  // example GPIO
	gpio_set_level(LCD_GPIO_BL, true);
	
	ESP_ERROR_CHECK(esp_lcd_new_rgb_panel(&config, &handle));
	ESP_ERROR_CHECK(esp_lcd_panel_reset(handle));
	ESP_ERROR_CHECK(esp_lcd_panel_init(handle));
}

static void gt911_init()
{
	i2c_config_t i2c_conf = {
    	.mode = I2C_MODE_MASTER,
    	.sda_io_num = 19,
    	.scl_io_num = 20,
    	.sda_pullup_en = GPIO_PULLUP_ENABLE,
    	.scl_pullup_en = GPIO_PULLUP_ENABLE,
    	.master.clk_speed = 400000,
	};
	i2c_param_config(TOUCH_I2C_NUM, &i2c_conf);
	i2c_driver_install(TOUCH_I2C_NUM, i2c_conf.mode, 0, 0, 0);
	
	esp_lcd_panel_io_i2c_config_t io_config = ESP_LCD_TOUCH_IO_I2C_GT911_CONFIG();

    esp_lcd_touch_io_gt911_config_t tp_gt911_config = {
        .dev_addr = io_config.dev_addr,
    };

    esp_lcd_touch_config_t tp_cfg = {
        .x_max = SCREEN_WIDTH,
        .y_max = SCREEN_HEIGHT,
        .rst_gpio_num = -1,
        .int_gpio_num = -1,
        .levels = {
            .reset = 0,
            .interrupt = 0,
        },
        .flags = {
            .swap_xy = 0,
            .mirror_x = 0,
            .mirror_y = 0,
        },
        .driver_data = &tp_gt911_config,
    };

	esp_lcd_new_panel_io_i2c(0, &io_config, &touch_handle);
    esp_lcd_touch_new_i2c_gt911(touch_handle, &tp_cfg, &tp);
}