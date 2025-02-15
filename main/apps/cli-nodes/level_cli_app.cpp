#include <string.h>
#include "freertos/FreeRTOS.h"
#include "freertos/event_groups.h"
#include "esp_log.h"
#include "esp_event.h"
#include "esp_err.h"
#include "esp_check.h"

#include "level_cli_app.h"
#include "bsp/esp-bsp.h"

#include "mesh.h"
#include "models.h"

#include "ble_mesh_provisioner.h"
#define TAG "hsl"

static LevelClientApp *nodes_app = new LevelClientApp();
void mesh_model_set_level(uint16_t addr, int lvl);
static lv_obj_t *slider_level = nullptr;
static lv_obj_t *slider_sat = nullptr;
static lv_obj_t *slider_light = nullptr;

void install_level_control_app(ESP_Brookesia_Phone *phone)
{
	ESP_BROOKESIA_CHECK_NULL_EXIT(nodes_app, "Create hsl app failed");
	ESP_BROOKESIA_CHECK_FALSE_EXIT((phone->installApp(nodes_app) >= 0), "Install hsl app failed");
}

LevelClientApp::LevelClientApp() : ESP_Brookesia_PhoneApp(
									   {
										   .name = "Level",
										   .launcher_icon = ESP_BROOKESIA_STYLE_IMAGE(&esp_brookesia_image_small_app_launcher_default_98_98),
										   .screen_size = ESP_BROOKESIA_STYLE_SIZE_RECT_PERCENT(100, 100),
										   .flags = {
											   .enable_default_screen = 1,
											   .enable_recycle_resource = 1,
											   .enable_resize_visual_area = 1,
										   },
									   },
									   {
										   .app_launcher_page_index = 1,
										   .status_icon_area_index = 0,
										   .status_icon_data = {
											   .size = {},
											   .icon = {
												   .image_num = 1,
												   .images = {
													   ESP_BROOKESIA_STYLE_IMAGE(&esp_brookesia_image_small_app_launcher_default_98_98),
												   },
											   },
										   },
										   .status_bar_visual_mode = ESP_BROOKESIA_STATUS_BAR_VISUAL_MODE_SHOW_FIXED,
										   .navigation_bar_visual_mode = ESP_BROOKESIA_NAVIGATION_BAR_VISUAL_MODE_SHOW_FIXED,
										   .flags = {
											   .enable_status_icon_common_size = 1,
											   .enable_navigation_gesture = 0,
										   },
									   })
{
}

LevelClientApp::~LevelClientApp()
{
	ESP_BROOKESIA_LOGD("Destroy(@0x%p)", this);
}

void LevelClientApp::change(uint16_t hue, uint16_t lvl, uint16_t addr)
{
}

static bool screen()
{
    static lv_style_t style;
	lv_style_init(&style);
	lv_style_set_pad_left(&style, 5);
	lv_style_set_pad_right(&style, 25);
	lv_style_set_pad_bottom(&style, 0);

	auto top_bar = lv_obj_create(lv_scr_act());
	lv_obj_set_flex_flow(top_bar, LV_FLEX_FLOW_ROW);
	lv_obj_set_size(top_bar, lv_pct(100), 80);
	lv_obj_set_pos(top_bar, 0, 0);

	auto lbl = lv_label_create(top_bar);
	lv_label_set_text(lbl, "Level");
	slider_level = lv_slider_create(top_bar);
	lv_slider_set_range(slider_level, -32768, 32767);
	lv_obj_set_size(slider_level, lv_pct(22), 15);
	lv_obj_align(slider_level, LV_ALIGN_CENTER, 0, 15);

	auto container = lv_obj_create(lv_scr_act());
	lv_obj_set_flex_flow(container, LV_FLEX_FLOW_ROW_WRAP);
	lv_obj_set_size(container, lv_pct(100), lv_pct(100));
	lv_obj_set_pos(container, 0, 80);

	auto count = BLEmeshProvisioner::GetInstance()->nodesCount();
	auto nodes = BLEmeshProvisioner::GetInstance()->getNodes();

	for (size_t k = 0; k < sizeof(models_id) / sizeof(uint16_t); k++)
	{
		auto btn = lv_btn_create(container);
		auto lbl = lv_label_create(btn);
		lv_label_set_text_fmt(lbl, "0x%04X\n Level", models_id[k]);
		auto click = [](lv_event_t *ev)
		{
			auto addr = (uint32_t)lv_event_get_user_data(ev) & 0xffff;
			int level;
			level = lv_slider_get_value(slider_level);
			mesh_model_set_level(addr, level);
		};
		lv_obj_add_event_cb(btn, click, LV_EVENT_SHORT_CLICKED, (void *)(models_id[k]));
	}

	for (size_t i = 0; i < CONFIG_BLE_MESH_MAX_PROV_NODES; i++)
	{
		if (!nodes[i])
			continue;
		auto addr = nodes[i]->unicast_addr;
		auto comp = *mesh_get_composition(addr);
		for (size_t j = 0; j < comp.element_num; j++)
		{
			auto elem = comp.elements[j];
			for (size_t k = 0; k < elem.count; k++)
			{
				if (elem.models[k].mod_id == 0x1002)
				{
					auto btn = lv_btn_create(container);
					auto lbl = lv_label_create(btn);
					lv_label_set_text_fmt(lbl, "0x%04X\n Level", elem.elem_addr);
					auto click = [](lv_event_t *ev)
					{
						auto addr = (uint32_t)lv_event_get_user_data(ev) & 0xffff;
						int level;
						level = lv_slider_get_value(slider_level);
						mesh_model_set_level(addr, level);
					};
					lv_obj_add_event_cb(btn, click, LV_EVENT_SHORT_CLICKED, (void *)(elem.elem_addr));
				}
			}
		}
	}

	lv_obj_clear_flag(lv_scr_act(), LV_OBJ_FLAG_SCROLLABLE);
	return true;
}

bool LevelClientApp::run(void)
{
	ESP_BROOKESIA_LOGD("Run");

	// Create all UI resources here
	ESP_BROOKESIA_CHECK_FALSE_RETURN(screen(), true, "Main init failed");

	return true;
}

bool LevelClientApp::back(void)
{
	ESP_BROOKESIA_LOGD("Back");

	// If the app needs to exit, call notifyCoreClosed() to notify the core to close the app
	ESP_BROOKESIA_CHECK_FALSE_RETURN(notifyCoreClosed(), false, "Notify core closed failed");

	return true;
}
