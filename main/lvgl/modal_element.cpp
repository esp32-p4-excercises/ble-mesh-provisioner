#include "esp_log.h"
#include "lvgl.h"

#include "mesh.h"
#include "models.h"

const char* mesh_model_get_type(uint16_t id);
ble_mesh_comp_t *mesh_get_composition(uint16_t addr);
void mesh_model_sub_add(uint16_t addr, uint16_t sub, uint16_t model_id);
void mesh_model_sub_del(uint16_t addr, uint16_t sub, uint16_t model_id);
void mesh_model_sub_get(uint16_t addr, uint16_t model_id);
void mesh_model_app_key_bind(uint16_t addr, uint16_t model_id, uint8_t appIdx);
void mesh_model_app_key_unbind(uint16_t addr, uint16_t model_id, uint8_t appIdx);

static uint16_t address;
static uint16_t model_id;
static lv_obj_t* textarea = nullptr;
static lv_obj_t* sub_cont = nullptr;

static void event_cb(lv_event_t *e)
{
    lv_obj_t *btn = (lv_obj_t *)lv_event_get_target(e);
    lv_obj_t *label = lv_obj_get_child(btn, 0);
    LV_UNUSED(label);
    printf("Button %s clicked\n", lv_label_get_text(label));
}

static void element_bind_cb(lv_event_t *e)
{
    auto btn = (lv_obj_t*)lv_event_get_target(e);
    uint8_t appIdx = (uint32_t)lv_event_get_user_data(e);
    mesh_model_app_key_bind(address, model_id, appIdx);
}

static void element_unbind_cb(lv_event_t *e)
{
    auto btn = (lv_obj_t*)lv_event_get_target(e);
    uint8_t appIdx = (uint32_t)lv_event_get_user_data(e);
    mesh_model_app_key_unbind(address, model_id, appIdx);
}

static void add_bind_menu(lv_obj_t* parent)
{
    auto mbox = lv_msgbox_create(parent);
    lv_msgbox_add_title(mbox, "Bound app keys");
    lv_obj_set_width(mbox, LV_PCT(100));
    auto _cont = lv_obj_create(mbox);
    lv_obj_set_flex_flow(_cont, LV_FLEX_FLOW_ROW);
    lv_obj_set_size(_cont, LV_PCT(100), LV_SIZE_CONTENT);

    auto label = lv_label_create(_cont);
    lv_label_set_text(label, "App key 0");
    lv_obj_align(label, LV_ALIGN_LEFT_MID, 0, 10);

    auto btn = lv_button_create(_cont);
    label = lv_label_create(btn);
    lv_label_set_text(label, "Bind");
    lv_obj_align_to(btn, NULL, LV_ALIGN_RIGHT_MID, 0, 0);
    lv_obj_add_event(btn, element_bind_cb, LV_EVENT_SHORT_CLICKED, 0);

    btn = lv_button_create(_cont);
    label = lv_label_create(btn);
    lv_label_set_text(label, "Unbind");
    lv_obj_align_to(btn, NULL, LV_ALIGN_RIGHT_MID, 0, 0);
    lv_obj_add_event(btn, element_unbind_cb, LV_EVENT_SHORT_CLICKED, 0);
}

static void add_publish_menu(lv_obj_t* parent)
{
    auto mbox = lv_msgbox_create(parent);
    lv_msgbox_add_title(mbox, "Publish");
    auto box_c = lv_msgbox_get_content(mbox);
    lv_obj_set_width(mbox, LV_PCT(100));
    auto label = lv_label_create(box_c);
    lv_label_set_text(label, "Not implemented yet");
}

static void add_subscribe_menu(lv_obj_t* parent)
{
    sub_cont = lv_msgbox_create(parent);
    lv_msgbox_add_title(sub_cont, "Subscribe");
    lv_obj_set_width(sub_cont, LV_PCT(100));
    auto box_c = lv_msgbox_get_content(sub_cont);
    auto box = lv_obj_create(box_c);
    lv_obj_set_flex_flow(box, LV_FLEX_FLOW_COLUMN);
    lv_obj_set_size(box, LV_PCT(100), LV_SIZE_CONTENT);

    for (size_t a = 0; a < 3; a++)
    {
        auto _cont = lv_obj_create(box);
        lv_obj_set_flex_flow(_cont, LV_FLEX_FLOW_ROW);
        lv_obj_set_size(_cont, LV_PCT(100), LV_SIZE_CONTENT);

        auto lbl = lv_label_create(_cont);
        lv_label_set_text_fmt(lbl, "0x%04X", 0xc000 + a);

        auto btn = lv_button_create(_cont);
        auto label = lv_label_create(btn);
        lv_label_set_text(label, "Add");
        lv_obj_align_to(btn, _cont, LV_ALIGN_RIGHT_MID, 0, 0);

        auto click = [](lv_event_t* ev)
        {
            uint16_t addr = (uint32_t)lv_event_get_user_data(ev);
            mesh_model_sub_add(address, addr, model_id);
        };
        lv_obj_add_event(btn, click, LV_EVENT_SHORT_CLICKED, (void*)0xc000 + a);
    }

    auto comp = *mesh_get_composition(address);
    auto el = comp.elements[address - comp.node_addr];

    for (size_t j = 0; j < el.count; j++)
    {
        for (size_t k = 0; k < 3; k++)
        {
            auto sub_adr = el.models[j].subs[k];
            if (sub_adr and el.models[j].mod_id == model_id)
            {
                auto click1 = [](lv_event_t* ev)
                {
                    auto addr = (uint32_t)lv_event_get_user_data(ev);
                    mesh_model_sub_del(address, addr, model_id);
                };
                auto _cont = lv_obj_create(box_c);
                lv_obj_set_flex_flow(_cont, LV_FLEX_FLOW_ROW);
                lv_obj_set_size(_cont, LV_PCT(100), LV_SIZE_CONTENT);
                auto label = lv_label_create(_cont);
                lv_label_set_text_fmt(label, "0x%04x ", sub_adr);
                auto btn = lv_button_create(_cont);
                label = lv_label_create(btn);
                lv_label_set_text(label, "Del");
                lv_obj_add_event(btn, click1, LV_EVENT_SHORT_CLICKED, (void*)sub_adr);
                printf("\tSUB: 0x%04x\n", sub_adr);
            }                
        }
    }
}

static void add_model_action(lv_obj_t* parent)
{
    switch (model_id)
    {
    case 0x1000:{
        model_1000_modal_action(parent, address, false);
        break;
    }
    case 0x1002:{
        model_1002_modal_action(parent, address, false);
        break;
    }
    case 0x1307:{
        model_1307_modal_action(parent, address, false);
        break;
    }    
    default:
        ESP_LOGW("", "Model action not implemented");
        break;
    }
}

static lv_obj_t* main_box_cont = nullptr;
void model_modal_mbox_open(uint16_t addr, uint16_t model)
{
    bsp_display_lock(0);
    address = addr;
    model_id = model;
    auto main_mbox = lv_msgbox_create(NULL);
    auto par = lv_obj_get_parent(main_mbox);

    char *title = (char*)calloc(1, 100);
    sprintf(title, "%s\nModelID: 0x%04X", mesh_model_get_type(model_id), model_id);

    lv_msgbox_add_title(main_mbox, title);
    lv_msgbox_add_close_button(main_mbox);

    main_box_cont = lv_msgbox_get_content(main_mbox);

    if(model == 0)
    {

    } else {
        add_bind_menu(main_box_cont);
        add_publish_menu(main_box_cont);
        add_subscribe_menu(main_box_cont);
        add_model_action(main_box_cont);
    }

    lv_obj_set_size(par, LV_PCT(100), LV_PCT(100));
    lv_obj_set_size(main_mbox, 430, LV_SIZE_CONTENT);
    // lv_obj_set_pos(par, LV_PCT(50), 10);
    lv_obj_align(main_mbox, LV_ALIGN_TOP_MID, 0, 10);
 
    free(title);

    lv_obj_add_event(par, [](lv_event_t* e){
        auto obj = lv_event_get_target_obj(e);
        lv_obj_delete(obj);
    }, LV_EVENT_PRESSED, NULL);

    bsp_display_unlock();
}

void lvgl_modal_update_subs()
{
    bsp_display_lock(0);
    lv_obj_delete(sub_cont);
    add_subscribe_menu(main_box_cont);
    bsp_display_unlock();

}
