
#include "lvgl.h"

#include "mesh.h"

void mesh_model_get_level(uint16_t address);
void mesh_model_set_level(uint16_t addr, int lvl);

static uint16_t address = 0;
static lv_obj_t *slider = nullptr;

static void slider_event_cb(lv_event_t *e)
{
	lv_event_code_t code = lv_event_get_code(e);
	if (code == LV_EVENT_DELETE)
	{
		slider = nullptr;
	}
	else
	{
		lv_obj_t *slider = (lv_obj_t *)lv_event_get_target(e);
		auto val = (int)lv_slider_get_value(slider);
		mesh_model_set_level(address, val);
	}
}

void model_1002_modal_action(lv_obj_t *parent, uint16_t addr, bool clean)
{
	address = addr;
	if(clean)
		lv_obj_clean(parent);
	auto mbox = lv_msgbox_create(parent, "Generic level controls", NULL, NULL, false);
	lv_obj_set_width(mbox, LV_PCT(100));
	lv_obj_align(mbox, LV_ALIGN_TOP_MID, 0, 0);

	auto cont = lv_msgbox_get_content(mbox);
	slider = lv_slider_create(cont);
	lv_slider_set_range(slider, -32768, 32767);
	lv_obj_center(slider);
	lv_obj_add_event_cb(slider, slider_event_cb, LV_EVENT_VALUE_CHANGED, NULL);
	lv_obj_add_event_cb(slider, slider_event_cb, LV_EVENT_DELETE, NULL);

	mesh_model_get_level(address);
}

void lvgl_slider_update_level(uint16_t addr, int lvl)
{
	if (addr != address or !slider)
		return;
	lv_slider_set_value(slider, lvl, LV_ANIM_OFF);
}
