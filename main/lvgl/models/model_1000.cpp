
#include "lvgl.h"

#include "mesh.h"

void model_1000_modal_action(lv_obj_t* parent, uint16_t address)
{
    auto mbox = lv_msgbox_create(parent);
    lv_msgbox_add_title(mbox, "Generic OnOff controls");
    lv_obj_set_width(mbox, LV_PCT(100));

    auto btn = lv_msgbox_add_footer_button(mbox, "Read state");

    btn = lv_msgbox_add_footer_button(mbox, "On");

}
