
#include <string>
#include "lvgl.h"

#include "json.hpp"
#include "ble_mesh_provisioner.h"

#include "mesh.h"
#include "models.h"

static void lvgl_nodes_select_node(uint16_t addr);
const char *mesh_model_get_type(uint16_t id);
void model_modal_mbox_open(uint16_t addr, uint16_t model);
void mesh_node_reset_node(uint16_t addr);
void mesh_node_get_comp_data(uint16_t addr);

static lv_obj_t *screen = NULL;
static lv_obj_t *left_pane = NULL;
static lv_obj_t *right_pane = NULL;
static lv_style_t style;

static void select_event_cb(lv_event_t *e)
{
	lv_obj_t *t = (lv_obj_t *)lv_event_get_target(e);
	auto lbl = lv_obj_get_child(t, 0);

	auto addr = lv_label_get_text(lbl);
	lvgl_nodes_select_node(strtol(addr, NULL, 16));
}

static void lvgl_nodes_select_node(uint16_t addr)
{
	lv_obj_clean(right_pane);
	esp_ble_mesh_node_t *node = esp_ble_mesh_provisioner_get_node_with_addr(addr);
	auto comp = *mesh_get_composition(addr);
	if (comp.element_num == 0)
	{
		mesh_node_get_comp_data(addr);
		// return;
	}

	auto label = lv_label_create(right_pane);
	lv_label_set_text_fmt(label, "Address: 0x%04X", addr);
	for (size_t i = 0; i < comp.element_num; i++)
	{
		auto btn = lv_btn_create(right_pane);

		label = lv_label_create(btn);
		lv_label_set_text_fmt(label, "Element: 0x%04X", addr + i);

		auto elem = comp.elements[i];
		auto cont0 = lv_obj_create(right_pane);
		lv_obj_set_size(cont0, LV_SIZE_CONTENT, LV_PCT(100));
		lv_obj_set_flex_flow(cont0, LV_FLEX_FLOW_COLUMN);
		for (size_t j = 0; j < elem.count; j++)
		{
			auto cont = lv_obj_create(cont0);
			lv_obj_set_size(cont, LV_SIZE_CONTENT, LV_SIZE_CONTENT);
			lv_obj_set_flex_flow(cont, LV_FLEX_FLOW_ROW);

			lv_obj_t *bind_btn = nullptr;
			auto lbl = lv_label_create(cont);
			if (elem.models[j].vnd_id == 0xffff)
			{
				lv_label_set_text_fmt(lbl, "%s\nSIG Model ID: 0x%04X", mesh_model_get_type(elem.models[j].mod_id), elem.models[j].mod_id);
			}
			else
			{
				lv_label_set_text_fmt(lbl, "Vendor model\nVendor Model ID: 0x%04X%04X", elem.models[j].vnd_id, elem.models[j].mod_id);
			}

			// bind_btn = lv_button_create(cont);
			auto click = [](lv_event_t *ev)
			{
				auto val = (uint32_t)lv_event_get_user_data(ev);
				uint16_t model = val & 0xffff;
				uint16_t addr = val >> 16;
				printf("\t\t0x%04X, 0x%04X\n", addr, model);
				model_modal_mbox_open(addr, model);
			};
			uint32_t val = ((addr + i) << 16) + elem.models[j].mod_id;
			lv_obj_add_event_cb(cont, click, LV_EVENT_SHORT_CLICKED, (void *)val);
		}

		lv_obj_set_size(cont0, LV_SIZE_CONTENT, 0);
		auto click = [](lv_event_t *ev)
		{
			lv_event_code_t code = lv_event_get_code(ev);
			auto cont = (lv_obj_t *)lv_event_get_user_data(ev);
			if (lv_obj_has_flag(cont, LV_OBJ_FLAG_USER_1))
			{
				lv_obj_set_size(cont, LV_SIZE_CONTENT, 0);
				lv_obj_clear_flag(cont, LV_OBJ_FLAG_USER_1);
			}
			else
			{
				lv_obj_set_size(cont, LV_SIZE_CONTENT, LV_SIZE_CONTENT);
				lv_obj_add_flag(cont, LV_OBJ_FLAG_USER_1);
			}
		};
		lv_obj_add_event_cb(btn, click, LV_EVENT_SHORT_CLICKED, cont0);
	}

	auto btn = lv_btn_create(right_pane);
	label = lv_label_create(btn);
	lv_label_set_text(label, "Reset node");
	auto reset = [](lv_event_t *ev)
	{
		auto addr = (uint32_t)lv_event_get_user_data(ev);
		mesh_node_reset_node(addr);
	};
	lv_obj_add_event_cb(btn, reset, LV_EVENT_SHORT_CLICKED, (void *)addr);
}

void refresh_all_nodes()
{
	lv_obj_clean(left_pane);
	auto count = BLEmeshProvisioner::GetInstance()->nodesCount();
	auto nodes = BLEmeshProvisioner::GetInstance()->getNodes();

	auto cont = lv_obj_create(left_pane);
	lv_obj_set_size(cont, LV_PCT(100), LV_SIZE_CONTENT);
	lv_obj_set_flex_flow(cont, LV_FLEX_FLOW_ROW);

	auto lbl = lv_label_create(cont);
	lv_label_set_text(lbl, "Address");
	lv_obj_add_style(lbl, &style, 0);
	lbl = lv_label_create(cont);
	lv_label_set_text(lbl, "Elements");
	lv_obj_add_style(lbl, &style, 0);
	lbl = lv_label_create(cont);
	lv_label_set_text(lbl, "Name");
	lv_obj_add_style(lbl, &style, 0);

	auto n = 1;
	for (size_t i = 0; i < CONFIG_BLE_MESH_MAX_PROV_NODES; i++)
	{
		auto node = nodes[i];
		if (node)
		{
			auto cont = lv_obj_create(left_pane);
			lv_obj_set_size(cont, LV_PCT(100), LV_SIZE_CONTENT);
			lv_obj_set_flex_flow(cont, LV_FLEX_FLOW_ROW);

			auto lbl = lv_label_create(cont);
			lv_obj_add_style(lbl, &style, 0);
			lv_label_set_text_fmt(lbl, "0x%04X", node->unicast_addr);
			lbl = lv_label_create(cont);
			lv_obj_add_style(lbl, &style, 0);
			lv_label_set_text_fmt(lbl, "%d", node->element_num);
			lbl = lv_label_create(cont);
			lv_obj_add_style(lbl, &style, 0);
			lv_label_set_text_fmt(lbl, "%s", strlen(node->name) ? node->name : "node");
			lv_obj_add_event_cb(cont, select_event_cb, LV_EVENT_SHORT_CLICKED, NULL);
			if (++n > count)
				return;
		}
	}
}

bool lvgl_screen2()
{
	lv_style_init(&style);
	lv_style_set_pad_left(&style, 5);
	lv_style_set_pad_right(&style, 30);
	// lv_style_set_pad_top(&style, 10);
	lv_style_set_pad_bottom(&style, 0);

	screen = lv_obj_create(lv_scr_act());

	left_pane = lv_table_create(screen);
	right_pane = lv_obj_create(screen);
	lv_obj_set_size(screen, LV_PCT(100), LV_PCT(100));
	lv_obj_set_flex_flow(right_pane, LV_FLEX_FLOW_COLUMN);

	lv_obj_set_pos(right_pane, LV_PCT(60), 0);
	lv_obj_set_size(left_pane, LV_PCT(60), LV_PCT(100));
	lv_obj_set_size(right_pane, LV_PCT(40), LV_PCT(100));

	lv_obj_set_flex_flow(left_pane, LV_FLEX_FLOW_COLUMN);

	refresh_all_nodes();
	return true;
}
