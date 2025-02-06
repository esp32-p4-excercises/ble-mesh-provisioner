#include <string>
#include "lvgl.h"

#include "mesh.h"
#include "models.h"

static uint16_t address = 0;
static lv_obj_t *state_lbl = nullptr;

void mesh_model_get_onoff(uint16_t addr);
void mesh_model_set_onoff(uint16_t addr, bool on);

void model_1000_modal_action(lv_obj_t* parent, uint16_t addr, bool clean)
{
    state_lbl = nullptr;
    address = addr;
    if(clean)
        lv_obj_clean(parent);
    auto mbox = lv_msgbox_create(parent);
    lv_msgbox_add_title(mbox, "Generic OnOff controls");
    lv_obj_set_width(mbox, LV_PCT(100));
    lv_obj_align(mbox, LV_ALIGN_TOP_MID, 0, 0);

    auto cont = lv_msgbox_get_content(mbox);
    if(addr < 0x8000)
    {
        mesh_model_get_onoff(address);
        auto btn = lv_msgbox_add_footer_button(mbox, "Read state");
        lv_obj_add_event(btn, [](lv_event_t *ev){
            mesh_model_get_onoff(address);
        }, LV_EVENT_SHORT_CLICKED, NULL);
        state_lbl = lv_label_create(cont);
        lv_label_set_text_fmt(state_lbl, "State: %s", "Unknown");
    }

    lv_obj_t * cont_row = lv_obj_create(cont);
    lv_obj_set_size(cont_row, LV_PCT(100), LV_SIZE_CONTENT);
    lv_obj_set_flex_flow(cont_row, LV_FLEX_FLOW_ROW_WRAP);
    lv_obj_set_pos(cont_row, 0, 0);

    auto btn = lv_button_create(cont_row);
    auto lbl = lv_label_create(btn);
    lv_label_set_text(lbl, "On");
    lv_obj_add_event(btn, [](lv_event_t *ev){
        mesh_model_set_onoff(address, 1);
    }, LV_EVENT_SHORT_CLICKED, NULL);

    btn = lv_button_create(cont_row);
    lbl = lv_label_create(btn);
    lv_label_set_text(lbl, "Off");
    lv_obj_add_event(btn, [](lv_event_t *ev){
        mesh_model_set_onoff(address, 0);
    }, LV_EVENT_SHORT_CLICKED, NULL);
}

void lvgl_update_onoff_btn(uint16_t addr, uint8_t lvl)
{
    if(address != addr or !state_lbl) return;

    if(lvl)
        lv_label_set_text_fmt(state_lbl, "State: %s", "On");
    else
        lv_label_set_text_fmt(state_lbl, "State: %s", "Off");
}
