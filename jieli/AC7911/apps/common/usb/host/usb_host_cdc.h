#ifndef __USB_HOST_CDC_H__
#define __USB_HOST_CDC_H__

#include "asm/usb.h"
#include "device.h"

#ifdef __cplusplus
extern "C" {
#endif

/*Class-Specific Request Codes*/
#define CDC_SEND_ENCAPSULATED_COMMAND                           0x00
#define CDC_GET_ENCAPSULATED_RESPONSE                           0x01
#define CDC_SET_COMM_FEATURE                                    0x02
#define CDC_GET_COMM_FEATURE                                    0x03
#define CDC_CLEAR_COMM_FEATURE                                  0x04

#define CDC_SET_AUX_LINE_STATE                                  0x10
#define CDC_SET_HOOK_STATE                                      0x11
#define CDC_PULSE_SETUP                                         0x12
#define CDC_SEND_PULSE                                          0x13
#define CDC_SET_PULSE_TIME                                      0x14
#define CDC_RING_AUX_JACK                                       0x15

#define CDC_SET_LINE_CODING                                     0x20
#define CDC_GET_LINE_CODING                                     0x21
#define CDC_SET_CONTROL_LINE_STATE                              0x22
#define CDC_SEND_BREAK                                          0x23

#define CDC_SET_RINGER_PARMS                                    0x30
#define CDC_GET_RINGER_PARMS                                    0x31
#define CDC_SET_OPERATION_PARMS                                 0x32
#define CDC_GET_OPERATION_PARMS                                 0x33
#define CDC_SET_LINE_PARMS                                      0x34
#define CDC_GET_LINE_PARMS                                      0x35
#define CDC_DIAL_DIGITS                                         0x36
#define CDC_SET_UNIT_PARAMETER                                  0x37
#define CDC_GET_UNIT_PARAMETER                                  0x38
#define CDC_CLEAR_UNIT_PARAMETER                                0x39
#define CDC_GET_PROFILE                                         0x3A

#define CDC_SET_ETHERNET_MULTICAST_FILTERS                      0x40
#define CDC_SET_ETHERNET_POWER_MANAGEMENT_PATTERN FILTER        0x41
#define CDC_GET_ETHERNET_POWER_MANAGEMENT_PATTERN FILTER        0x42
#define CDC_SET_ETHERNET_PACKET_FILTER                          0x43
#define CDC_GET_ETHERNET_STATISTIC                              0x44

#define CDC_SET_ATM_DATA_FORMAT                                 0x50
#define CDC_GET_ATM_DEVICE_STATISTICS                           0x51
#define CDC_SET_ATM_DEFAULT_VC                                  0x52
#define CDC_GET_ATM_VC_STATISTICS                               0x53


/* wValue for SetControlLineState*/
#define CDC_ACTIVATE_CARRIER_SIGNAL_RTS                         0x0002
#define CDC_DEACTIVATE_CARRIER_SIGNAL_RTS                       0x0000
#define CDC_ACTIVATE_SIGNAL_DTR                                 0x0001
#define CDC_DEACTIVATE_SIGNAL_DTR                               0x0000



struct usb_cdc_platform_data {
    u32 baud_rate;     /*Data terminal rate, in bits per second*/
    u8  stop_bits;     /*Stop bits*/
    u8  parity;        /* Parity */
    u8  data_bits;     /* Data bits (5, 6, 7, 8 or 16). */
};

#define USB_CDC_PLATFORM_DATA_BEGIN(data) \
    static const struct usb_cdc_platform_data data = {\

#define USB_CDC_PLATFORM_DATA_END()  \
};

struct h_usb_cdc {
    u16 itf_num;
    u8  active;
    u8  host_epout;
    u8  epout;
    u32 txmaxp;
    u8 *epout_buffer;
    u8  host_epin;
    u8  epin;
    u32 rxmaxp;
    u8 *epin_buffer;
    OS_MUTEX mutex;
};

extern const struct device_operations usb_cdc_ops;

struct usb_host_device;
s32 usb_cdc_parser(struct usb_host_device *host_dev, s32 idx, u8 *pBuf);
s32 usb_cdc_init(usb_dev usb_id);
s32 usb_cdc_exit(usb_dev usb_id);
void host_cdc_send_demo(u8 usb_id);

#ifdef __cplusplus
}
#endif
#endif
