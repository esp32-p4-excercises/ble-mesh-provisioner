#include <string.h>

#include "esp_log.h"
#ifdef CONFIG_BT_BLUEDROID_ENABLED
// #include "esp_bt.h"
#include "esp_bt_main.h"
#include "esp_bt_device.h"
#include "esp_gatt_common_api.h"
#endif
#include "ble_mesh_prov.h"

#define TAG "prov"

uint16_t BLEmeshProvisioning::_net_idx = 0;

