#include "all_nodes_app.h"

#include "lvgl.h"

using namespace std;

bool lvgl_screen2();
void refresh_all_nodes();

static AllNodesApp *nodes_app = new AllNodesApp();

void install_nodes_app(ESP_Brookesia_Phone *phone)
{
	ESP_BROOKESIA_CHECK_NULL_EXIT(nodes_app, "Create app all nodes failed");
	ESP_BROOKESIA_CHECK_FALSE_EXIT((phone->installApp(nodes_app) >= 0), "Install app all nodes failed");
}

AllNodesApp::AllNodesApp() : ESP_Brookesia_PhoneApp(
	{
		.name = "Nodes",
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

AllNodesApp::~AllNodesApp()
{
	ESP_BROOKESIA_LOGD("Destroy(@0x%p)", this);
}

bool AllNodesApp::run(void)
{
	ESP_BROOKESIA_LOGD("Run");

	// Create all UI resources here
	ESP_BROOKESIA_CHECK_FALSE_RETURN(lvgl_screen2(), false, "Main init failed");

	return true;
}

bool AllNodesApp::back(void)
{
	ESP_BROOKESIA_LOGD("Back");

	// If the app needs to exit, call notifyCoreClosed() to notify the core to close the app
	ESP_BROOKESIA_CHECK_FALSE_RETURN(notifyCoreClosed(), false, "Notify core closed failed");

	return true;
}

bool AllNodesApp::close(void)
{
	ESP_BROOKESIA_LOGD("Close");
    return true;
}

bool AllNodesApp::resume(void)
{
	ESP_BROOKESIA_LOGD("Resume");
	refresh_all_nodes();
	return true;
}
