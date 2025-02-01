#pragma once
#include "lvgl.h"
#include "mesh.h"

ble_mesh_comp_t *mesh_get_composition(uint16_t addr);

void model_1000_modal_action(lv_obj_t* parent, uint16_t address);

void onoff_client_publish(uint16_t addr, bool on);
