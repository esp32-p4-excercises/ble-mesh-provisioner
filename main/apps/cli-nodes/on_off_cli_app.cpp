#include <string.h>
#include "freertos/FreeRTOS.h"
#include "freertos/event_groups.h"
#include "esp_log.h"
#include "esp_event.h"
#include "esp_err.h"
#include "esp_check.h"

#include "bsp/esp-bsp.h"
#include "on_off_cli_app.h"

#include "mesh.h"
#include "models.h"

#include "ble_mesh_provisioner.h"

#define TAG "on-off"

void mesh_model_set_onoff(uint16_t addr, bool on);

static OnOffClientApp *on_cli_app = nullptr;
static OnOffClientApp *off_cli_app = nullptr;

void install_on_off_control_app(ESP_Brookesia_Phone *phone, uint8_t lvl)
{
	if (lvl)
	{
		on_cli_app = new OnOffClientApp("On", lvl);
		ESP_BROOKESIA_CHECK_NULL_EXIT(on_cli_app, "Create On app failed");
		ESP_BROOKESIA_CHECK_FALSE_EXIT((phone->installApp(on_cli_app) >= 0), "Install On app failed");
	}
	else
	{
		off_cli_app = new OnOffClientApp("Off", lvl);
		ESP_BROOKESIA_CHECK_NULL_EXIT(off_cli_app, "Create Off app failed");
		ESP_BROOKESIA_CHECK_FALSE_EXIT((phone->installApp(off_cli_app) >= 0), "Install Off app failed");
	}
}

OnOffClientApp::OnOffClientApp(const char *name, uint8_t lvl) : ESP_Brookesia_PhoneApp(
																	{
																		.name = name,
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
	level = lvl;
}

OnOffClientApp::~OnOffClientApp()
{
	ESP_BROOKESIA_LOGD("Destroy(@0x%p)", this);
}

void OnOffClientApp::change(uint16_t addr)
{
	mesh_model_set_onoff(addr, level);
}

static bool screen(uint8_t lvl)
{
	auto container = lv_obj_create(lv_scr_act());
	lv_obj_set_flex_flow(container, LV_FLEX_FLOW_ROW_WRAP);
	lv_obj_set_size(container, lv_pct(100), lv_pct(100));
	lv_obj_set_pos(container, 0, 0);

	auto count = BLEmeshProvisioner::GetInstance()->nodesCount();
	auto nodes = BLEmeshProvisioner::GetInstance()->getNodes();

	for (size_t k = 0; k < sizeof(models_id) / sizeof(uint16_t); k++)
	{
		auto btn = lv_btn_create(container);
		auto lbl = lv_label_create(btn);
		lv_label_set_text_fmt(lbl, "0x%04X\n%s", models_id[k], lvl ? "On" : "Off");
		auto click = [](lv_event_t *ev)
		{
			auto addr = (uint32_t)lv_event_get_user_data(ev);
			auto lvl = (uint32_t)lv_event_get_user_data(ev) >> 16;
			mesh_model_set_onoff(addr, lvl);
		};
		lv_obj_add_event_cb(btn, click, LV_EVENT_SHORT_CLICKED, (void *)(models_id[k] + (lvl << 16)));
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
				if (elem.models[k].mod_id == 0x1000)
				{
					auto btn = lv_btn_create(container);
					auto lbl = lv_label_create(btn);
					lv_label_set_text_fmt(lbl, "0x%04X\n%s", addr, lvl ? "On" : "Off");
					auto click = [](lv_event_t *ev)
					{
						auto addr = (uint32_t)lv_event_get_user_data(ev) & 0xffff;
						auto lvl = (uint32_t)lv_event_get_user_data(ev) >> 16;
						mesh_model_set_onoff(addr, lvl);
					};
					lv_obj_add_event_cb(btn, click, LV_EVENT_SHORT_CLICKED, (void *)(addr + j + (lvl << 16)));
				}
			}
		}
	}

	lv_obj_clear_flag(lv_scr_act(), LV_OBJ_FLAG_SCROLLABLE);
	return true;
}

bool OnOffClientApp::run(void)
{
	ESP_BROOKESIA_LOGD("Run");

	// Create all UI resources here
	ESP_BROOKESIA_CHECK_FALSE_RETURN(screen(level), true, "Main init failed");

	return true;
}

bool OnOffClientApp::back(void)
{
	ESP_BROOKESIA_LOGD("Back");

	// If the app needs to exit, call notifyCoreClosed() to notify the core to close the app
	ESP_BROOKESIA_CHECK_FALSE_RETURN(notifyCoreClosed(), false, "Notify core closed failed");

	return true;
}
