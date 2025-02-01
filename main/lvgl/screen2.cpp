
#include <string>
#include "lvgl.h"

#include "json.hpp"
#include "ble_mesh_provisioner.h"

#include "mesh.h"
#include "models.h"

std::string provisioner_request_nodes_cb();
static void lvgl_nodes_select_node(int addr);
const char* mesh_model_get_type(uint16_t id);
void model_modal_mbox_open(uint16_t addr, uint16_t model);
void mesh_node_reset_node(uint16_t addr);
void mesh_node_get_comp_data(uint16_t addr);

static lv_obj_t *screen = NULL;
static lv_obj_t *left_pane = NULL;
static lv_obj_t *right_pane = NULL;

static void screen_evt_cb(lv_event_t *ev)
{
    lv_event_code_t code = lv_event_get_code(ev);
    auto scr = (lv_obj_t *)lv_event_get_target(ev);
    if (code == LV_EVENT_SCREEN_LOADED)
    {
        // printf("screen 2 loaded\n");
    }
    else if (code == LV_EVENT_SCREEN_UNLOADED)
    {
        // printf("screen 2 unloaded\n");
        lv_obj_delete(scr);
    }
}

static void select_event_cb(lv_event_t *e)
{
    lv_obj_t *obj = (lv_obj_t *)lv_event_get_target(e);
    uint32_t col;
    uint32_t row;
    lv_table_get_selected_cell(obj, &row, &col);
    if(row == 0) return;
    auto addr = lv_table_get_cell_value(obj, row, 0);
    assert(addr);
    lvgl_nodes_select_node(atoi(addr));
}

static void lvgl_nodes_select_node(int addr)
{
    lv_obj_clean(right_pane);
    esp_ble_mesh_node_t * node = esp_ble_mesh_provisioner_get_node_with_addr(addr);
    auto comp = *mesh_get_composition(addr);
    if (comp.element_num == 0)
    {
        mesh_node_get_comp_data(addr);
        return;
    }

    /*Create a container with ROW flex direction*/
    lv_obj_t * cont_row = lv_obj_create(right_pane);
    lv_obj_set_size(cont_row, 470, LV_SIZE_CONTENT);
    // lv_obj_align(cont_row, LV_ALIGN_TOP_MID, 0, 5);
    lv_obj_set_flex_flow(cont_row, LV_FLEX_FLOW_COLUMN);

    auto label = lv_label_create(cont_row);
    lv_label_set_text_fmt(label, "Address: %d", addr);
    int offsY = 60;
    for (size_t i = 0; i < comp.element_num; i++)
    {
        auto btn = lv_button_create(cont_row);

        label = lv_label_create(btn);
        lv_label_set_text_fmt(label, "Element: 0x%04X", addr + i);

        auto elem = comp.elements[i];
        auto cont0 = lv_obj_create(cont_row);
        lv_obj_set_size(cont0, LV_SIZE_CONTENT, LV_SIZE_CONTENT);
        lv_obj_set_flex_flow(cont0, LV_FLEX_FLOW_COLUMN);
        for (size_t j = 0; j < elem.count; j++)
        {
            auto cont = lv_obj_create(cont0);
            lv_obj_set_size(cont, LV_SIZE_CONTENT, LV_SIZE_CONTENT);
            lv_obj_set_flex_flow(cont, LV_FLEX_FLOW_ROW);

            lv_obj_t* bind_btn = nullptr;
            auto lbl = lv_label_create(cont);
            if(elem.models[j].vnd_id == 0xffff)
            {
                lv_label_set_text_fmt(lbl, "%s\nSIG Model ID: 0x%04X", mesh_model_get_type(elem.models[j].mod_id), elem.models[j].mod_id);
            }
            else {
                lv_label_set_text_fmt(lbl, "Vendor model\nVendor Model ID: 0x%04X%04X", elem.models[j].vnd_id, elem.models[j].mod_id);
            }

            // bind_btn = lv_button_create(cont);
            auto click = [](lv_event_t* ev) {
                auto val = (uint32_t)lv_event_get_user_data(ev);
                uint16_t model = val & 0xffff;
                uint16_t addr = val >> 16;
                printf("\t\t0x%04X, 0x%04X\n", addr, model);
                model_modal_mbox_open(addr, model);
            };
            // if (bind_btn)
            // {
            //     label = lv_label_create(bind_btn);
            //     lv_label_set_text(label, "Bind");
                uint32_t val = ((addr + i) << 16) + elem.models[j].mod_id;
            //     printf("\t\t0x%04X, 0x%04X, 0x%08lx\n", addr, elem.models[j].mod_id, val);
                lv_obj_add_event(cont, click, LV_EVENT_SHORT_CLICKED, (void*)val);
            // }
        }
        

        lv_obj_set_size(cont0, LV_SIZE_CONTENT, 0);
        auto click = [](lv_event_t* ev) {
            auto cont = (lv_obj_t *)lv_event_get_user_data(ev);
            if(lv_obj_has_flag(cont, LV_OBJ_FLAG_USER_1))
            {
                lv_obj_set_size(cont, LV_SIZE_CONTENT, 0);
                lv_obj_remove_flag(cont, LV_OBJ_FLAG_USER_1);
            } else {
                lv_obj_set_size(cont, LV_SIZE_CONTENT, LV_SIZE_CONTENT);
                lv_obj_add_flag(cont, LV_OBJ_FLAG_USER_1);
            }
        };
        lv_obj_add_event(btn, click, LV_EVENT_SHORT_CLICKED, cont0);
    }

    auto btn = lv_button_create(cont_row);
    label = lv_label_create(btn);
    lv_label_set_text(label, "Reset node");
    auto reset = [](lv_event_t* ev)
    {
        auto addr = (uint32_t)lv_event_get_user_data(ev);
        mesh_node_reset_node(addr);
    };
    lv_obj_add_event(btn, reset, LV_EVENT_SHORT_CLICKED, (void*)addr);
}

void lvgl_screen2()
{
    screen = lv_obj_create(NULL);
    lv_obj_add_event(screen, screen_evt_cb, LV_EVENT_ALL, NULL);
    lv_screen_load(screen);

    left_pane = lv_obj_create(screen);
    right_pane = lv_obj_create(screen);

    lv_obj_set_pos(left_pane, 0, 100);
    lv_obj_set_pos(right_pane, LV_PCT(50), 100);
    lv_obj_set_size(left_pane, LV_PCT(50), LV_PCT(100));
    lv_obj_set_size(right_pane, LV_PCT(50), LV_PCT(100));

    auto count = BLEmeshProvisioner::GetInstance()->nodesCount();
    auto nodes = BLEmeshProvisioner::GetInstance()->getNodes();

    lv_obj_t *table = lv_table_create(left_pane);

    /*Set a smaller height to the table. It'll make it scrollable*/
    lv_obj_set_size(table, LV_PCT(100), LV_SIZE_CONTENT);

    lv_table_set_column_width(table, 0, 120);
    lv_table_set_column_width(table, 1, 120);
    lv_table_set_column_width(table, 2, LV_SIZE_CONTENT);
    // lv_table_set_row_count(table, count); /*Not required but avoids a lot of memory reallocation lv_table_set_set_value*/
    lv_table_set_column_count(table, 3);
    lv_obj_remove_style(table, NULL, LV_PART_ITEMS | LV_STATE_PRESSED);
    lv_table_set_cell_value(table, 0, 0, "Addr");
    lv_table_set_cell_value(table, 0, 1, "Elem");
    lv_table_set_cell_value(table, 0, 2, "Name");

    auto n = 1;
    for (size_t i = 0; i < CONFIG_BLE_MESH_MAX_PROV_NODES; i++)
    {
        auto node = nodes[i];
        if(node)
        {
            lv_table_set_cell_value_fmt(table, n, 0, "%d", node->unicast_addr);
            lv_table_set_cell_value_fmt(table, n, 1, "%d", node->element_num);
            lv_table_set_cell_value_fmt(table, n, 2, "%s", strlen(node->name) ? node->name : "node");
            n++;
            if(n > count) break;
        }
    }

    lv_obj_add_event(table, select_event_cb, LV_EVENT_PRESSED, NULL);
}
