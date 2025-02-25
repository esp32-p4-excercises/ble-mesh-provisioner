#include "lvgl.h"
#include "time_app.h"

#include "ble_mesh_provisioner.h"
#include "mesh.h"
#include "models.h"

using namespace std;

static TimeApp *provisioner_app = new TimeApp();
void mesh_model_send_time();

void install_time_app(ESP_Brookesia_Phone *phone)
{
	ESP_BROOKESIA_CHECK_NULL_EXIT(provisioner_app, "Create app provisioner failed");
	ESP_BROOKESIA_CHECK_FALSE_EXIT((phone->installApp(provisioner_app) >= 0), "Install app provisioner failed");
}

TimeApp::TimeApp() : ESP_Brookesia_PhoneApp(
								 {
									 .name = "Time",
									 .launcher_icon = ESP_BROOKESIA_STYLE_IMAGE(&esp_brookesia_image_small_app_launcher_default_98_98),
									 .screen_size = ESP_BROOKESIA_STYLE_SIZE_RECT_PERCENT(100, 100),
									 .flags = {
										 .enable_default_screen = 1,
										 .enable_recycle_resource = 1,
										 .enable_resize_visual_area = 1,
									 },
								 },
								 {
									 .app_launcher_page_index = 2,
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

TimeApp::~TimeApp()
{
	ESP_BROOKESIA_LOGD("Destroy(@0x%p)", this);
}

bool TimeApp::run(void)
{
	ESP_BROOKESIA_LOGD("Run");
    mesh_model_send_time();

	// Create all UI resources here
	// ESP_BROOKESIA_CHECK_FALSE_RETURN(screen(), false, "Main init failed");

	return false;
}

bool TimeApp::back(void)
{
	ESP_BROOKESIA_LOGW("Back");

	// If the app needs to exit, call notifyCoreClosed() to notify the core to close the app
	ESP_BROOKESIA_CHECK_FALSE_RETURN(notifyCoreClosed(), false, "Notify core closed failed");

	return true;
}

bool TimeApp::close(void)
{
	ESP_BROOKESIA_LOGD("CLOSE");
	return true;
}

bool TimeApp::resume(void)
{
	ESP_BROOKESIA_LOGD("Resume");
	return true;
}
