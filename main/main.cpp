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
//  #include "app_examples/phone/simple_conf/src/phone_app_simple_conf.hpp"
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
		.flags = {
			.buff_dma = false,
			.buff_spiram = true,
			.sw_rotate = false,
		}};
	cfg.lvgl_port_cfg.task_stack = 10000;

	lv_disp_t *disp = bsp_display_start_with_config(&cfg);
	bsp_display_brightness_set(25);

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
