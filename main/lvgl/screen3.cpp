
#include "lvgl.h"
#include "mesh.h"
#include "models.h"

#include "ble_mesh_provisioner.h"

static lv_obj_t* screen = NULL;

static void screen_evt_cb(lv_event_t *ev)
{
    lv_event_code_t code = lv_event_get_code(ev);
    auto scr = (lv_obj_t*)lv_event_get_target(ev);
    if (code == LV_EVENT_SCREEN_LOADED)
    {
        // printf("screen 3 loaded\n");
    }
    else if (code == LV_EVENT_SCREEN_UNLOADED)
    {
        // printf("screen 3 unloaded\n");
        lv_obj_delete(scr);
    }
}

static void on_btn_pressed_cb(lv_event_t *ev)
{
    auto btn = (lv_obj_t*)lv_event_get_target(ev);
    auto addr = (uint32_t)lv_event_get_user_data(ev);
    lv_obj_t * label = lv_obj_get_child(btn, 0);
    auto on = lv_label_get_text(label) == std::string("On");
    onoff_client_publish(addr, on);
}

void model_modal_mbox_open(uint16_t addr, uint16_t model);
static void model_modal_box_cb(lv_event_t *ev)
{
    auto btn = (lv_obj_t*)lv_event_get_target(ev);
    auto addr = (uint32_t)lv_event_get_user_data(ev);
    model_modal_mbox_open(addr, 0x1000);
}


static void lvgl_add_onoff_model(lv_obj_t* cont, uint16_t addr)
{
    auto c = lv_obj_create(cont);
    lv_obj_set_flex_flow(c, LV_FLEX_FLOW_COLUMN);
    lv_obj_set_size(c, LV_SIZE_CONTENT, LV_SIZE_CONTENT);
    
    auto label = lv_label_create(c);
    lv_label_set_text_fmt(label, "0x%04X  " LV_SYMBOL_SETTINGS, addr);
    // lv_obj_set_width(label, LV_SIZE_CONTENT);
    auto btn = lv_button_create(c);
    label = lv_label_create(btn);
    lv_label_set_text(label, "On");
    lv_obj_add_event(btn, on_btn_pressed_cb, LV_EVENT_SHORT_CLICKED, (void*)addr);
    btn = lv_button_create(c);
    label = lv_label_create(btn);
    lv_label_set_text(label, "Off");
    lv_obj_add_event(btn, on_btn_pressed_cb, LV_EVENT_SHORT_CLICKED, (void*)addr);

    lv_obj_add_event(c, model_modal_box_cb, LV_EVENT_SHORT_CLICKED, (void*)addr);
}

void lvgl_screen3()
{
    screen = lv_obj_create(NULL);
    lv_obj_add_event(screen, screen_evt_cb, LV_EVENT_ALL, NULL);
    lv_scr_load(screen);

    /*Create a container with ROW flex direction*/
    lv_obj_t * cont_row = lv_obj_create(screen);
    lv_obj_set_size(cont_row, 1024, LV_SIZE_CONTENT);
    // lv_obj_align(cont_row, LV_ALIGN_TOP_MID, 0, 5);
    lv_obj_set_flex_flow(cont_row, LV_FLEX_FLOW_ROW_WRAP);
    lv_obj_set_pos(cont_row, 0, 100);
    auto count = BLEmeshProvisioner::GetInstance()->nodesCount();
    auto nodes = BLEmeshProvisioner::GetInstance()->getNodes();

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
                if(elem.models[k].mod_id == 0x1000)
                {
                    lvgl_add_onoff_model(cont_row, elem.elem_addr);
                }
            }
        }        
    }    
}
