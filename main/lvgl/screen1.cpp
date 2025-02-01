#include <string>

#include "lvgl.h"
#include "ble_mesh_provisioner.h"


static lv_obj_t *screen = NULL;
static lv_obj_t *left_pane = NULL;

static void screen_evt_cb(lv_event_t *ev)
{
    lv_event_code_t code = lv_event_get_code(ev);
    auto scr = (lv_obj_t *)lv_event_get_target(ev);
    if (code == LV_EVENT_SCREEN_LOADED)
    {
        // printf("screen 1 loaded\n");
    }
    else if (code == LV_EVENT_SCREEN_UNLOADED)
    {
        // printf("screen 1 unloaded\n");
        left_pane = nullptr;
        lv_obj_delete(scr);
    }
}

void provisioner_device_prov(const char *uuid);
static void select_event_cb(lv_event_t *e)
{
    lv_obj_t *obj = (lv_obj_t *)lv_event_get_target(e);
    uint32_t col;
    uint32_t row;
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
    lv_obj_set_size(table, LV_PCT(100), LV_PCT(100));

    lv_table_set_column_width(table, 0, 250);
    lv_table_set_column_width(table, 1, LV_PCT(75));
    lv_table_set_column_count(table, 2);

    /*Don't make the cell pressed, we will draw something different in the event*/
    lv_obj_remove_style(table, NULL, LV_PART_ITEMS | LV_STATE_PRESSED);

    uint32_t i = 0;
    auto devs = BLEmeshProvisioner::GetInstance()->getDevices();
    for (auto [key, value] : devs)
    {
        lv_table_set_cell_value(table, i, 1, key.c_str());
        char bdr[25] = {};
        sprintf(bdr, "%02X:%02X:%02X:%02X:%02X:%02X", value.addr[0], value.addr[1], value.addr[2], value.addr[3], value.addr[4], value.addr[5]);
        lv_table_set_cell_value(table, i, 0, bdr);
        i++;
    }

    lv_obj_add_event(table, select_event_cb, LV_EVENT_PRESSED, NULL);
}

void lvgl_screen1()
{
    screen = lv_obj_create(NULL);
    lv_obj_add_event(screen, screen_evt_cb, LV_EVENT_ALL, NULL);
    lv_screen_load(screen);

    left_pane = lv_obj_create(screen);
    lv_obj_set_pos(left_pane, 0, 100);
    lv_obj_set_size(left_pane, lv_pct(100), 500);

    lvgl_refresh_unprovisioned_device();
}
