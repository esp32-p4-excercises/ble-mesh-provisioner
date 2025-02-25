#include "esp_log.h"
#include "esp_check.h"
#include "esp_err.h"

#include "lvgl.h"

#include "mesh.h"
#include "models.h"

void mesh_model_set_publish(uint16_t addr, uint16_t pub_addr, uint16_t model_id, uint8_t ttl, uint8_t period, uint8_t retrans);

static lv_obj_t *address_ta = nullptr;
static lv_obj_t *ttl_slider = nullptr;
static lv_obj_t *period_slider = nullptr;
static lv_obj_t *count_slider = nullptr;
static lv_obj_t *retrans_slider = nullptr;

uint16_t address, pub_addr, model_id;

static lv_obj_t *container = nullptr;

static lv_obj_t *kb = nullptr;

static void lv_example_keyboard_2(void)
{
    /*Create an AZERTY keyboard map*/
    static const char * kb_map[] = {"1","2","3","4","5","\n",
									"6","7","8","9","0","\n",
									"A","B","C","D","E","F","\n",
									LV_SYMBOL_LEFT,LV_SYMBOL_RIGHT,LV_SYMBOL_BACKSPACE, LV_SYMBOL_OK, NULL
									};

    /*Set the relative width of the buttons and other controls*/
    static const lv_btnmatrix_ctrl_t kb_ctrl[] = {4, 4, 4, 4, 4, 
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
	lv_obj_set_size(kb, lv_pct(75), lv_pct(35));
	lv_obj_align(kb, LV_ALIGN_BOTTOM_MID, 0, -70);
	lv_example_keyboard_2();
	// lv_obj_add_flag(kb, LV_OBJ_FLAG_HIDDEN);
	lv_keyboard_set_textarea(kb, address_ta);
}

void element_model_modal_publish(lv_obj_t *parent, uint16_t addr, uint16_t model)
{
	address = addr;
	model_id = model;
	bsp_display_lock(0);
	// address = addr;
	auto main_mbox = lv_msgbox_create(parent, "Publish settings", NULL, NULL, true);
	lv_obj_set_size(main_mbox, LV_PCT(100), LV_SIZE_CONTENT);

	lv_obj_add_event_cb(main_mbox, [](lv_event_t *e)
						{ container = nullptr; }, LV_EVENT_DELETE, NULL);

	container = lv_obj_create(main_mbox);
	lv_obj_set_flex_flow(container, LV_FLEX_FLOW_COLUMN);
	lv_obj_set_size(container, lv_pct(100), 500);

	lv_obj_set_size(main_mbox, 430, LV_SIZE_CONTENT);
	lv_obj_set_style_min_height(main_mbox, 550, 0);
	lv_obj_align(main_mbox, LV_ALIGN_TOP_MID, 0, 10);

	{ // address
		auto cont = lv_msgbox_create(container, "Address", NULL, NULL, false);
		lv_obj_set_size(cont, lv_pct(100), LV_SIZE_CONTENT);
		// lv_obj_set_flex_flow(cont, LV_FLEX_FLOW_COLUMN);
		auto lbl = lv_label_create(cont);
		lv_label_set_text(lbl, "0x");
		lv_obj_align(lbl, LV_ALIGN_LEFT_MID, 0, 30);
		address_ta = lv_textarea_create(cont);
		lv_obj_set_size(address_ta, lv_pct(50), LV_SIZE_CONTENT);
		lv_obj_add_event_cb(address_ta, [](lv_event_t *e){
			lv_event_code_t code = lv_event_get_code(e);
			lv_obj_t *ta = (lv_obj_t *)lv_event_get_target(e);
		
			if (code == LV_EVENT_READY)
			{
				printf("LV_EVENT_READY: %d\n", code);
				lv_event_send(address_ta, LV_EVENT_DEFOCUSED, NULL);
				// lv_keyboard_set_textarea(kb, NULL);
			} else if(code == LV_EVENT_FOCUSED){
				lv_example_keyboard();
			} else if(code == LV_EVENT_DEFOCUSED){
				if(kb)
					lv_obj_del(kb);
				kb = nullptr;
			} 
		}, LV_EVENT_ALL, NULL);
	}

	{ // security credentials - not implemented yet
	  // friendship flag
	  // auto cont = lv_msgbox_create(container, "Security credentials");
	  // lv_obj_set_size(cont, lv_pct(100), LV_SIZE_CONTENT);
	  // lv_obj_set_flex_flow(cont, LV_FLEX_FLOW_COLUMN);
	  // auto lbl = lv_label_create(cont);
	  // lv_label_set_text(lbl, "Not implemented yet");
	}

	{ // publish TTL
		// slider 0-127 - TTL
		auto cont = lv_msgbox_create(container, "Time To Live", NULL, NULL, false);
		lv_obj_set_size(cont, lv_pct(100), LV_SIZE_CONTENT);
		lv_obj_set_flex_flow(cont, LV_FLEX_FLOW_COLUMN);
		auto lbl = lv_label_create(cont);
		lv_label_set_text(lbl, "Publish TTL");
		ttl_slider = lv_slider_create(cont);
		lv_slider_set_range(ttl_slider, 0, 127);
		lv_slider_set_value(ttl_slider, 7, LV_ANIM_OFF);
	}

	{ // publish period
		// slider 0-37800 minutes - interval
		auto cont = lv_msgbox_create(container, "Publish period", NULL, NULL, false);
		lv_obj_set_size(cont, lv_pct(100), LV_SIZE_CONTENT);
		lv_obj_set_flex_flow(cont, LV_FLEX_FLOW_COLUMN);
		auto lbl = lv_label_create(cont);
		lv_label_set_text(lbl, "interval");
		period_slider = lv_slider_create(cont);
	}

	{ // publish retransmission
		// slider  0-7 - count
		auto cont = lv_msgbox_create(container, "Publish retransmission", NULL, NULL, false);
		lv_obj_set_size(cont, lv_pct(100), LV_SIZE_CONTENT);
		lv_obj_set_flex_flow(cont, LV_FLEX_FLOW_COLUMN);
		auto lbl = lv_label_create(cont);
		lv_label_set_text(lbl, "count");
		count_slider = lv_slider_create(cont);

		// slider 50-1600ms - interval
		lbl = lv_label_create(cont);
		lv_label_set_text(lbl, "interval");
		retrans_slider = lv_slider_create(cont);
	}

	{
		auto btn = lv_btn_create(main_mbox);
		auto lbl = lv_label_create(btn);
		lv_label_set_text(lbl, LV_SYMBOL_OK);
		lv_obj_center(lbl);
		lv_obj_align(btn, LV_ALIGN_BOTTOM_RIGHT, lv_pct(80), 0);
		lv_obj_set_size(btn, 60, 60);
		lv_obj_set_style_radius(btn, LV_RADIUS_CIRCLE, 0);

		lv_obj_add_event_cb(btn, [](lv_event_t *ev){
			lv_obj_t *main_mbox = (lv_obj_t*)lv_event_get_user_data(ev);
			auto pub_addr = strtol(lv_textarea_get_text(address_ta), NULL, 16);
			uint8_t ttl = lv_slider_get_value(ttl_slider);
			uint8_t period = lv_slider_get_value(period_slider);
			uint8_t count = lv_slider_get_value(count_slider);
			uint8_t retrans = lv_slider_get_value(retrans_slider);
			mesh_model_set_publish(address, pub_addr, model_id, ttl, period, retrans);
			// lv_obj_del(main_mbox);
		}, LV_EVENT_SHORT_CLICKED, main_mbox);
	}

	bsp_display_unlock();
}
