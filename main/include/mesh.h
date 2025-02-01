#pragma once
#include "bsp/esp-bsp.h"

#include <stdio.h>
#include <cstdint>


typedef struct {
    uint16_t mod_id;
    uint16_t vnd_id;
    uint8_t netIdx;
    uint8_t appIdx;
    uint16_t subs[3];
} ble_mesh_model_t;

typedef struct 
{
    uint16_t elem_addr;
    ble_mesh_model_t models[10];
    uint8_t count;
} ble_mesh_element_t;

typedef struct
{
    uint16_t cid;
    uint16_t node_addr;
    uint8_t element_num;
    uint16_t comp_length;
    uint8_t *comp_data;
    ble_mesh_element_t elements[10];
} ble_mesh_comp_t;

