#include <stdio.h>
#include "bsp/esp-bsp.h"
#include "bsp/display.h"
#include "lvgl.h"

void init_ble_mesh();
void lvgl_top_menu();

extern "C"
void app_main(void)
{
    bsp_display_cfg_t cfg = {
        .lvgl_port_cfg = ESP_LVGL_PORT_INIT_CONFIG(),
        .buffer_size = 1024 * 100,
        .double_buffer = BSP_LCD_DRAW_BUFF_DOUBLE,
        .flags = {
            .buff_dma = true,
            .buff_spiram = true,
            .sw_rotate = true,
        }
    };
    cfg.lvgl_port_cfg.task_stack = 10000;
    bsp_display_start_with_config(&cfg);
    bsp_display_brightness_set(10);

    bsp_display_lock(0);

    lvgl_top_menu();
    bsp_display_unlock();

    init_ble_mesh();
}
