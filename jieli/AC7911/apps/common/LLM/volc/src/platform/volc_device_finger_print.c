#include "volc_device_finger_print.h"

//#include <esp_netif.h>
#include "wifi/wifi_connect.h"
#include "volc_type.h"
#include "os/os_api.h"
uint32_t volc_get_mac_address(volc_string_t *mac_addr)
{
    volc_debug("%s\n", __FUNCTION__);
    uint32_t ret = VOLC_STATUS_SUCCESS;
//    esp_netif_t *netif = NULL;
    VOLC_CHK(NULL != mac_addr, VOLC_STATUS_INVALID_ARG);

//    while ((netif = esp_netif_next_unsafe(netif)) != NULL) {
    uint8_t hwaddr[6] = {0};
//        esp_netif_get_mac(netif, hwaddr);
    wifi_get_mac(hwaddr);
    volc_string_snprintf(mac_addr, 32, "%2X%2X%2X%2X%2X%2X", hwaddr[0], hwaddr[1], hwaddr[2], hwaddr[3], hwaddr[4], hwaddr[5]);
//    }

err_out_label:
    return ret;
}

uint32_t volc_get_device_finger_print(volc_string_t *finger)
{
    volc_debug("%s\n", __FUNCTION__);
    uint32_t ret = VOLC_STATUS_SUCCESS;
    VOLC_CHK(NULL != finger, VOLC_STATUS_INVALID_ARG);
    volc_get_mac_address(finger);
err_out_label:
    return ret;
}

uint32_t volc_generate_device_id(volc_string_t *did)
{
    volc_debug("%s\n", __FUNCTION__);
    uint32_t ret = VOLC_STATUS_SUCCESS;
    volc_string_t device_id;
    VOLC_CHK(NULL != did, VOLC_STATUS_INVALID_ARG);
    volc_string_init(&device_id);
    volc_get_mac_address(&device_id);
    volc_string_set(did, volc_string_get(&device_id));
    volc_string_deinit(&device_id);
err_out_label:
    return ret;
}
