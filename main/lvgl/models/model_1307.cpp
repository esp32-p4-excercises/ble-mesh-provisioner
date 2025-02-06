#include "lvgl.h"

#include "mesh.h"

void mesh_model_get_hsl(uint16_t address);
void mesh_model_set_hsl(uint16_t addr, uint16_t hue, uint16_t sat, uint16_t light);

static uint16_t address = 0, hue_val, saturation, lightness;
static lv_obj_t *slider_hue = nullptr;
static lv_obj_t *slider_sat = nullptr;
static lv_obj_t *slider_light = nullptr;


static void delete_sliders(lv_event_t *ev)
{
    slider_light = nullptr;
    slider_sat = nullptr;
    slider_hue = nullptr;
}

void model_1307_modal_action(lv_obj_t* parent, uint16_t addr, bool clean)
{
    address = addr;
    if(clean)
        lv_obj_clean(parent);
    auto mbox = lv_msgbox_create(parent);
    lv_msgbox_add_title(mbox, "HSL light controls");
    lv_obj_set_width(mbox, LV_PCT(100));
    lv_obj_align(mbox, LV_ALIGN_TOP_MID, 0, 0);

    auto cont = lv_msgbox_get_content(mbox);

    lv_obj_t * cont_row = lv_obj_create(cont);
    lv_obj_set_size(cont_row, LV_PCT(100), LV_SIZE_CONTENT);
    lv_obj_set_flex_flow(cont_row, LV_FLEX_FLOW_COLUMN);
    lv_obj_set_pos(cont_row, 0, 0);
    lv_obj_add_event_cb(cont_row, delete_sliders, LV_EVENT_DELETE, NULL);
    auto label = lv_label_create(cont_row);
    lv_label_set_text_fmt(label, "Address: 0x%04X", addr);

    auto lbl = lv_label_create(cont_row);
    lv_label_set_text(lbl, "Hue");
    slider_hue = lv_slider_create(cont_row);
    lv_slider_set_range(slider_hue, 0, 65535);
    lv_obj_center(slider_hue);

    lbl = lv_label_create(cont_row);
    lv_label_set_text(lbl, "Saturation");
    slider_sat = lv_slider_create(cont_row);
    lv_slider_set_range(slider_sat, 0, 65535);
    lv_obj_center(slider_sat);

    lbl = lv_label_create(cont_row);
    lv_label_set_text(lbl, "Lightness");
    slider_light = lv_slider_create(cont_row);
    lv_slider_set_range(slider_light, 0, 65535);
    lv_obj_center(slider_light);

    auto btn = lv_button_create(cont_row);
    lbl = lv_label_create(btn);
    lv_label_set_text(lbl, "Set");
    lv_obj_add_event(btn, [](lv_event_t *ev){
        uint16_t hue_val, saturation, lightness;
        hue_val = lv_slider_get_value(slider_hue);
        saturation = lv_slider_get_value(slider_sat);
        lightness = lv_slider_get_value(slider_light);
        mesh_model_set_hsl(address, hue_val, saturation, lightness);
    }, LV_EVENT_SHORT_CLICKED, NULL);

    if(addr < 0x8000)
    {
        mesh_model_get_hsl(address);
        btn = lv_msgbox_add_footer_button(mbox, "Read state");
        lv_obj_add_event(btn, [](lv_event_t *ev){
            mesh_model_get_hsl(address);
        }, LV_EVENT_SHORT_CLICKED, NULL);
    }
}

void lvgl_update_hue_sliders(uint16_t hue, uint16_t sat, uint16_t light)
{
    if(slider_hue)
    {
        lv_slider_set_value(slider_hue, hue, LV_ANIM_OFF);
    }
    if(slider_sat)
    {
        lv_slider_set_value(slider_sat, sat, LV_ANIM_OFF);
    }
    if(slider_light)
    {
        lv_slider_set_value(slider_light, light, LV_ANIM_OFF);
    }
}
