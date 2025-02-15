#include "esp_log.h"
#include "lvgl.h"

#include "mesh.h"
#include "models.h"

const char *mesh_model_get_type(uint16_t id);
ble_mesh_comp_t *mesh_get_composition(uint16_t addr);
void mesh_model_sub_add(uint16_t addr, uint16_t sub, uint16_t model_id);
void mesh_model_sub_del(uint16_t addr, uint16_t sub, uint16_t model_id);
void mesh_model_sub_get(uint16_t addr, uint16_t model_id);
void mesh_model_app_key_bind(uint16_t addr, uint16_t model_id, uint8_t appIdx);
void mesh_model_app_key_unbind(uint16_t addr, uint16_t model_id, uint8_t appIdx);

static uint16_t address;
static uint16_t model_id;
static lv_obj_t *sub_cont = nullptr;

static void element_bind_cb(lv_event_t *e)
{
	auto btn = (lv_obj_t *)lv_event_get_target(e);
	uint8_t appIdx = (uint32_t)lv_event_get_user_data(e);
	mesh_model_app_key_bind(address, model_id, appIdx);
}

static void element_unbind_cb(lv_event_t *e)
{
	auto btn = (lv_obj_t *)lv_event_get_target(e);
	uint8_t appIdx = (uint32_t)lv_event_get_user_data(e);
	mesh_model_app_key_unbind(address, model_id, appIdx);
}

static void add_bind_menu(lv_obj_t *parent)
{
	static lv_style_t style;
	lv_style_init(&style);
	lv_style_set_pad_left(&style, 5);
	lv_style_set_pad_right(&style, 15);
	lv_style_set_pad_top(&style, 10);
	lv_style_set_pad_bottom(&style, 0);

	auto mbox = lv_msgbox_create(parent, "Bound app keys", NULL, NULL, false);
	lv_obj_set_width(mbox, LV_PCT(100));
	auto _cont = lv_msgbox_get_content(mbox);
	lv_obj_set_flex_flow(_cont, LV_FLEX_FLOW_ROW);
	lv_obj_set_size(_cont, LV_PCT(100), LV_SIZE_CONTENT);

	auto label = lv_label_create(_cont);
	lv_obj_add_style(label, &style, 0);
	lv_label_set_text(label, "App key 0");

	auto btn = lv_btn_create(_cont);
	label = lv_label_create(btn);
	lv_label_set_text(label, "Bind");
	lv_obj_align_to(btn, NULL, LV_ALIGN_RIGHT_MID, 10, 0);
	lv_obj_add_event_cb(btn, element_bind_cb, LV_EVENT_SHORT_CLICKED, 0);

	btn = lv_btn_create(_cont);
	label = lv_label_create(btn);
	lv_label_set_text(label, "Unbind");
	lv_obj_align_to(btn, NULL, LV_ALIGN_RIGHT_MID, 10, 0);
	lv_obj_add_event_cb(btn, element_unbind_cb, LV_EVENT_SHORT_CLICKED, 0);
}

static void add_publish_menu(lv_obj_t *parent)
{
	auto mbox = lv_msgbox_create(parent, "Publish", NULL, NULL, false);
	lv_obj_set_width(mbox, LV_PCT(100));
	auto box_c = lv_msgbox_get_content(mbox);
	auto label = lv_label_create(box_c);
	lv_label_set_text(label, "Not implemented yet");
}

static void add_subscribe_menu(lv_obj_t *parent)
{
	sub_cont = lv_msgbox_create(parent, "Subscribe", NULL, NULL, false);
	lv_obj_set_width(sub_cont, LV_PCT(100));
	auto box_c = lv_msgbox_get_content(sub_cont);
	lv_obj_set_flex_flow(box_c, LV_FLEX_FLOW_COLUMN);

	auto add_btn = [](lv_obj_t *btn, uint16_t sub)
	{
		auto label = lv_label_create(btn);
		lv_label_set_text(label, "Add");

		auto click = [](lv_event_t *ev)
		{
			uint16_t addr = (uint32_t)lv_event_get_user_data(ev);
			mesh_model_sub_add(address, addr, model_id);
		};
		lv_obj_add_event_cb(btn, click, LV_EVENT_SHORT_CLICKED, (void *)sub);
	};

	auto del_btn = [](lv_obj_t *btn, uint16_t sub)
	{
		auto click1 = [](lv_event_t *ev)
		{
			auto addr = (uint32_t)lv_event_get_user_data(ev);
			mesh_model_sub_del(address, addr, model_id);
		};
		auto label = lv_label_create(btn);
		lv_label_set_text(label, "Del");
		lv_obj_add_event_cb(btn, click1, LV_EVENT_SHORT_CLICKED, (void *)sub);
	};

	auto comp = *mesh_get_composition(address);
	auto el = comp.elements[address - comp.node_addr];

	for (size_t j = 0; j < el.count; j++)
	{
		if (el.models[j].mod_id == model_id)
		{
			for (size_t k = 0; k < 3; k++)
			{
				auto sub = 0xc000 + k;
				auto _cont = lv_obj_create(box_c);
				lv_obj_set_flex_flow(_cont, LV_FLEX_FLOW_ROW);
				lv_obj_set_size(_cont, LV_PCT(100), LV_SIZE_CONTENT);
				auto label = lv_label_create(_cont);
				lv_label_set_text_fmt(label, "0x%04x ", sub);
				auto btn = lv_btn_create(_cont);

				if (el.models[j].subs[0] == sub or el.models[j].subs[1] == sub or el.models[j].subs[2] == sub)
				{
					del_btn(btn, sub);
				}
				else
				{
					add_btn(btn, sub);
				}
			}
		}
	}
}

static void add_model_action(lv_obj_t *parent)
{
	switch (model_id)
	{
	case 0x1000:
	{
		model_1000_modal_action(parent, address, false);
		break;
	}
	case 0x1002:
	{
		model_1002_modal_action(parent, address, false);
		break;
	}
	case 0x1307:
	{
		model_1307_modal_action(parent, address, false);
		break;
	}
	default:
		ESP_LOGW("", "Model action not implemented");
		break;
	}
}

static lv_obj_t *container = nullptr;
void model_modal_mbox_open(uint16_t addr, uint16_t model)
{
	bsp_display_lock(0);
	address = addr;
	model_id = model;
	auto main_mbox = lv_msgbox_create(NULL, "Model actions", NULL, NULL, true);
	lv_obj_set_size(main_mbox, LV_PCT(100), LV_SIZE_CONTENT);

	lv_obj_add_event_cb(main_mbox, [](lv_event_t *e)
						{ container = nullptr; }, LV_EVENT_DELETE, NULL);

	char *title = (char *)calloc(1, 100);
	sprintf(title, "%s\nModelID: 0x%04X", mesh_model_get_type(model_id), model_id);

	container = lv_obj_create(main_mbox);
	lv_obj_set_flex_flow(container, LV_FLEX_FLOW_COLUMN);
	lv_obj_set_size(container, lv_pct(100), 500);

	if (model == 0)
	{
	}
	else
	{
		add_bind_menu(container);
		add_model_action(container);
		add_publish_menu(container);
		add_subscribe_menu(container);
	}

	lv_obj_set_size(main_mbox, 430, LV_SIZE_CONTENT);
	lv_obj_set_style_min_height(main_mbox, 550, 0);
	lv_obj_align(main_mbox, LV_ALIGN_TOP_MID, 0, 10);

	free(title);

	bsp_display_unlock();
}

void lvgl_modal_update_subs()
{
	if (container)
	{
		bsp_display_lock(0);
		lv_obj_del(sub_cont);
		add_subscribe_menu(container);
		bsp_display_unlock();
	}
}
