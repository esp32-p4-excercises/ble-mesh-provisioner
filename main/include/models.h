#pragma once
#include "lvgl.h"
#include "mesh.h"

static uint16_t models_id[] = {0xc000, 0xc001, 0xc002, 0xffff};

ble_mesh_comp_t *mesh_get_composition(uint16_t addr);

void model_1000_modal_action(lv_obj_t* parent, uint16_t address, bool clean = true);
void model_1002_modal_action(lv_obj_t* parent, uint16_t address, bool clean = true);
void model_1307_modal_action(lv_obj_t* parent, uint16_t address, bool clean = true);

