#include "esp_err.h"
#include "esp_check.h"
#include <map>

extern "C"
{
#include "esp_hosted_bt.h"
}
#include "json.hpp"

#include "mesh.h"
#include "models.h"

#include "ble_mesh_provisioner.h"
#include "generic_on_off_models.h"

static void mesh_composition_data_parse(uint16_t addr);
extern "C" void nvs_dump(const char *partName);
void lvgl_modal_update_subs();
void lvgl_refresh_unprovisioned_device();

static BLEmeshProvisioner *provisioner;
static std::map<uint16_t, ble_mesh_comp_t> nodes_comp;

class ConfigCliCB : public BLEmeshModelCb
{
    virtual void onEvent(IBLEMeshModel *model, uint32_t event, uint32_t opcode, void *params) override;
};

static void init_hosted_ble()
{
    /* initialize TRANSPORT first */
    hosted_hci_bluedroid_open();

    /* get HCI driver operations */
    esp_bluedroid_hci_driver_operations_t operations = {
        .send = hosted_hci_bluedroid_send,
        .check_send_available = hosted_hci_bluedroid_check_send_available,
        .register_host_callback = hosted_hci_bluedroid_register_host_callback,
    };
    ESP_ERROR_CHECK_WITHOUT_ABORT(esp_bluedroid_attach_hci_driver(&operations));
}

void mesh_model_sub_get(uint16_t addr, uint16_t model_id);
void init_ble_mesh()
{
    esp_err_t err;
    err = nvs_flash_init();
    if (err == ESP_ERR_NVS_NO_FREE_PAGES)
    {
        printf("flash init error: %d\n\n\n", err);
        ESP_ERROR_CHECK(nvs_flash_erase());
        err = nvs_flash_init();
    }
    ESP_ERROR_CHECK(err);

    init_hosted_ble(); // esp32-p4 only
    // add all models
    provisioner = BLEmeshProvisioner::GetInstance();
    auto onoffCli = new GenericOnOffCliModel("on-off cli");
    provisioner->addPrimaryModel(onoffCli);

    // when all models are added
    provisioner->init_ble();
    provisioner->init_mesh();

    // when ble mesh is init
    provisioner->registerCallbacks();
    provisioner->selfProvisioning();
    provisioner->enableProvisioning(ESP_BLE_MESH_PROV_GATT);

    provisioner->configCli()->setCb(new ConfigCliCB());
    onoffCli->bindLocalAppKey();
    onoffCli->keys(0, 0); // hardcoded

    auto count = provisioner->nodesCount();
    auto nodes = provisioner->getNodes();

    for (size_t i = 0; i < count; i++)
    {
        auto node = nodes[i];
        auto addr = node->unicast_addr;
        mesh_composition_data_parse(addr);
    }
}

void mesh_node_add_app_key(uint16_t addr)
{
    provisioner->configCli()->nodeAppKeyAdd(addr, 0, 0);
}

void mesh_node_get_comp_data(uint16_t addr)
{
    provisioner->configCli()->nodeCompositionDataGet(addr, 0);
}

void ConfigCliCB::onEvent(IBLEMeshModel *model, uint32_t event, uint32_t opcode, void *params)
{
    auto param = (esp_ble_mesh_cfg_client_cb_param_t *)params;
    auto unicast_addr = param->params->ctx.addr;
    auto node = esp_ble_mesh_provisioner_get_node_with_addr(unicast_addr);
    printf("\t\tCB: 0x%04lx\n", opcode);
    if(event == ESP_BLE_MESH_CFG_CLIENT_TIMEOUT_EVT)
    {
        auto opcode = param->params->opcode;
        switch (opcode)
        {
        case ESP_BLE_MESH_MODEL_OP_NODE_RESET:
            esp_ble_mesh_provisioner_delete_node_with_addr(unicast_addr);
            break;
        
        default:
            ESP_LOGW("", "CLI timeout not handled opcode: 0x%04x", opcode);
            break;
        }
        return;
    }
    switch (opcode)
    {
    case ESP_BLE_MESH_MODEL_OP_COMPOSITION_DATA_STATUS:
    {
        ESP_LOGI(TAG, "composition data: %d => %s", unicast_addr, bt_hex(param->status_cb.comp_data_status.composition_data->data, param->status_cb.comp_data_status.composition_data->len));
        ESP_ERROR_CHECK_WITHOUT_ABORT(esp_ble_mesh_provisioner_store_node_comp_data(unicast_addr, param->status_cb.comp_data_status.composition_data->data, param->status_cb.comp_data_status.composition_data->len));

        mesh_node_add_app_key(unicast_addr);
        bsp_display_lock(0);
        lvgl_refresh_unprovisioned_device();
        bsp_display_unlock();
        break;
    }
    case ESP_BLE_MESH_MODEL_OP_APP_KEY_STATUS:
    {
        printf("\t\tNODE: %d\n", node->comp_length);
        mesh_composition_data_parse(unicast_addr);
        break;
    }
    case ESP_BLE_MESH_MODEL_OP_MODEL_SUB_STATUS:
    {
        auto model_sub_status = param->status_cb.model_sub_status;
        auto status = model_sub_status.status;
        auto sub_addr = model_sub_status.sub_addr;
        auto element_addr = model_sub_status.element_addr;
        auto company_id = model_sub_status.company_id;
        auto model_id = model_sub_status.model_id;
        if(status) return;
        auto node = mesh_get_composition(element_addr);
        for (size_t i = 0; i < node->element_num; i++)
        {
            auto&& el = node->elements[i];
            for (size_t j = 0; j < el.count; j++)
            {
                if(el.models[j].mod_id != model_id or element_addr != el.elem_addr) continue;
                auto&& model = el.models[j];
                bool remove = false;
                for (size_t k = 0; k < 3; k++)
                {
                    if(model.subs[k] == sub_addr)
                    {
                        model.subs[k] = 0;
                        remove = true;
                    }
                }
                if(!remove)
                {
                    for (size_t k = 0; k < 3; k++)
                    {
                        if(model.subs[k] == 0)
                        {
                            printf("3: 0x%04x\n", sub_addr);
                            model.subs[k] = sub_addr;
                            k = 5;
                        }
                    }
                }
            }
        }
        lvgl_modal_update_subs();

        break;
    }
    case ESP_BLE_MESH_MODEL_OP_SIG_MODEL_SUB_LIST:
    {
        auto model_sub_list = param->status_cb.model_sub_list;
        auto status = model_sub_list.status;
        auto element_addr = model_sub_list.element_addr;
        auto company_id = model_sub_list.company_id;
        auto model_id = model_sub_list.model_id;
        auto net_buf = model_sub_list.sub_addr;

        auto data = net_buf->data;
        auto len = net_buf->len;
        ESP_LOG_BUFFER_HEX("", data, len);
        auto comp = mesh_get_composition(element_addr);
        printf("\t\t\t ELEM: %d\n", element_addr - unicast_addr);
        for (size_t i = 0; i < comp->element_num; i++)
        {
            if (comp->elements[i].elem_addr != element_addr)
                continue;
            auto&& el = comp->elements[i];
            for (size_t j = 0; j < el.count; j++)
            {
                if (el.models[j].mod_id != model_id)
                    continue;
                for (size_t k = 0; k < len / 2; k++)
                {
                    uint16_t sub_adr = ((uint16_t *)data)[k];
                    el.models[j].subs[k] = sub_adr;
                    printf("\tstatus: %d, len: %d, SUB: 0x%04x\n", status, k, sub_adr);
                }
            }
        }
        lvgl_modal_update_subs();
        break;
    }
    case ESP_BLE_MESH_MODEL_OP_MODEL_APP_STATUS:
    {
        mesh_model_sub_get(param->status_cb.model_app_status.element_addr, 0x1000);
        break;
    }
    case ESP_BLE_MESH_MODEL_OP_NODE_RESET_STATUS:
    {
        esp_ble_mesh_provisioner_delete_node_with_addr(unicast_addr);
        break;
    }
    default:
        ESP_LOGW("", "CLI cb not handled opcode: 0x%04x", opcode);
        break;
    }
}
//---------------------------------------------------//

//-------------------------screen 1------------------------------//

void provisioner_device_prov(const char *uuid)
{
    provisioner->addUnprovisioned(uuid);
}

void mesh_model_app_key_bind(uint16_t addr, uint16_t model_id, uint8_t appIdx)
{
    auto node_addr = mesh_get_composition(addr)->node_addr;
    provisioner->configCli()->modelAppKeyBind(node_addr, addr, appIdx, model_id);
}

void mesh_model_app_key_unbind(uint16_t addr, uint16_t model_id, uint8_t appIdx)
{
    auto node_addr = mesh_get_composition(addr)->node_addr;
    provisioner->configCli()->modelAppKeyUnbind(node_addr, addr, appIdx, model_id);
}

void mesh_model_sub_get(uint16_t addr, uint16_t model_id)
{
    auto node_addr = mesh_get_composition(addr)->node_addr;
    provisioner->configCli()->modelSubGet(node_addr, addr, model_id);
}

void mesh_model_sub_add(uint16_t addr, uint16_t sub, uint16_t model_id)
{
    auto node = mesh_get_composition(addr);
    for (size_t i = 0; i < node->element_num; i++)
    {
        auto&& el = node->elements[i];
        for (size_t j = 0; j < el.count; j++)
        {
            if(el.models[j].mod_id != model_id or addr != el.elem_addr) continue;
            auto&& model = el.models[j];
            bool remove = false;
            for (size_t k = 0; k < 3; k++)
            {
                if(model.subs[k] == sub)
                {
                    return;
                }
            }
        }
    }

    provisioner->configCli()->modelSubAdd(node->node_addr, addr, sub, model_id);
}

void mesh_model_sub_del(uint16_t addr, uint16_t sub, uint16_t model_id)
{
    auto node = mesh_get_composition(addr);
    provisioner->configCli()->modelSubDelete(node->node_addr, addr, sub, model_id);
}

void mesh_client_bind(uint16_t addr, uint16_t model)
{
    auto comp = *mesh_get_composition(addr);

    for (size_t i = 0; i < comp.element_num; i++)
    {
        auto el = comp.elements[i];
        auto node_addr = comp.node_addr;
        printf("\tnode: 0x%04X, 0x%04X => 0x%04X\n", node_addr, addr, el.elem_addr);
        if (el.elem_addr == addr)
        {
            for (size_t j = 0; j < el.count; j++)
            {
                if ((el.models[j].mod_id != 0 and el.models[j].vnd_id == 0xffff) or el.models[j].vnd_id != 0xffff)
                {
                    assert(el.models[j].vnd_id);
                    printf("\t\tEL: %d, model: 0x%04X, vendor: 0x%04X\n", i, el.models[j].mod_id, el.models[j].vnd_id);
                    mesh_model_sub_get(addr, model);
                }
            }
        }
    }
}

void mesh_node_reset_node(uint16_t addr)
{
    esp_ble_mesh_node_t* node = esp_ble_mesh_provisioner_get_node_with_addr(addr);
    provisioner->configCli()->nodeReset(addr);
}

void onoff_client_publish(uint16_t addr, bool on)
{
    auto model = (GenericOnOffCliModel *)provisioner->findModel(0x1001);
    assert(model);
    if (on)
        model->turnOn(addr);
    else
        model->turnOff(addr);
}

static void mesh_composition_data_parse(uint16_t addr)
{
    esp_ble_mesh_node_t *node = esp_ble_mesh_provisioner_get_node_with_addr(addr);
    ble_mesh_comp_t comp = {};
    if(!node->comp_length) return;
    comp.comp_length = node->comp_length;
    comp.cid = *(uint16_t *)node->comp_data;
    comp.node_addr = addr;
    comp.comp_data = node->comp_data;
    comp.element_num = node->element_num;

    auto data = comp.comp_data + 10;
    for (size_t i = 0; i < comp.element_num; i++)
    {
        auto &el = comp.elements[i];
        el.elem_addr = addr + i;
        data = data + 2; // ignore element position
        uint8_t sig_cnt = *(uint8_t *)data;
        data++;
        uint8_t vnd_cnt = *(uint8_t *)data;
        data++;
        size_t j = 0;
        for (; j < sig_cnt; j++)
        {
            el.models[j].mod_id = *(uint16_t *)data;
            el.models[j].vnd_id = 0xffff;
            data = data + 2;
            // printf("\tEL: %d, model: 0x%04X, vendor: 0x%04X\n", i, el.models[j].mod_id, el.models[j].vnd_id);
            el.count++;
        }

        for (; j < vnd_cnt + sig_cnt; j++)
        {
            el.models[j].vnd_id = *(uint16_t *)data;
            data = data + 2;
            el.models[j].mod_id = *(uint16_t *)data;
            data = data + 2;
            el.count++;
            // printf("\tEL: %d, model: 0x%04X, vendor: 0x%04X\n", i, el.models[j].mod_id, el.models[j].vnd_id);
        }
    }

    nodes_comp[addr] = comp;
}

ble_mesh_comp_t* mesh_get_composition(uint16_t addr)
{
    for (auto&& node : nodes_comp)
    {
        for (size_t i = 0; i < node.second.element_num; i++)
        {
            if (node.second.elements[i].elem_addr == addr)
            {
                return (ble_mesh_comp_t*)&node.second;
            }
        }
    }

    return &nodes_comp[addr];
}

const char *mesh_model_get_type(uint16_t id)
{
    switch (id)
    {
    case 0:
        return "config srv";
    case 1:
        return "config cli";
    case 2:
        return "health srv";
    case 3:
        return "health cli";
    case 0x1000:
        return "gen on/off srv";
    case 0x1001:
        return "gen on/off cli";
    case 0x1002:
        return "gen level srv";
    case 0x1003:
        return "gen level cli";
    case 0x100e:
        return "gen loc srv";
    case 0x100f:
        return "gen loc setup srv";
    case 0x1010:
        return "gen loc cli";
    case 0x100c:
        return "gen battery srv";
    case 0x100d:
        return "gen battery cli";
    case 0x1200:
        return "time srv";
    case 0x1201:
        return "time setup srv";
    case 0x1202:
        return "time cli";
    case 0x1203:
        return "scene srv";
    case 0x1204:
        return "scene setup srv";
    case 0x1205:
        return "scene cli";
    case 0x1300:
        return "lightness srv";
    case 0x1302:
        return "lightness cli";
    case 0x1307:
        return "light hsl srv";
    case 0x1309:
        return "light hsl cli";
    default:
        return "not supported";
    }
}
