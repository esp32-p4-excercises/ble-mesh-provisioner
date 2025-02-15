#include "lvgl.h"

#include "mesh.h"

void mesh_model_get_hsl(uint16_t address);
void mesh_model_set_hsl(uint16_t addr, uint16_t hue, uint16_t sat, uint16_t light);

static uint16_t address = 0;
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
	auto mbox = lv_msgbox_create(parent, "HSL light controls", NULL, NULL, false);
	lv_obj_set_width(mbox, LV_PCT(100));
	lv_obj_align(mbox, LV_ALIGN_TOP_MID, 0, 0);

	auto cont_row = lv_msgbox_get_content(mbox);

	static lv_style_t style;
	lv_style_init(&style);
	lv_style_set_pad_bottom(&style, 8);

	lv_obj_set_size(cont_row, LV_PCT(100), LV_SIZE_CONTENT);
	lv_obj_set_flex_flow(cont_row, LV_FLEX_FLOW_COLUMN);
	lv_obj_set_pos(cont_row, 0, 0);
	lv_obj_add_event_cb(cont_row, delete_sliders, LV_EVENT_DELETE, NULL);

	auto lbl = lv_label_create(cont_row);
    lv_obj_add_style(lbl, &style, 0);
	lv_label_set_text(lbl, "Hue");
	slider_hue = lv_slider_create(cont_row);
	lv_slider_set_range(slider_hue, 0, 65535);
	lv_obj_center(slider_hue);

	lbl = lv_label_create(cont_row);
    lv_obj_add_style(lbl, &style, 0);
	lv_label_set_text(lbl, "Saturation");
	slider_sat = lv_slider_create(cont_row);
	lv_slider_set_range(slider_sat, 0, 65535);
	lv_obj_center(slider_sat);

	lbl = lv_label_create(cont_row);
    lv_obj_add_style(lbl, &style, 0);
	lv_label_set_text(lbl, "Lightness");
	slider_light = lv_slider_create(cont_row);
	lv_slider_set_range(slider_light, 0, 65535);
	lv_obj_center(slider_light);
    lv_obj_add_style(slider_hue, &style, 0);
    lv_obj_add_style(slider_sat, &style, 0);
    lv_obj_add_style(slider_light, &style, 0);

	auto btn = lv_btn_create(cont_row);
	lbl = lv_label_create(btn);
	lv_label_set_text(lbl, "Set");
	lv_obj_add_event_cb(btn, [](lv_event_t *ev){
		// uint16_t hue_val, saturation, lightness;
		auto hue_val = lv_slider_get_value(slider_hue);
		auto saturation = lv_slider_get_value(slider_sat);
		auto lightness = lv_slider_get_value(slider_light);
		mesh_model_set_hsl(address, hue_val, saturation, lightness);
	}, LV_EVENT_SHORT_CLICKED, NULL);

	mesh_model_get_hsl(address);
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
