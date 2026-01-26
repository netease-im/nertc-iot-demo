#include "iot_c_api.h"
#include "thing_manager.h"
#include "cJSON.h"
#include <rtthread.h>
#include <string.h>

static std::string descriptors_json;
static std::string states_json;

extern "C" {

void iot_initialize() {
    auto& manager = iot::ThingManager::GetInstance();
    rt_kprintf("Registering Speaker...\n");
    manager.AddThing(iot::CreateThing("Speaker"));
}

void iot_invoke(const uint8_t* data, uint16_t len) {
    char* json_str = new char[len + 1];
    memcpy(json_str, data, len);
    json_str[len] = '\0';

    cJSON* root = cJSON_Parse(json_str);
    if (root == nullptr) {
        rt_kprintf("[IoT] Failed to parse command JSON\n");
        rt_free(json_str);
        return;
    }

    // 打印完整 JSON
    rt_kprintf("[IoT] Received command: %s\n", cJSON_PrintUnformatted(root));

    // 直接调用 ThingManager::Invoke(root)
    auto& manager = iot::ThingManager::GetInstance();
    manager.Invoke(root);  // 这里直接传 root 即可

    cJSON_Delete(root);
    rt_free(json_str);
}

const char* iot_get_descriptors_json() {
    auto& manager = iot::ThingManager::GetInstance();
    descriptors_json = manager.GetDescriptorsJson();
    return descriptors_json.c_str();
}

const char* iot_get_states_json() {
    auto& manager = iot::ThingManager::GetInstance();
    states_json.clear(); // 清空旧数据
    manager.GetStatesJson(states_json, false); // 获取所有设备的状态
    return states_json.c_str();
}

} // extern "C"