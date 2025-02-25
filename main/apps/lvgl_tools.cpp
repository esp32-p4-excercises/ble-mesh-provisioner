
#include "lvgl.h"


lv_obj_t *create_button(lv_obj_t *parent, const char *txt)
{
    auto btn = lv_btn_create(parent);
    auto lbl = lv_label_create(btn);
    lv_label_set_text(lbl, txt);
    return btn;
}
