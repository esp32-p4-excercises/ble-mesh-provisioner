#include <stdio.h>

#include "esp_check.h"
#include "esp_err.h"
#include "lvgl.h"


#include "mesh.h"
#include "models.h"
#include "ble_mesh_provisioner.h"


static lv_obj_t *screen = NULL;
static lv_obj_t *left_pane = NULL;
static lv_obj_t *right_pane = NULL;

static void screen_evt_cb(lv_event_t *ev)
{
    lv_event_code_t code = lv_event_get_code(ev);
    auto scr = (lv_obj_t *)lv_event_get_target(ev);
    if (code == LV_EVENT_SCREEN_LOADED)
    {
    }
    else if (code == LV_EVENT_SCREEN_UNLOADED)
    {
        lv_obj_delete(scr);
    }
}


void lvgl_screen5()
{
    screen = lv_obj_create(NULL);
    lv_screen_load(screen);
    lv_obj_add_event(screen, screen_evt_cb, LV_EVENT_ALL, NULL);

    left_pane = lv_obj_create(screen);
    right_pane = lv_obj_create(screen);
    lv_obj_set_pos(left_pane, 0, 100);
    lv_obj_set_pos(right_pane, LV_PCT(60), 100);
    lv_obj_set_size(left_pane, LV_PCT(60), LV_PCT(100));
    lv_obj_set_size(right_pane, LV_PCT(40), LV_PCT(100));

    lv_obj_t * cont_row = lv_obj_create(left_pane);
    lv_obj_set_size(cont_row, LV_PCT(100), LV_PCT(100));
    lv_obj_set_flex_flow(cont_row, LV_FLEX_FLOW_ROW_WRAP);
    lv_obj_set_pos(cont_row, 0, 0);

    auto count = BLEmeshProvisioner::GetInstance()->nodesCount();
    auto nodes = BLEmeshProvisioner::GetInstance()->getNodes();

    for (size_t k = 0; k < sizeof(models_id) / sizeof(uint16_t); k++)
    {
        auto btn = lv_button_create(cont_row);
        auto lbl = lv_label_create(btn);
        lv_label_set_text_fmt(lbl, "HSL: 0x%04X", models_id[k]);
        auto click = [](lv_event_t* ev)
        {
            auto addr = (uint32_t)lv_event_get_user_data(ev);
            model_1307_modal_action(right_pane, addr);
        };
        lv_obj_add_event(btn, click, LV_EVENT_SHORT_CLICKED, (void*)models_id[k]);
    }

    for (size_t i = 0; i < CONFIG_BLE_MESH_MAX_PROV_NODES; i++)
    {
        if(!nodes[i]) continue;
        auto addr = nodes[i]->unicast_addr;
        auto comp = *mesh_get_composition(addr);
        for (size_t j = 0; j < comp.element_num; j++)
        {
            auto elem = comp.elements[j];
            for (size_t k = 0; k < elem.count; k++)
            {
                if(elem.models[k].mod_id == 0x1307)
                {
                    auto btn = lv_button_create(cont_row);
                    auto lbl = lv_label_create(btn);
                    lv_label_set_text_fmt(lbl, "HSL: 0x%04X", addr);
                    auto click = [](lv_event_t* ev)
                    {
                        auto addr = (uint32_t)lv_event_get_user_data(ev);
                        model_1307_modal_action(right_pane, addr);
                    };
                    lv_obj_add_event(btn, click, LV_EVENT_SHORT_CLICKED, (void*)addr);
                }
            }
        }        
    }
}

