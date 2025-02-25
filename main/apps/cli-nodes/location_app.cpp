#include "lvgl.h"
#include "location_app.h"

#include "ble_mesh_provisioner.h"
#include "mesh.h"
#include "models.h"

using namespace std;

static LocationApp *provisioner_app = new LocationApp();
void mesh_model_send_global_location(uint16_t addr, esp_ble_mesh_gen_location_state_t state);
void mesh_model_send_local_location(uint16_t addr, esp_ble_mesh_gen_location_state_t state);

void install_location_app(ESP_Brookesia_Phone *phone)
{
	ESP_BROOKESIA_CHECK_NULL_EXIT(provisioner_app, "Create app provisioner failed");
	ESP_BROOKESIA_CHECK_FALSE_EXIT((phone->installApp(provisioner_app) >= 0), "Install app provisioner failed");
}

LocationApp::LocationApp() : ESP_Brookesia_PhoneApp(
								 {
									 .name = "Location",
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

LocationApp::~LocationApp()
{
	ESP_BROOKESIA_LOGD("Destroy(@0x%p)", this);
}

static lv_obj_t *container = NULL;

static uint16_t tab_sel = 0;
static lv_obj_t *north_area = nullptr;
static lv_obj_t *east_area = nullptr;
static lv_obj_t *g_altitude = nullptr;
static lv_obj_t *floor_area = nullptr;
static lv_obj_t *lat_area = nullptr;
static lv_obj_t *long_area = nullptr;
static lv_obj_t *l_altitude = nullptr;

static lv_obj_t *kb = nullptr;

static void lv_example_keyboard_2(void)
{
	/*Create an AZERTY keyboard map*/
	static const char *kb_map[] = {"1", "2", "3", "4", "5", "\n",
								   "6", "7", "8", "9", "0", "\n",
								   "A", "B", "C", "D", "E", "F", "\n",
								   LV_SYMBOL_LEFT, LV_SYMBOL_RIGHT, LV_SYMBOL_BACKSPACE, LV_SYMBOL_OK, NULL};

	/*Set the relative width of the buttons and other controls*/
	static const lv_btnmatrix_ctrl_t kb_ctrl[] = {
		4, 4, 4, 4, 4,
		4, 4, 4, 4, 4,
		4, 4, 4, 4, 4,
		4, 4, 4, 4, 4,
		//  2, LV_BUTTONMATRIX_CTRL_HIDDEN | 2, 6, LV_BUTTONMATRIX_CTRL_HIDDEN | 2, 2
	};

	lv_keyboard_set_map(kb, LV_KEYBOARD_MODE_USER_1, kb_map, kb_ctrl);
	lv_keyboard_set_mode(kb, LV_KEYBOARD_MODE_USER_1);
}

static inline void lv_example_keyboard(void)
{
	/*Create a keyboard to use it with an of the text areas*/
	kb = lv_keyboard_create(lv_layer_top());
	lv_obj_set_size(kb, lv_pct(50), lv_pct(35));
	lv_obj_align(kb, LV_ALIGN_BOTTOM_RIGHT, 0, -70);
	lv_example_keyboard_2();
	// lv_obj_add_flag(kb, LV_OBJ_FLAG_HIDDEN);
	// lv_keyboard_set_textarea(kb, address_ta);
}

static bool screen()
{
	container = lv_obj_create(lv_scr_act());
	lv_obj_set_pos(container, 0, 0);
	lv_obj_set_size(container, lv_pct(100), lv_pct(100));
	lv_obj_t *left_pane = lv_obj_create(container);
	lv_obj_t *right_pane = lv_obj_create(container);

	lv_obj_set_pos(right_pane, lv_pct(50), 0);
	lv_obj_set_size(left_pane, lv_pct(50), lv_pct(100));
	lv_obj_set_size(right_pane, lv_pct(50), lv_pct(100));

	{
		auto tabview = lv_tabview_create(left_pane, LV_DIR_TOP, 50);
		lv_obj_t *tab1 = lv_tabview_add_tab(tabview, "Global");
		lv_obj_t *tab2 = lv_tabview_add_tab(tabview, "Local");
		lv_obj_set_flex_flow(tab1, LV_FLEX_FLOW_COLUMN);
		// lv_obj_set_size(tab1, lv_pct(100), LV_SIZE_CONTENT);
		lv_obj_set_flex_flow(tab2, LV_FLEX_FLOW_COLUMN);
		// lv_obj_set_size(tab2, lv_pct(100), LV_SIZE_CONTENT);

		{ // global
			auto latitude = lv_obj_create(tab1);
			auto longitude = lv_obj_create(tab1);
			auto altitude = lv_obj_create(tab1);
			lv_obj_set_size(latitude, lv_pct(100), LV_SIZE_CONTENT);
			lv_obj_set_size(longitude, lv_pct(100), LV_SIZE_CONTENT);
			lv_obj_set_size(altitude, lv_pct(100), LV_SIZE_CONTENT);

			{
				lv_obj_t *lbl = lv_label_create(latitude);
				lv_label_set_text(lbl, "Lat: ");
				lat_area = lv_textarea_create(latitude);
				lv_obj_align(lat_area, LV_ALIGN_LEFT_MID, 70, 0);
				lv_obj_set_size(lat_area, lv_pct(70), LV_SIZE_CONTENT);
				lv_obj_add_event_cb(lat_area, [](lv_event_t *ev){
					auto ta = lv_event_get_target(ev);
					lv_keyboard_set_textarea(kb, ta);
				}, LV_EVENT_FOCUSED, NULL);
			}
			{
				lv_obj_t *lbl = lv_label_create(longitude);
				lv_label_set_text(lbl, "Long: ");
				long_area = lv_textarea_create(longitude);
				lv_obj_align(long_area, LV_ALIGN_LEFT_MID, 70, 0);
				// lv_obj_align_to(area, lbl, LV_ALIGN_LEFT_MID, 0, 0);
				lv_obj_set_size(long_area, lv_pct(70), LV_SIZE_CONTENT);
				lv_obj_add_event_cb(long_area, [](lv_event_t *ev){
					auto ta = lv_event_get_target(ev);
					lv_keyboard_set_textarea(kb, ta);
				}, LV_EVENT_FOCUSED, NULL);
			}
			{
				lv_obj_t *lbl = lv_label_create(altitude);
				lv_label_set_text(lbl, "Alt: ");
				g_altitude = lv_textarea_create(altitude);
				lv_obj_align(g_altitude, LV_ALIGN_LEFT_MID, 70, 0);
				// lv_obj_align_to(area, lbl, LV_ALIGN_LEFT_MID, 0, 0);
				lv_obj_set_size(g_altitude, lv_pct(70), LV_SIZE_CONTENT);
				lv_obj_add_event_cb(g_altitude, [](lv_event_t *ev){
					auto ta = lv_event_get_target(ev);
					lv_keyboard_set_textarea(kb, ta);
				}, LV_EVENT_FOCUSED, NULL);
			}
		}
		{ // local
			auto north = lv_obj_create(tab2);
			auto east = lv_obj_create(tab2);
			auto altitude = lv_obj_create(tab2);
			auto floor_number = lv_obj_create(tab2);
			// lv_obj_t *uncertainty = lv_obj_create(tab2);
			lv_obj_set_size(north, lv_pct(100), LV_SIZE_CONTENT);
			lv_obj_set_size(east, lv_pct(100), LV_SIZE_CONTENT);
			lv_obj_set_size(altitude, lv_pct(100), LV_SIZE_CONTENT);
			lv_obj_set_size(floor_number, lv_pct(100), LV_SIZE_CONTENT);
			// lv_obj_set_size(longitude, lv_pct(100), LV_SIZE_CONTENT);

			{
				lv_obj_t *lbl = lv_label_create(north);
				lv_label_set_text(lbl, "North: ");
				north_area = lv_textarea_create(north);
				lv_obj_align(north_area, LV_ALIGN_LEFT_MID, 70, 0);
				lv_obj_set_size(north_area, lv_pct(70), LV_SIZE_CONTENT);
				lv_obj_add_event_cb(north_area, [](lv_event_t *ev){
					auto ta = lv_event_get_target(ev);
					lv_keyboard_set_textarea(kb, ta);
				}, LV_EVENT_FOCUSED, NULL);
			}
			{
				lv_obj_t *lbl = lv_label_create(east);
				lv_label_set_text(lbl, "Lat: ");
				east_area = lv_textarea_create(east);
				lv_obj_align(east_area, LV_ALIGN_LEFT_MID, 70, 0);
				lv_obj_set_size(east_area, lv_pct(70), LV_SIZE_CONTENT);
				lv_obj_add_event_cb(east_area, [](lv_event_t *ev){
					auto ta = lv_event_get_target(ev);
					lv_keyboard_set_textarea(kb, ta);
				}, LV_EVENT_FOCUSED, NULL);
			}
			{
				lv_obj_t *lbl = lv_label_create(altitude);
				lv_label_set_text(lbl, "Alt: ");
				l_altitude = lv_textarea_create(altitude);
				lv_obj_align(l_altitude, LV_ALIGN_LEFT_MID, 70, 0);
				lv_obj_set_size(l_altitude, lv_pct(70), LV_SIZE_CONTENT);
				lv_obj_add_event_cb(l_altitude, [](lv_event_t *ev){
					auto ta = lv_event_get_target(ev);
					lv_keyboard_set_textarea(kb, ta);
				}, LV_EVENT_FOCUSED, NULL);
			}
			{
				lv_obj_t *lbl = lv_label_create(floor_number);
				lv_label_set_text(lbl, "Floor: ");
				floor_area = lv_textarea_create(floor_number);
				lv_obj_align(floor_area, LV_ALIGN_LEFT_MID, 70, 0);
				lv_obj_set_size(floor_area, lv_pct(70), LV_SIZE_CONTENT);
				lv_obj_add_event_cb(floor_area, [](lv_event_t *ev){
					auto ta = lv_event_get_target(ev);
					lv_keyboard_set_textarea(kb, ta);
				}, LV_EVENT_FOCUSED, NULL);
			}
		}

		lv_obj_add_event_cb(tabview, [](lv_event_t *ev)
							{
			auto code  = lv_event_get_code(ev);
			auto tabview = lv_event_get_target(ev);
			printf("code: %d => %d\n", code, lv_tabview_get_tab_act(tabview));
			if(lv_tabview_get_tab_act(tabview) < 10) tab_sel = lv_tabview_get_tab_act(tabview); }, LV_EVENT_VALUE_CHANGED, NULL);
	}

	auto count = BLEmeshProvisioner::GetInstance()->nodesCount();
	auto nodes = BLEmeshProvisioner::GetInstance()->getNodes();

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
				if (elem.models[k].mod_id == ESP_BLE_MESH_MODEL_ID_GEN_LOCATION_SETUP_SRV)
				{
					auto btn = lv_btn_create(right_pane);
					auto lbl = lv_label_create(btn);
					lv_label_set_text_fmt(lbl, "0x%04X\nLoc setup", elem.elem_addr);
					auto click = [](lv_event_t *ev)
					{
						auto addr = (uint32_t)lv_event_get_user_data(ev) & 0xffff;
						if (tab_sel)
						{
							esp_ble_mesh_gen_location_state_t state = {
								.local_north = (int16_t)strtol(lv_textarea_get_text(north_area), NULL, 10),
								.local_east = (int16_t)strtol(lv_textarea_get_text(east_area), NULL, 10),
								.local_altitude = (int16_t)strtol(lv_textarea_get_text(l_altitude), NULL, 10),
								.floor_number = (uint8_t)strtol(lv_textarea_get_text(floor_area), NULL, 10),
								.uncertainty = 0,
							};
							mesh_model_send_local_location(addr, state);
						}
						else
						{
							esp_ble_mesh_gen_location_state_t state = {
								.global_latitude =   strtol(lv_textarea_get_text(lat_area), NULL, 10),
								.global_longitude =   strtol(lv_textarea_get_text(long_area), NULL, 10),
								.global_altitude =  (int16_t)strtol(lv_textarea_get_text(g_altitude), NULL, 10),
							};
							mesh_model_send_global_location(addr, state);
						}
					};
					lv_obj_add_event_cb(btn, click, LV_EVENT_SHORT_CLICKED, (void *)(elem.elem_addr));
				}
			}
		}
	}

	lv_example_keyboard();

	return true;
}

bool LocationApp::run(void)
{
	ESP_BROOKESIA_LOGD("Run");

	// Create all UI resources here
	ESP_BROOKESIA_CHECK_FALSE_RETURN(screen(), false, "Main init failed");

	return true;
}

bool LocationApp::back(void)
{
	ESP_BROOKESIA_LOGW("Back");

	lv_obj_add_flag(kb, LV_OBJ_FLAG_HIDDEN);
	// If the app needs to exit, call notifyCoreClosed() to notify the core to close the app
	ESP_BROOKESIA_CHECK_FALSE_RETURN(notifyCoreClosed(), false, "Notify core closed failed");

	return true;
}

bool LocationApp::close(void)
{
	ESP_BROOKESIA_LOGD("CLOSE");
	lv_obj_del(kb);
	return true;
}

bool LocationApp::pause(void)
{
	lv_obj_add_flag(kb, LV_OBJ_FLAG_HIDDEN);
	return true;
}

bool LocationApp::resume(void)
{
	ESP_BROOKESIA_LOGD("Resume");
	lv_obj_clear_flag(kb, LV_OBJ_FLAG_HIDDEN);
	return true;
}
