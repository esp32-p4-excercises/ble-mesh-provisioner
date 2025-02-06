#pragma once

#include "ble_mesh_model.h"
#include "esp_ble_mesh_lighting_model_api.h"

#include "callbacks.h"


/**
 * @brief 
 * 
 */
class ILightSrvModel : public IBLEMeshModel
{
public:
    using IBLEMeshModel::IBLEMeshModel;
    ~ILightSrvModel() {}

    esp_err_t registerCallback() override {
        return registerCallback(NULL);
    }

    esp_err_t registerCallback(esp_ble_mesh_lighting_server_cb_t callback)
    {
        if (callback == NULL)
        {
            return esp_ble_mesh_register_lighting_server_callback(_default_ble_mesh_lighting_server_cb);
        }

        return esp_ble_mesh_register_lighting_server_callback(callback);
    }

    /**
     * @brief 
     * 
     * @param address 
     * @param data 
     * @return esp_err_t 
     */
    // virtual esp_err_t sendStatus(uint16_t address, uint8_t *data)
    // {
    //     auto ctx = getCtx();
    //     ctx->addr = address;
    //     return esp_ble_mesh_server_model_send_msg(_model, ctx, get_opcode, status_msg_len, data);
    // }

    /**
     * @brief 
     * 
     * @param data 
     * @return esp_err_t 
     */
    // virtual esp_err_t publishStatus(uint8_t *data)
    // {
    //     return esp_ble_mesh_model_publish(_model, get_opcode, status_msg_len, data, ROLE_NODE);
    // }
};

class ILightCliModel : public IBLEMeshModel
{
private:
public:
    using IBLEMeshModel::IBLEMeshModel;
    ~ILightCliModel() {}

    esp_err_t registerCallback() override {
        return registerCallback(NULL);
    }

    esp_err_t registerCallback(esp_ble_mesh_light_client_cb_t callback)
    {
        if (callback == NULL)
        {
            return esp_ble_mesh_register_light_client_callback(_default_ble_mesh_lighting_client_cb);
        }

        return esp_ble_mesh_register_light_client_callback(callback);
    }

// esp_err_t esp_ble_mesh_light_client_get_state(esp_ble_mesh_client_common_param_t *params, esp_ble_mesh_light_client_get_state_t *get_state);
// esp_err_t esp_ble_mesh_light_client_set_state(esp_ble_mesh_client_common_param_t *params, esp_ble_mesh_light_client_set_state_t *set_state);

    /**
     * @brief send GET status to publish address setup with provisioner (usually)
     * 
     * @param data[in] not used
     * @return esp_err_t 
     */
    // virtual esp_err_t requestStatus(uint8_t *data)
    // {
    //     return esp_ble_mesh_model_publish(_model, get_opcode, status_msg_len, data, ROLE_NODE);
    // }
};
