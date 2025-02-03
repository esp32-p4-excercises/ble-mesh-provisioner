#include <string>
#include "lvgl.h"

#include "mesh.h"

static lv_obj_t *on_btn = nullptr;
static uint16_t address = 0;

void onoff_client_publish(uint16_t addr, bool on);
void mesh_model_get_onoff(uint16_t addr);

void model_1000_modal_action(lv_obj_t* parent, uint16_t addr)
{
    address = addr;
    auto mbox = lv_msgbox_create(parent);
    lv_msgbox_add_title(mbox, "Generic OnOff controls");
    lv_obj_set_width(mbox, LV_PCT(100));

    auto btn = lv_msgbox_add_footer_button(mbox, "Read state");

    on_btn = lv_msgbox_add_footer_button(mbox, "On");

    lv_obj_add_event(btn, [](lv_event_t *ev){
        mesh_model_get_onoff(address);
    }, LV_EVENT_SHORT_CLICKED, NULL);

    lv_obj_add_event(on_btn, [](lv_event_t *ev){
        auto btn = (lv_obj_t*)lv_event_get_target(ev);
        lv_obj_t * label = lv_obj_get_child(btn, 0);
        auto on = lv_label_get_text(label) != std::string("On");
        onoff_client_publish(address, on);
    }, LV_EVENT_SHORT_CLICKED, NULL);

    lv_obj_add_event(mbox, [](lv_event_t* ev){
        on_btn = nullptr;
    }, LV_EVENT_DELETE, NULL);
}

void lvgl_update_onoff_btn(uint16_t addr, uint8_t lvl)
{
    if(address != addr or !on_btn) return;
    lv_obj_t * label = lv_obj_get_child(on_btn, 0);

    if(lvl)
        lv_label_set_text(label, "On");
    else
        lv_label_set_text(label, "Off");
}
