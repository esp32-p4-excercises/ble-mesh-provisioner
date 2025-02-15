#include "lvgl.h"
#include "provisioner_app.h"

#include "ble_mesh_provisioner.h"
#include "mesh.h"
#include "models.h"

using namespace std;

bool lvgl_screen1();
static BLEProvisionerApp *provisioner_app = new BLEProvisionerApp();

void install_provisioner_app(ESP_Brookesia_Phone *phone)
{
	ESP_BROOKESIA_CHECK_NULL_EXIT(provisioner_app, "Create app provisioner failed");
	ESP_BROOKESIA_CHECK_FALSE_EXIT((phone->installApp(provisioner_app) >= 0), "Install app provisioner failed");
}

BLEProvisionerApp::BLEProvisionerApp() : ESP_Brookesia_PhoneApp(
											 {
												 .name = "Provisioner",
												 .launcher_icon = ESP_BROOKESIA_STYLE_IMAGE(&esp_brookesia_image_small_app_launcher_default_98_98),
												 .screen_size = ESP_BROOKESIA_STYLE_SIZE_RECT_PERCENT(100, 100),
												 .flags = {
													 .enable_default_screen = 1,
													 .enable_recycle_resource = 1,
													 .enable_resize_visual_area = 1,
												 },
											 },
											 {
												 .app_launcher_page_index = 0,
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

BLEProvisionerApp::~BLEProvisionerApp()
{
	ESP_BROOKESIA_LOGD("Destroy(@0x%p)", this);
}

static lv_obj_t *left_pane = NULL;

void provisioner_device_prov(const char *uuid);
static void select_event_cb(lv_event_t *e)
{
	lv_obj_t *t = (lv_obj_t *)lv_event_get_target(e);
	auto lbl = lv_obj_get_child(t, 2);
	auto uuid = lv_label_get_text(lbl);
	provisioner_device_prov(uuid);
}

void lvgl_refresh_unprovisioned_device()
{
	if (!left_pane)
		return;
	lv_obj_clean(left_pane);
	lv_obj_t *container = lv_table_create(left_pane);
	lv_obj_set_size(container, LV_PCT(100), LV_PCT(100));
	lv_obj_set_flex_flow(container, LV_FLEX_FLOW_COLUMN);

	auto devs = BLEmeshProvisioner::GetInstance()->getDevices();
	uint8_t n = 1;
	for (auto [key, value] : devs)
	{
		lv_obj_t *cont = lv_obj_create(container);
		lv_obj_set_size(cont, LV_PCT(100), LV_SIZE_CONTENT);
		lv_obj_set_flex_flow(cont, LV_FLEX_FLOW_ROW);

		auto lbl = lv_label_create(cont);
		lv_label_set_text_fmt(lbl, "%d.", n++);
		lbl = lv_label_create(cont);
		lv_label_set_text_fmt(lbl, "%02X:%02X:%02X:%02X:%02X:%02X", value.addr[0], value.addr[1], value.addr[2], value.addr[3], value.addr[4], value.addr[5]);
		lbl = lv_label_create(cont);
		lv_label_set_text_fmt(lbl, "%s", key.c_str());

		lv_obj_add_event_cb(cont, select_event_cb, LV_EVENT_SHORT_CLICKED, NULL);
	}
}

static bool screen()
{
	left_pane = lv_obj_create(lv_scr_act());
	lv_obj_set_pos(left_pane, 0, 0);
	lv_obj_set_size(left_pane, lv_pct(100), lv_pct(100));

	lvgl_refresh_unprovisioned_device();

	return true;
}

bool BLEProvisionerApp::run(void)
{
	ESP_BROOKESIA_LOGD("Run");

	// Create all UI resources here
	ESP_BROOKESIA_CHECK_FALSE_RETURN(screen(), false, "Main init failed");

	return true;
}

bool BLEProvisionerApp::back(void)
{
	ESP_BROOKESIA_LOGW("Back");

	// If the app needs to exit, call notifyCoreClosed() to notify the core to close the app
	ESP_BROOKESIA_CHECK_FALSE_RETURN(notifyCoreClosed(), false, "Notify core closed failed");

	return true;
}

bool BLEProvisionerApp::close(void)
{
	ESP_BROOKESIA_LOGD("CLOSE");
	left_pane = nullptr;
	return true;
}

bool BLEProvisionerApp::resume(void)
{
	ESP_BROOKESIA_LOGD("Resume");
	lvgl_refresh_unprovisioned_device();
    return true;
}
