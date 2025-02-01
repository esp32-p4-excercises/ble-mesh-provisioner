#include "string"

#include "lvgl.h"
#include "_lvgl.h"


static void button1_evt_cb(lv_event_t *ev)
{
    lv_event_code_t code = lv_event_get_code(ev);
    if (code == LV_EVENT_SHORT_CLICKED)
    {
        lvgl_screen1();
    }
}

static void button2_evt_cb(lv_event_t *ev)
{
    lv_event_code_t code = lv_event_get_code(ev);
    if (code == LV_EVENT_SHORT_CLICKED)
    {
        lvgl_screen2();
    }
}

static void button3_evt_cb(lv_event_t *ev)
{
    lv_event_code_t code = lv_event_get_code(ev);
    if (code == LV_EVENT_SHORT_CLICKED)
    {
        lvgl_screen3();
    }
}

static void button4_evt_cb(lv_event_t *ev)
{
    lv_event_code_t code = lv_event_get_code(ev);
    if (code == LV_EVENT_SHORT_CLICKED)
    {
        lvgl_screen4();
    }
}


static void button_add_style(lv_obj_t* btn)
{
    static lv_style_t style;
    lv_style_init(&style);

    lv_style_set_radius(&style, 3);

    lv_style_set_bg_opa(&style, LV_OPA_100);
    lv_style_set_bg_color(&style, lv_palette_main(LV_PALETTE_BLUE));
    lv_style_set_bg_grad_color(&style, lv_palette_darken(LV_PALETTE_BLUE, 2));
    lv_style_set_bg_grad_dir(&style, LV_GRAD_DIR_VER);

    lv_style_set_border_opa(&style, LV_OPA_40);
    lv_style_set_border_width(&style, 2);
    lv_style_set_border_color(&style, lv_palette_main(LV_PALETTE_GREY));

    lv_style_set_shadow_width(&style, 8);
    lv_style_set_shadow_color(&style, lv_palette_main(LV_PALETTE_GREY));
    lv_style_set_shadow_offset_y(&style, 8);

    lv_style_set_outline_opa(&style, LV_OPA_COVER);
    lv_style_set_outline_color(&style, lv_palette_main(LV_PALETTE_BLUE));

    lv_style_set_text_color(&style, lv_color_white());
    lv_style_set_pad_all(&style, 10);

    /*Init the pressed style*/
    static lv_style_t style_pr;
    lv_style_init(&style_pr);

    /*Add a large outline when pressed*/
    lv_style_set_outline_width(&style_pr, 30);
    lv_style_set_outline_opa(&style_pr, LV_OPA_TRANSP);

    lv_style_set_translate_y(&style_pr, 5);
    lv_style_set_shadow_offset_y(&style_pr, 3);
    lv_style_set_bg_color(&style_pr, lv_palette_darken(LV_PALETTE_BLUE, 2));
    lv_style_set_bg_grad_color(&style_pr, lv_palette_darken(LV_PALETTE_BLUE, 4));

    /*Add a transition to the outline*/
    static lv_style_transition_dsc_t trans;
    static lv_style_prop_t props[] = {LV_STYLE_OUTLINE_WIDTH, LV_STYLE_OUTLINE_OPA, 0};
    lv_style_transition_dsc_init(&trans, props, lv_anim_path_linear, 300, 0, NULL);

    lv_style_set_transition(&style_pr, &trans);

    lv_obj_remove_style_all(btn);                          /*Remove the style coming from the theme*/
    lv_obj_add_style(btn, &style, 0);
    lv_obj_add_style(btn, &style_pr, LV_STATE_PRESSED);

}

static void menu_add_button(lv_obj_t* cont, lv_event_cb_t cb, const char* txt)
{
    auto btn = lv_button_create(cont);
    lv_obj_t *label = lv_label_create(btn);
    lv_label_set_text(label, txt);
    lv_obj_center(label);
    button_add_style(btn);
    lv_obj_add_event(btn, cb, LV_EVENT_SHORT_CLICKED, NULL);
}

void lvgl_top_menu(lv_obj_t *scr)
{
    /*Create a container with ROW flex direction*/
    lv_obj_t * cont_row = lv_obj_create(lv_layer_top());
    lv_obj_set_size(cont_row, 1024, 100);
    lv_obj_align(cont_row, LV_ALIGN_TOP_MID, 0, 5);
    lv_obj_set_flex_flow(cont_row, LV_FLEX_FLOW_ROW);

    menu_add_button(cont_row, button1_evt_cb, "Unprov");
    menu_add_button(cont_row, button2_evt_cb, "Nodes");
    menu_add_button(cont_row, button3_evt_cb, "OnOff");
    menu_add_button(cont_row, button4_evt_cb, "Groups");
}
