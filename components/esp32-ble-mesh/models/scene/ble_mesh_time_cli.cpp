#include "ble_mesh_time_models.h"

void BLEmeshTimeCli::init(void* p)
{
    (void)p;
    model_id = ESP_BLE_MESH_MODEL_ID_TIME_CLI;
    BLE_MESH_MODEL_PUB_DEFINE(&pub, 2 + 20, ROLE_NODE, __COUNTER__); /*!< msg len: 2 (opcode) */
    _model = new esp_ble_mesh_model_t(ESP_BLE_MESH_MODEL_TIME_CLI(&pub, &_data));
    pub.msg->len = 12;
    pub.publish_addr = 0xffff;
    esp_ble_mesh_model_msg_opcode_init(pub.msg->data, ESP_BLE_MESH_MODEL_OP_TIME_SET);
}
