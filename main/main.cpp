/*
 * SPDX-FileCopyrightText: 2023-2024 Espressif Systems (Shanghai) CO LTD
 *
 * SPDX-License-Identifier: CC0-1.0
 */

#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "esp_check.h"
#include "esp_log.h"
#include "bsp/esp-bsp.h"
#include "esp_brookesia.hpp"

#define EXAMPLE_SHOW_MEM_INFO (1)

#include "provisioner_app.h"
#include "all_nodes_app.h"

static const char *TAG = "app_main";

static void on_clock_update_timer_cb(struct _lv_timer_t *t);
void init_ble_mesh();
void install_apps(ESP_Brookesia_Phone *phone);


extern "C" void app_main(void)
{
	bsp_i2c_init();
	bsp_display_cfg_t cfg = {
		.lvgl_port_cfg = ESP_LVGL_PORT_INIT_CONFIG(),
		.buffer_size = BSP_LCD_H_RES * BSP_LCD_V_RES,
		.hw_cfg = {
			.dsi_bus = 
			{
				.lane_bit_rate_mbps = 800,
			},
		},
		.flags = {
			.buff_dma = false,
			.buff_spiram = true,
			.sw_rotate = true,
		}};
	cfg.lvgl_port_cfg.task_stack = 10000;

	lv_disp_t *disp = bsp_display_start_with_config(&cfg);
	bsp_display_brightness_set(100);
	lv_disp_set_rotation(disp, LV_DISP_ROT_180);

	ESP_LOGI(TAG, "Display ESP-Brookesia phone demo");
	/**
	 * To avoid errors caused by multiple tasks simultaneously accessing LVGL,
	 * should acquire a lock before operating on LVGL.
	 */
	bsp_display_lock(0);

	/* Create a phone object */
	ESP_Brookesia_Phone *phone = new ESP_Brookesia_Phone(disp);
	ESP_BROOKESIA_CHECK_NULL_EXIT(phone, "Create phone failed");

	/* Try using a stylesheet that corresponds to the resolution */
	ESP_Brookesia_PhoneStylesheet_t *stylesheet = nullptr;
	stylesheet = new ESP_Brookesia_PhoneStylesheet_t ESP_BROOKESIA_PHONE_1024_600_DARK_STYLESHEET();
	ESP_BROOKESIA_CHECK_NULL_EXIT(stylesheet, "Create stylesheet failed");

	if (stylesheet != nullptr)
	{	
		ESP_LOGI(TAG, "Using stylesheet (%s)", stylesheet->core.name);
		ESP_BROOKESIA_CHECK_FALSE_EXIT(phone->addStylesheet(stylesheet), "Add stylesheet failed");
		ESP_BROOKESIA_CHECK_FALSE_EXIT(phone->activateStylesheet(stylesheet), "Activate stylesheet failed");
		delete stylesheet;
	}

	/* Configure and begin the phone */
	ESP_BROOKESIA_CHECK_FALSE_EXIT(phone->setTouchDevice(bsp_display_get_input_dev()), "Set touch device failed");
	phone->registerLvLockCallback((ESP_Brookesia_LvLockCallback_t)(bsp_display_lock), 0);
	phone->registerLvUnlockCallback((ESP_Brookesia_LvUnlockCallback_t)(bsp_display_unlock));
	ESP_BROOKESIA_CHECK_FALSE_EXIT(phone->begin(), "Begin failed");

	/* Create a timer to update the clock */
	lv_timer_create(on_clock_update_timer_cb, 1000, phone);

	phone->getHome().getStatusBar()->hideBatteryIcon();
	phone->getHome().getStatusBar()->hideBatteryPercent();
	phone->getHome().getStatusBar()->setClockFormat(ESP_Brookesia_StatusBar::ClockFormat::FORMAT_24H);

	/* Release the lock */
	bsp_display_unlock();


	init_ble_mesh();

	/* Install apps */
	phone->lockLv();

	install_apps(phone);

	phone->unlockLv();


	char buffer[128];    /* Make sure buffer is enough for `sprintf` */
    size_t internal_free = 0;
    size_t internal_total = 0;
    size_t external_free = 0;
    size_t external_total = 0;
    while (1) {
        internal_free = heap_caps_get_free_size(MALLOC_CAP_INTERNAL);
        internal_total = heap_caps_get_total_size(MALLOC_CAP_INTERNAL);
        external_free = heap_caps_get_free_size(MALLOC_CAP_SPIRAM);
        external_total = heap_caps_get_total_size(MALLOC_CAP_SPIRAM);
        // sprintf(buffer, "   Biggest /     Free /    Total\n"
        //         "\t  SRAM : [%8d / %8d / %8d]\n"
        //         "\t PSRAM : [%8d / %8d / %8d]",
        //         heap_caps_get_largest_free_block(MALLOC_CAP_INTERNAL), internal_free, internal_total,
        //         heap_caps_get_largest_free_block(MALLOC_CAP_SPIRAM), external_free, external_total);
        // ESP_LOGI("MEM", "%s", buffer);

        /**
         * The `lockLv()` and `unlockLv()` functions are used to lock and unlock the LVGL task.
         * They are registered by the `registerLvLockCallback()` and `registerLvUnlockCallback()` functions.
         */
        phone->lockLv();
        // Update memory label on "Recents Screen"
        if (!phone->getHome().getRecentsScreen()->setMemoryLabel(
                    internal_free / 1024, internal_total / 1024, external_free / 1024, external_total / 1024
                )) {
            ESP_LOGE(TAG, "Set memory label failed");
        }
        phone->unlockLv();

        vTaskDelay(pdMS_TO_TICKS(2000));
    }
}

static void on_clock_update_timer_cb(struct _lv_timer_t *t)
{
	time_t now;
	struct tm timeinfo;
	bool is_time_pm = false;
	ESP_Brookesia_Phone *phone = (ESP_Brookesia_Phone *)t->user_data;

	time(&now);
	localtime_r(&now, &timeinfo);
	is_time_pm = (timeinfo.tm_hour >= 12);

	/* Since this callback is called from LVGL task, it is safe to operate LVGL */
	// Update clock on "Status Bar"
	ESP_BROOKESIA_CHECK_FALSE_EXIT(
		phone->getHome().getStatusBar()->setClock(timeinfo.tm_hour, timeinfo.tm_min, is_time_pm),
		"Refresh status bar failed");
}
