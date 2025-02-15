#include <string>

#include "lvgl.h"
#include "ble_mesh_provisioner.h"


static lv_obj_t *left_pane = NULL;

void provisioner_device_prov(const char *uuid);
static void select_event_cb(lv_event_t *e)
{
	lv_obj_t *obj = (lv_obj_t *)lv_event_get_target(e);
	uint16_t col;
	uint16_t row;
	lv_table_get_selected_cell(obj, &row, &col);
	auto addr = lv_table_get_cell_value(obj, row, 0);
	auto uuid = lv_table_get_cell_value(obj, row, 1);
	if(strlen(uuid) == 0) return;
	provisioner_device_prov(uuid);
}

void lvgl_refresh_unprovisioned_device()
{
	if(!left_pane) return;
	lv_obj_clean(left_pane);
	lv_obj_t *table = lv_table_create(left_pane);

	/*Set a smaller height to the table. It'll make it scrollable*/
	lv_obj_set_size(table, LV_SIZE_CONTENT, LV_PCT(100));
	lv_table_set_col_cnt(table, 2);

	lv_table_set_col_width(table, 0, LV_PCT(20));
	lv_table_set_col_width(table, 1, LV_PCT(75));

	/*Don't make the cell pressed, we will draw something different in the event*/
	lv_obj_remove_style(table, NULL, LV_PART_ITEMS | LV_STATE_PRESSED);

	uint32_t i = 0;
	auto devs = BLEmeshProvisioner::GetInstance()->getDevices();
	for (auto [key, value] : devs)
	{
		char bdr[25] = {};
		sprintf(bdr, "%02X:%02X:%02X:%02X:%02X:%02X", value.addr[0], value.addr[1], value.addr[2], value.addr[3], value.addr[4], value.addr[5]);
		lv_table_set_cell_value(table, i, 0, bdr);
		lv_table_set_cell_value(table, i, 1, key.c_str());
		i++;
	}

	lv_obj_add_event_cb(table, select_event_cb, LV_EVENT_PRESSED, NULL);
}

bool lvgl_screen1()
{
	left_pane = lv_obj_create(lv_scr_act());
	// lv_obj_set_pos(left_pane, 0, 100);
	lv_obj_set_size(left_pane, lv_pct(100), lv_pct(100));

	lvgl_refresh_unprovisioned_device();

	return true;
}
