#include "app_config.h"
#include "system/timer.h"
#include "device/ioctl_cmds.h"
#include "generic/includes.h"
#include "system/init.h"

#if TCFG_HOST_AUDIO_ENABLE
#include "usb/host/usb_host.h"
#include "usb_ctrl_transfer.h"
#include "usb_bulk_transfer.h"
#include "audio.h"
#include "usb_config.h"
#include "event/device_event.h"

#define LOG_TAG_CONST       USB
#define LOG_TAG             "[AUDIO]"
#define LOG_ERROR_ENABLE
#define LOG_DEBUG_ENABLE
#define LOG_INFO_ENABLE
/* #define LOG_DUMP_ENABLE */
#define LOG_CLI_ENABLE
#include "debug.h"


#define retval_check(ret, out) \
    if ((int)(ret) < 0) { \
        log_error("ret %d", (ret)); \
        if ((out)) { \
            goto __exit_fail; \
        } \
    }

#define my_abs(a)    ((a) > 0 ? (a) : (-(a)))

/* #define USB_AUDIO_PLAY_TEST */

struct usb_audio_mic {
    u32 sample_rate;
    u8 *send_buf;
    u8 *usb_audio_mic_buf;
    u8 *usb_audio_mic_buf2;
    cbuffer_t usb_audio_record_cbuf;
    u32 usb_audio_remain_len;
    OS_SEM sem;
    int (*get_buf)(const usb_dev usb_id, void *ptr, u32 len, u8 channel, u32 sample_rate);

    u16 vol_cur;
    u16 vol_min;
    u16 vol_max;
    u16 vol_res;
    u16 vol_l_cur;
    u16 vol_l_min;
    u16 vol_l_max;
    u16 vol_l_res;
    u16 vol_r_cur;
    u16 vol_r_min;
    u16 vol_r_max;
    u16 vol_r_res;

    u16 mute_en : 1;
    u16 vol_en : 1;
    u16 mute_l_en : 1;
    u16 vol_l_en : 1;
    u16 mute_r_en : 1;
    u16 vol_r_en : 1;
    u16 mute : 1;
    u16 mute_l : 1;
    u16 mute_r : 1;
    u16 sr_ctrl_en : 1;
    u16 reserve : 6;

    u8 i_t_id;  //input terminal index
    u8 f_u_id;  //feature unit index
    u8 o_t_id;  //output terminal index

    u8 Cur_AlternateSetting;
    u8 src_channel;
    u8 mic_state;
};
struct usb_audio_play {
    u32 sample_rate;
    u8 *usb_audio_spk_buf;
    u8 *usb_audio_spk_buf2;
    u8 *buffer;
    cbuffer_t usb_audio_spk_cbuf;
    int (*put_buf)(const usb_dev usb_id, void *ptr, u32 len, u8 channel, u32 sample_rate);

    u16 vol_cur;
    u16 vol_min;
    u16 vol_max;
    u16 vol_res;
    u16 vol_l_cur;
    u16 vol_l_min;
    u16 vol_l_max;
    u16 vol_l_res;
    u16 vol_r_cur;
    u16 vol_r_min;
    u16 vol_r_max;
    u16 vol_r_res;

    u16 mute_en : 1;
    u16 vol_en : 1;
    u16 agc_en : 1;
    u16 mute_l_en : 1;
    u16 vol_l_en : 1;
    u16 agc_l_en : 1;
    u16 mute_r_en : 1;
    u16 vol_r_en : 1;
    u16 agc_r_en : 1;
    u16 mute : 1;
    u16 mute_l : 1;
    u16 mute_r : 1;
    u16 agc : 1;
    u16 agc_l : 1;
    u16 agc_r : 1;
    u16 sr_ctrl_en : 1;

    u8 i_t_id;  //input terminal index
    u8 f_u_id;  //feature unit index
    u8 o_t_id;  //output terminal index

    u8 Cur_AlternateSetting;
    u8 spk_state;
    u8 src_channel;
};
struct usb_audio_info {
    struct usb_audio_mic host_mic;
    struct usb_audio_play host_spk;
    OS_SEM *wait_sem;
};

enum {
    AUDIO_MIC_IDLE = 0,
    AUDIO_MIC_START,
    AUDIO_MIC_STOP,
    AUDIO_MIC_PAUSE,
};
enum {
    AUDIO_SPK_IDLE = 0,
    AUDIO_SPK_START,
    AUDIO_SPK_STOP,
    AUDIO_SPK_PAUSE,
};

struct usb_audio_info _usb_audio_info[USB_MAX_HW_NUM] = {0};
#define __this   (&_usb_audio_info[usb_id])

struct audio_device_t audio_device[USB_MAX_HW_NUM][MAX_HOST_INTERFACE];

static u8 *ep_in_dma_buf[USB_MAX_HW_NUM];
static u8 *ep_out_dma_buf[USB_MAX_HW_NUM];

static int set_power(struct usb_host_device *host_dev, u32 value)
{
    const usb_dev usb_id = host_device2id(host_dev);
    return 0;
}

static int get_power(struct usb_host_device *host_dev, u32 value)
{
    return 0;
}

static const struct interface_ctrl uac_ctrl = {
    .interface_class = USB_CLASS_AUDIO,
    .set_power = set_power,
    .get_power = get_power,
    .ioctl = NULL,
};

static const struct usb_interface_info _uac_if[USB_MAX_HW_NUM][MAX_HOST_INTERFACE] = {
    {
        {
            .ctrl = (struct interface_ctrl *) &uac_ctrl,
            .dev.audio = &audio_device[0][0],
        },
        {
            .ctrl = (struct interface_ctrl *) &uac_ctrl,
            .dev.audio = &audio_device[0][1],
        },
        {
            .ctrl = (struct interface_ctrl *) &uac_ctrl,
            .dev.audio = &audio_device[0][2],
        },
        {
            .ctrl = (struct interface_ctrl *) &uac_ctrl,
            .dev.audio = &audio_device[0][3],
        },
        {
            .ctrl = (struct interface_ctrl *) &uac_ctrl,
            .dev.audio = &audio_device[0][4],
        },
        {
            .ctrl = (struct interface_ctrl *) &uac_ctrl,
            .dev.audio = &audio_device[0][5],
        },
        {
            .ctrl = (struct interface_ctrl *) &uac_ctrl,
            .dev.audio = &audio_device[0][6],
        },
        {
            .ctrl = (struct interface_ctrl *) &uac_ctrl,
            .dev.audio = &audio_device[0][7],
        },
    },
#if USB_MAX_HW_NUM > 1
    {
        {
            .ctrl = (struct interface_ctrl *) &uac_ctrl,
            .dev.audio = &audio_device[1][0],
        },
        {
            .ctrl = (struct interface_ctrl *) &uac_ctrl,
            .dev.audio = &audio_device[1][1],
        },
        {
            .ctrl = (struct interface_ctrl *) &uac_ctrl,
            .dev.audio = &audio_device[1][2],
        },
        {
            .ctrl = (struct interface_ctrl *) &uac_ctrl,
            .dev.audio = &audio_device[1][3],
        },
        {
            .ctrl = (struct interface_ctrl *) &uac_ctrl,
            .dev.audio = &audio_device[1][4],
        },
        {
            .ctrl = (struct interface_ctrl *) &uac_ctrl,
            .dev.audio = &audio_device[1][5],
        },
        {
            .ctrl = (struct interface_ctrl *) &uac_ctrl,
            .dev.audio = &audio_device[1][6],
        },
        {
            .ctrl = (struct interface_ctrl *) &uac_ctrl,
            .dev.audio = &audio_device[1][7],
        },
    }
#endif
};

static void reset_usb_host_audio_control_info(const usb_dev usb_id)
{
    __this->host_spk.vol_cur = 0;
    __this->host_spk.vol_min = 0;
    __this->host_spk.vol_max = 0;
    __this->host_spk.vol_res = 0;
    __this->host_spk.vol_l_cur = 0;
    __this->host_spk.vol_l_min = 0;
    __this->host_spk.vol_l_max = 0;
    __this->host_spk.vol_l_res = 0;
    __this->host_spk.vol_r_cur = 0;
    __this->host_spk.vol_r_min = 0;
    __this->host_spk.vol_r_max = 0;
    __this->host_spk.vol_r_res = 0;

    __this->host_spk.mute_en = 0;
    __this->host_spk.vol_en = 0;
    __this->host_spk.agc_en = 0;
    __this->host_spk.mute_l_en = 0;
    __this->host_spk.vol_l_en = 0;
    __this->host_spk.agc_l_en = 0;
    __this->host_spk.mute_r_en = 0;
    __this->host_spk.vol_r_en = 0;
    __this->host_spk.agc_r_en = 0;
    __this->host_spk.mute = 0;
    __this->host_spk.mute_l = 0;
    __this->host_spk.mute_r = 0;
    __this->host_spk.agc = 0;
    __this->host_spk.agc_l = 0;
    __this->host_spk.agc_r = 0;
    __this->host_spk.sr_ctrl_en = 0;

    __this->host_spk.i_t_id = 0;
    __this->host_spk.f_u_id = 0;
    __this->host_spk.o_t_id = 0;

    __this->host_mic.vol_cur = 0;
    __this->host_mic.vol_min = 0;
    __this->host_mic.vol_max = 0;
    __this->host_mic.vol_res = 0;
    __this->host_mic.vol_l_cur = 0;
    __this->host_mic.vol_l_min = 0;
    __this->host_mic.vol_l_max = 0;
    __this->host_mic.vol_l_res = 0;
    __this->host_mic.vol_r_cur = 0;
    __this->host_mic.vol_r_min = 0;
    __this->host_mic.vol_r_max = 0;
    __this->host_mic.vol_r_res = 0;

    __this->host_mic.mute_en = 0;
    __this->host_mic.vol_en = 0;
    __this->host_mic.mute_l_en = 0;
    __this->host_mic.vol_l_en = 0;
    __this->host_mic.mute_r_en = 0;
    __this->host_mic.vol_r_en = 0;
    __this->host_mic.mute = 0;
    __this->host_mic.mute_l = 0;
    __this->host_mic.mute_r = 0;
    __this->host_mic.sr_ctrl_en = 0;
    __this->host_mic.reserve = 0;

    __this->host_mic.i_t_id = 0;
    __this->host_mic.f_u_id = 0;
    __this->host_mic.o_t_id = 0;
}

int usb_audio_parser(struct usb_host_device *host_dev, u8 interface_num, const u8 *pBuf)
{
    struct usb_interface_descriptor *interface = (struct usb_interface_descriptor *)pBuf;
    const usb_dev usb_id = host_device2id(host_dev);

    const struct usb_interface_info *usb_if = &_uac_if[usb_id][interface_num];
    struct audio_streaming_t *as_t = NULL;
    memset(usb_if->dev.p, 0, sizeof(struct audio_device_t));
    host_dev->interface_info[interface_num] = usb_if;
    usb_if->dev.audio->parent = host_dev;

    if (interface->bInterfaceSubClass == USB_SUBCLASS_AUDIOCONTROL) {
        log_info("audio control interface : %d\n", interface_num);
        pBuf += sizeof(struct usb_interface_descriptor);
        usb_if->dev.audio->subclass = interface->bInterfaceSubClass;
        usb_if->dev.audio->interface_num = interface_num;

        reset_usb_host_audio_control_info(usb_id);

        int len = 0;
        int cur_len;
        int cur_type;
        int cur_sub_type;
        int bControlSize = 0;
        u16 wTotalLength = 0;
        u8 i_t_id = -1;
        u8 f_u_id = -1;
        u8 o_t_id = -1;
        const u8 *feature_desc = NULL;

        while (1) {
            cur_len = pBuf[len];
            cur_type = pBuf[len + 1];
            cur_sub_type = pBuf[len + 2];

            if (!cur_len || cur_type != USB_DT_CS_INTERFACE) {
                break;
            }
            if (wTotalLength > 0 && len >= wTotalLength) {
                break;
            }

            if (cur_sub_type == UAC_HEADER) {
                wTotalLength = pBuf[len + 6] << 8 | pBuf[len + 5];
                log_debug("UAC_HEADER: version %04x, TotalLength %d",
                          pBuf[len + 4] << 8 | pBuf[len + 3],
                          wTotalLength);
            } else if (cur_sub_type == UAC_INPUT_TERMINAL) {
                u16 terminal_type = pBuf[len + 5] << 8 | pBuf[len + 4];
                u16 ch = pBuf[len + 7];
                i_t_id = pBuf[len + 3];
                switch (terminal_type) {
                case UAC_INPUT_TERMINAL_MICROPHONE:
                case UAC_INPUT_TERMINAL_DESKTOP_MICROPHONE:
                case UAC_INPUT_TERMINAL_PERSONAL_MICROPHONE:
                case 0x0402:
                    __this->host_spk.i_t_id = i_t_id;
                    break;
                default:
                    break;
                }
                log_debug("INPUT_TERMINAL: %d 0x%x %d", i_t_id, terminal_type, ch);
            } else if (cur_sub_type == UAC_FEATURE_UNIT) {
                f_u_id = pBuf[len + 3];
                u8 src_id = pBuf[len + 4];
                bControlSize = pBuf[len + 5];
                feature_desc = pBuf + len;
                if (__this->host_spk.i_t_id == i_t_id && src_id == i_t_id) {
                    __this->host_spk.f_u_id = f_u_id;
                    __this->host_spk.mute_en = !!(feature_desc[6] & 0x1);
                    __this->host_spk.vol_en = !!(feature_desc[6] & 0x2);
                    __this->host_spk.agc_en = !!(feature_desc[6] & 0x40);
                    if (cur_len == (7 + (2 + 1) * bControlSize)) {
                        __this->host_spk.mute_l_en = !!(feature_desc[6 + bControlSize * 1] & 0x1);
                        __this->host_spk.vol_l_en = !!(feature_desc[6 + bControlSize * 1] & 0x2);
                        __this->host_spk.agc_l_en = !!(feature_desc[6 + bControlSize * 1] & 0x40);

                        __this->host_spk.mute_r_en = !!(feature_desc[6 + bControlSize * 2] & 0x1);
                        __this->host_spk.vol_r_en = !!(feature_desc[6 + bControlSize * 2] & 0x2);
                        __this->host_spk.agc_r_en = !!(feature_desc[6 + bControlSize * 2] & 0x40);
                    }
                }
                log_debug("FEATURE_UNIT: %d %d %d", f_u_id, src_id, bControlSize);
            } else if (cur_sub_type == UAC_OUTPUT_TERMINAL) {
                o_t_id = pBuf[len + 3];
                u8 src_id = pBuf[len + 7];
                u16 terminal_type = pBuf[len + 5] << 8 | pBuf[len + 4];
                if (__this->host_spk.i_t_id == i_t_id) {
                    __this->host_spk.o_t_id = o_t_id;
                }
                switch (terminal_type) {
                case UAC_OUTPUT_TERMINAL_SPEAKER:
                case UAC_OUTPUT_TERMINAL_HEADPHONES:
                case UAC_OUTPUT_TERMINAL_DESKTOP_SPEAKER:
                case UAC_OUTPUT_TERMINAL_ROOM_SPEAKER:
                case UAC_OUTPUT_TERMINAL_COMMUNICATION_SPEAKER:
                case UAC_OUTPUT_TERMINAL_HEADSET:
                    __this->host_mic.i_t_id = i_t_id;
                    __this->host_mic.f_u_id = f_u_id;
                    __this->host_mic.o_t_id = o_t_id;
                    if (feature_desc && (feature_desc[4] == i_t_id)) {
                        __this->host_mic.mute_en = !!(feature_desc[6] & 0x01);
                        __this->host_mic.vol_en = !!(feature_desc[6] & 0x02);
                        __this->host_mic.mute_l_en = !!(feature_desc[6 + bControlSize * 1] & 0x01);
                        __this->host_mic.vol_l_en = !!(feature_desc[6 + bControlSize * 1] & 0x02);
                        if (feature_desc[0] == (7 + (2 + 1) * bControlSize)) {
                            __this->host_mic.mute_r_en = !!(feature_desc[6 + bControlSize * 2] & 0x01);
                            __this->host_mic.vol_r_en = !!(feature_desc[6 + bControlSize * 2] & 0x02);
                        }
                    }
                    break;
                default:
                    break;
                }
                log_debug("OUTPUT_TERMINAL: %d %d %x", o_t_id, src_id, terminal_type);
            }

            len += cur_len;
        }

        return sizeof(struct usb_interface_descriptor);
    }

    if (interface->bInterfaceSubClass == USB_SUBCLASS_AUDIOSTREAMING) {
        usb_if->dev.audio->subclass = interface->bInterfaceSubClass;
        usb_if->dev.audio->interface_num = interface_num;
        if (interface->bNumEndpoints == 0) {
            pBuf += sizeof(struct usb_interface_descriptor);
            do {
                struct usb_interface_descriptor *as_interface = (struct usb_interface_descriptor *)pBuf;
                if (as_interface->bNumEndpoints == 0 || as_interface->bInterfaceClass != USB_CLASS_AUDIO) {
                    break;
                }
                log_info("audio streaming interface : %d  ep_num:%d Altersetting:%d", interface_num, as_interface->bNumEndpoints, as_interface->bAlternateSetting);
                as_t = &usb_if->dev.audio->as[as_interface->bAlternateSetting - 1];
                as_t->bNumEndpoints = as_interface->bNumEndpoints;
                pBuf += (USB_DT_INTERFACE_SIZE + UAC_DT_AS_HEADER_SIZE);
                //解析format
                struct uac_format_type_i_discrete_descriptor *uac_format_desc = (struct uac_format_type_i_discrete_descriptor *)pBuf;
                if (uac_format_desc->bDescriptorSubtype == UAC_FORMAT_TYPE) {
                    as_t->bFormatType = uac_format_desc->bFormatType;
                    as_t->bNrChannels = uac_format_desc->bNrChannels;
                    as_t->bSubframeSize = uac_format_desc->bSubframeSize;
                    as_t->bBitResolution = uac_format_desc->bBitResolution;
                    as_t->bSamFreqType = uac_format_desc->bSamFreqType;
                    for (u8 i = 0; i < as_t->bSamFreqType; i++) {
                        memcpy(&as_t->tSamFreq[i], &uac_format_desc->tSamFreq[i], 3);
                        log_info("as bNrChannels:%d bBitResolution:%d  tSamFreq : %d", as_t->bNrChannels, as_t->bBitResolution, as_t->tSamFreq[i]);
                    }
                    //Endpointdescriptor
                    pBuf += uac_format_desc->bLength;
                    /* for (int i = 0; i < as_t->bNumEndpoints; i++) { */
                    struct usb_endpoint_descriptor *endpoint = (struct usb_endpoint_descriptor *)pBuf;
                    if (endpoint->bDescriptorType == USB_DT_ENDPOINT) {
                        as_t->ep_Interval = endpoint->bInterval;
                        as_t->ep_max_packet_size = endpoint->wMaxPacketSize;
                        if (endpoint->bEndpointAddress & USB_DIR_IN) {
                            as_t->ep = endpoint->bEndpointAddress & 0xf;
                            log_info("ep in : %x\n", as_t->ep);
                            usb_if->dev.audio->support = MICROPHONE_SUPPORTED;
                        } else {
                            as_t->ep = endpoint->bEndpointAddress;
                            log_info("ep out : %x\n", as_t->ep);
                            usb_if->dev.audio->support = HEADPHONE_SUPPORTED;
                        }
                        pBuf += USB_DT_ENDPOINT_AUDIO_SIZE;
                        pBuf += UAC_ISO_ENDPOINT_DESC_SIZE;
                    }

#if 0
                    struct uac_iso_endpoint_descriptor *iso_endpoint = (struct uac_iso_endpoint_descriptor *)pBuf;
                    if (iso_endpoint->bDescriptorType == USB_DT_CS_ENDPOINT) {
                        as_t->sr_ctrl_dis = !(iso_endpoint->bmAttributes & BIT(0));
                        pBuf += UAC_ISO_ENDPOINT_DESC_SIZE;
                    } else {
                        as_t->sr_ctrl_dis = 0;
                    }
#endif
                    /* } */
                } else {
                    log_error("uac_format_desc->bDescriptorSubtype err!!\n");
                    goto __exit;
                }

            } while (1);
            /* log_info("lennnn:%d\n",pBuf - (u8 *)interface); */
            return pBuf - (u8 *)interface;
        } else {
            log_info("audio streaming interface : %d  ep_num:%d Altersetting:%d\n", interface_num, interface->bNumEndpoints, interface->bAlternateSetting);
        }
    }

__exit:
    return USB_DT_INTERFACE_SIZE;
}

static struct audio_device_t *__find_microphone_interface(const struct usb_host_device *host_dev)
{
    struct audio_device_t *audio = NULL;

    for (u8 i = 0; i < MAX_HOST_INTERFACE; i++) {
        const struct usb_interface_info *usb_if = host_dev->interface_info[i];
        if (usb_if &&
            (usb_if->ctrl->interface_class == USB_CLASS_AUDIO)) {
            audio = usb_if->dev.audio;
            if (audio->subclass == USB_SUBCLASS_AUDIOSTREAMING &&
                audio->support == MICROPHONE_SUPPORTED) {
                // find host_spk
                return audio;
            }
        }
    }

    return NULL;
}

static struct audio_device_t *__find_headphone_interface(const struct usb_host_device *host_dev)
{
    struct audio_device_t *audio = NULL;

    for (u8 i = 0; i < MAX_HOST_INTERFACE; i++) {
        const struct usb_interface_info *usb_if = host_dev->interface_info[i];
        if (usb_if &&
            (usb_if->ctrl->interface_class == USB_CLASS_AUDIO)) {
            audio = usb_if->dev.audio;
            if (audio->subclass == USB_SUBCLASS_AUDIOSTREAMING &&
                audio->support == HEADPHONE_SUPPORTED) {
                // find headphone
                return audio;
            }
        }
    }

    return NULL;
}

static struct audio_device_t *__find_control_interface(const struct usb_host_device *host_dev)
{
    struct audio_device_t *audio = NULL;

    for (u8 i = 0; i < MAX_HOST_INTERFACE; i++) {
        const struct usb_interface_info *usb_if = host_dev->interface_info[i];
        if (usb_if &&
            (usb_if->ctrl->interface_class == USB_CLASS_AUDIO)) {
            audio = usb_if->dev.audio;
            if (audio->subclass == USB_SUBCLASS_AUDIOCONTROL) {
                return audio;
            }
        }
    }

    return NULL;
}

/*******************************************************************************************************/

static u16 host_mic_vol_convert(u16 v, u16 min, u16 max, u8 dir)
{
    u16 retval = 0;
    s16 reg1;
    s16 reg2;

    if (dir ==  0) {
        if (v > 100) {
            v = 100;
        }
        reg1 = my_abs((s16)max - (s16)min);
        retval = (s16)min + (s16)(reg1 * v / 100);
    } else {
        if ((s16)v > (s16)max) {
            v = max;
        }
        if ((s16)v < (s16)min) {
            v = min;
        }
        reg1 = my_abs((s16)max - (s16)min);
        reg2 = my_abs((s16)v - (s16)min);
        retval = reg2 * 100 / reg1;
    }

    return retval;
}

static s32 __usb_host_mic_get_mute(const usb_dev usb_id)
{
    struct usb_host_device *host_dev = (struct usb_host_device *)host_id2device(usb_id);
    struct audio_device_t *audio = __find_control_interface(host_dev);
    if (!audio) {
        log_error("no find control interface!");
        return -1;
    }

    s32 ret;
    u8 mute = 0;

    if (__this->host_mic.mute_en) {
        mute = 0;
        ret = usb_audio_mute_control_get_cur(host_dev, __this->host_mic.f_u_id, 0, &mute, audio->interface_num);
        retval_check(ret, 0);
        if (mute) {
            __this->host_mic.mute = 1;
            log_debug("host mic mic is mute");
        } else {
            __this->host_mic.mute = 0;
            log_debug("host mic mic is unmute");
        }
    }
    if (__this->host_mic.mute_l_en) {
        mute = 0;
        ret = usb_audio_mute_control_get_cur(host_dev, __this->host_mic.f_u_id, 1, &mute, audio->interface_num);
        retval_check(ret, 0);
        if (mute) {
            __this->host_mic.mute_l = 1;
            log_debug("host mic mic_l is mute");
        } else {
            __this->host_mic.mute_l = 0;
            log_debug("host mic mic_l is unmute");
        }
    }
    if (__this->host_mic.mute_r_en) {
        mute = 0;
        ret = usb_audio_mute_control_get_cur(host_dev, __this->host_mic.f_u_id, 2, &mute, audio->interface_num);
        retval_check(ret, 0);
        if (mute) {
            __this->host_mic.mute_r = 1;
            log_debug("host mic mic_r is mute");
        } else {
            __this->host_mic.mute_r = 0;
            log_debug("host mic mic_r is unmute");
        }
    }

__exit_fail:
    return ret;
}

static s32 __usb_host_mic_set_mute(const usb_dev usb_id)
{
    struct usb_host_device *host_dev = (struct usb_host_device *)host_id2device(usb_id);
    struct audio_device_t *audio = __find_control_interface(host_dev);
    if (!audio) {
        log_error("no find control interface!");
        return -1;
    }

    s32 ret = 0;
    u8 mute = 0;

    if (__this->host_mic.mute_en) {
        mute = __this->host_mic.mute;
        ret = usb_audio_mute_control_set_cur(host_dev, __this->host_mic.f_u_id, 0, mute, audio->interface_num);
        retval_check(ret, 0);
        log_debug("host mic set mute to %d", mute);
    }
    if (__this->host_mic.mute_l_en) {
        mute = __this->host_mic.mute_l;
        ret = usb_audio_mute_control_set_cur(host_dev, __this->host_mic.f_u_id, 1, mute, audio->interface_num);
        retval_check(ret, 0);
        log_debug("host mic set mute_l to %d", mute);
    }
    if (__this->host_mic.mute_r_en) {
        mute = __this->host_mic.mute_r;
        ret = usb_audio_mute_control_set_cur(host_dev, __this->host_mic.f_u_id, 2, mute, audio->interface_num);
        retval_check(ret, 0);
        log_debug("host mic set mute_r to %d", mute);
    }

__exit_fail:
    return ret;
}

static s32 __usb_host_mic_get_all_volume(const usb_dev usb_id)
{
    struct usb_host_device *host_dev = (struct usb_host_device *)host_id2device(usb_id);
    struct audio_device_t *audio = __find_control_interface(host_dev);
    if (!audio) {
        log_error("no find control interface!");
        return -1;
    }

    s32 ret = 0;

    if (__this->host_mic.vol_en) {
        //GET CUR
        ret = usb_audio_volume_control_get_cur(host_dev, __this->host_mic.f_u_id, 0, &__this->host_mic.vol_cur, audio->interface_num);
        retval_check(ret, 0);
        //GET MIN
        ret = usb_audio_volume_control_get_min(host_dev, __this->host_mic.f_u_id, 0, &__this->host_mic.vol_min, audio->interface_num);
        retval_check(ret, 0);
        //GET MAX
        ret = usb_audio_volume_control_get_max(host_dev, __this->host_mic.f_u_id, 0, &__this->host_mic.vol_max, audio->interface_num);
        retval_check(ret, 0);
        //GET RES
        ret = usb_audio_volume_control_get_res(host_dev, __this->host_mic.f_u_id, 0, &__this->host_mic.vol_res, audio->interface_num);
        retval_check(ret, 0);

        log_debug("host mic vol min %04x, vol max %04x, vol res %04x, vol cur %04x",
                  __this->host_mic.vol_min,
                  __this->host_mic.vol_max,
                  __this->host_mic.vol_res,
                  __this->host_mic.vol_cur);
    }
    if (__this->host_mic.vol_l_en) {
        //GET CUR
        ret = usb_audio_volume_control_get_cur(host_dev, __this->host_mic.f_u_id, 1, &__this->host_mic.vol_l_cur, audio->interface_num);
        retval_check(ret, 0);
        //GET MIN
        ret = usb_audio_volume_control_get_min(host_dev, __this->host_mic.f_u_id, 1, &__this->host_mic.vol_l_min, audio->interface_num);
        retval_check(ret, 0);
        //GET MAX
        ret = usb_audio_volume_control_get_max(host_dev, __this->host_mic.f_u_id, 1, &__this->host_mic.vol_l_max, audio->interface_num);
        retval_check(ret, 0);
        //GET RES
        ret = usb_audio_volume_control_get_res(host_dev, __this->host_mic.f_u_id, 1, &__this->host_mic.vol_l_res, audio->interface_num);
        retval_check(ret, 0);

        log_debug("host mic vol min %04x, vol max %04x, vol res %04x, vol cur %04x",
                  __this->host_mic.vol_l_min,
                  __this->host_mic.vol_l_max,
                  __this->host_mic.vol_l_res,
                  __this->host_mic.vol_l_cur);
    }
    if (__this->host_mic.vol_r_en) {
        //GET CUR
        ret = usb_audio_volume_control_get_cur(host_dev, __this->host_mic.f_u_id, 2, &__this->host_mic.vol_r_cur, audio->interface_num);
        retval_check(ret, 0);
        //GET MIN
        ret = usb_audio_volume_control_get_min(host_dev, __this->host_mic.f_u_id, 2, &__this->host_mic.vol_r_min, audio->interface_num);
        retval_check(ret, 0);
        //GET MAX
        ret = usb_audio_volume_control_get_max(host_dev, __this->host_mic.f_u_id, 2, &__this->host_mic.vol_r_max, audio->interface_num);
        retval_check(ret, 0);
        //GET RES
        ret = usb_audio_volume_control_get_res(host_dev, __this->host_mic.f_u_id, 2, &__this->host_mic.vol_r_res, audio->interface_num);
        retval_check(ret, 0);

        log_debug("host mic vol min %04x, vol max %04x, vol res %04x, vol cur %04x",
                  __this->host_mic.vol_r_min,
                  __this->host_mic.vol_r_max,
                  __this->host_mic.vol_r_res,
                  __this->host_mic.vol_r_cur);
    }

__exit_fail:
    return ret;
}

static s32 __usb_host_mic_get_cur_volume(const usb_dev usb_id)
{
    struct usb_host_device *host_dev = (struct usb_host_device *)host_id2device(usb_id);
    struct audio_device_t *audio = __find_control_interface(host_dev);
    if (!audio) {
        log_error("no find control interface!");
        return -1;
    }

    s32 ret = 0;

    if (__this->host_mic.vol_en) {
        //GET CUR
        ret = usb_audio_volume_control_get_cur(host_dev, __this->host_mic.f_u_id, 0, &__this->host_mic.vol_cur, audio->interface_num);
        retval_check(ret, 0);

        log_debug("host mic get vol cur %04x", __this->host_mic.vol_cur);
    }
    if (__this->host_mic.vol_l_en) {
        //GET CUR
        ret = usb_audio_volume_control_get_cur(host_dev, __this->host_mic.f_u_id, 1, &__this->host_mic.vol_l_cur, audio->interface_num);
        retval_check(ret, 0);

        log_debug("host mic get vol_l cur %04x", __this->host_mic.vol_l_cur);
    }
    if (__this->host_mic.vol_r_en) {
        //GET CUR
        ret = usb_audio_volume_control_get_cur(host_dev, __this->host_mic.f_u_id, 2, &__this->host_mic.vol_r_cur, audio->interface_num);
        retval_check(ret, 0);

        log_debug("host mic get vol_r cur %04x", __this->host_mic.vol_r_cur);
    }

__exit_fail:
    return ret;
}

static s32 __usb_host_mic_set_cur_volume(const usb_dev usb_id)
{
    struct usb_host_device *host_dev = (struct usb_host_device *)host_id2device(usb_id);
    struct audio_device_t *audio = __find_control_interface(host_dev);
    if (!audio) {
        log_error("no find control interface!");
        return -1;
    }

    s32 ret = 0;

    if (__this->host_mic.vol_en) {
        //SET CUR
        ret = usb_audio_volume_control_set_cur(host_dev, __this->host_mic.f_u_id, 0, __this->host_mic.vol_cur, audio->interface_num);
        retval_check(ret, 0);

        log_debug("host mic set vol to 0x%04x", __this->host_mic.vol_cur);
    }
    if (__this->host_mic.vol_l_en) {
        //SET CUR
        ret = usb_audio_volume_control_set_cur(host_dev, __this->host_mic.f_u_id, 1, __this->host_mic.vol_l_cur, audio->interface_num);
        retval_check(ret, 0);

        log_debug("host mic set vol_l to 0x%04x", __this->host_mic.vol_l_cur);
    }
    if (__this->host_mic.vol_r_en) {
        //SET CUR
        ret = usb_audio_volume_control_set_cur(host_dev, __this->host_mic.f_u_id, 2, __this->host_mic.vol_r_cur, audio->interface_num);
        retval_check(ret, 0);

        log_debug("host mic set vol_r to 0x%04x", __this->host_mic.vol_r_cur);
    }

__exit_fail:
    return ret;
}

int usb_host_mic_set_volume(const usb_dev usb_id, u16 vol_l, u16 vol_r)
{
    u32 vol = 0;

    if (__this->host_mic.vol_en) {
        vol = (vol_l + vol_r) / 2;
        __this->host_mic.vol_cur = host_mic_vol_convert(vol, __this->host_mic.vol_min, __this->host_mic.vol_max, 0);
    }
    if (__this->host_mic.vol_l_en) {
        __this->host_mic.vol_l_cur = host_mic_vol_convert(vol_l, __this->host_mic.vol_l_min, __this->host_mic.vol_l_max, 0);
    }
    if (__this->host_mic.vol_r_en) {
        __this->host_mic.vol_r_cur = host_mic_vol_convert(vol_r, __this->host_mic.vol_r_min, __this->host_mic.vol_r_max, 0);
    }
    return __usb_host_mic_set_cur_volume(usb_id);
}

int usb_host_mic_set_mute(const usb_dev usb_id, u8 mute)
{
    mute = !!mute;

    log_debug("host mic set mute %d", mute);

    if (__this->host_mic.mute_en) {
        __this->host_mic.mute = mute;
    }
    if (__this->host_mic.mute_l_en) {
        __this->host_mic.mute_l = mute;
    }
    if (__this->host_mic.mute_r_en) {
        __this->host_mic.mute_r = mute;
    }
    return __usb_host_mic_set_mute(usb_id);
}

#ifdef USB_AUDIO_PLAY_TEST
static const s16 sin_48k[] = {
    0, 2139, 4240, 6270, 8192, 9974, 11585, 12998,
    14189, 15137, 15826, 16244, 16384, 16244, 15826, 15137,
    14189, 12998, 11585, 9974, 8192, 6270, 4240, 2139,
    0, -2139, -4240, -6270, -8192, -9974, -11585, -12998,
    -14189, -15137, -15826, -16244, -16384, -16244, -15826, -15137,
    -14189, -12998, -11585, -9974, -8192, -6270, -4240, -2139
};
#endif

static u32 usb_audio_tx_len[USB_MAX_HW_NUM] = {0};

static void usb_audio_tx_isr(struct usb_host_device *host_dev, u32 ep)
{
    const usb_dev usb_id = host_device2id(host_dev);
    struct audio_device_t *audio = NULL;
    struct audio_streaming_t *as_t = NULL;
    u16 ep_max_packet_size = 0;
    u8 channel = 0;
    u32 rlen = 0;

    if (__this->host_mic.mic_state != AUDIO_MIC_START) {
        return;
    }

    audio = __find_headphone_interface(host_dev);
    if (!audio) {
        log_error("no find headphone interface!");
        return;
    }

    as_t = &audio->as[__this->host_mic.Cur_AlternateSetting - 1];
    ep_max_packet_size = as_t->ep_max_packet_size;
    channel = as_t->bNrChannels;

    //iso send
#ifdef USB_AUDIO_PLAY_TEST
    //For Test
    int tx_len = 0;
#if 1 // 单声道双声道输出
    s16 buf[240 / 2];
    for (u8 i = 0, j = 0; i < 240 / 2; i += 2) {
        buf[i] = sin_48k[j];
        buf[i + 1] = sin_48k[j];
        j++;
        if (j >= sizeof(sin_48k) / sizeof(sin_48k[0])) {
            j = 0;
        }
    }
#else
    //单声道直接输出
    u8 buf[248];
    do {
        memcpy(&buf[tx_len], sin_48k, sizeof(sin_48k));
        tx_len += sizeof(sin_48k);
    } while (tx_len < ep_max_packet_size);
#endif
    usb_h_ep_write_async(usb_id, ep, ep_max_packet_size, as_t->ep, buf, ep_max_packet_size, USB_ENDPOINT_XFER_ISOC, 0);
#else
    if (__this->host_mic.usb_audio_remain_len == 0) {
        cbuf_read_alloc(&__this->host_mic.usb_audio_record_cbuf, &__this->host_mic.usb_audio_remain_len);
        usb_audio_tx_len[usb_id] = 0;
    }
    if (__this->host_mic.usb_audio_remain_len) {
        if (usb_audio_tx_len[usb_id] == 0) {
            rlen = cbuf_read(&__this->host_mic.usb_audio_record_cbuf, __this->host_mic.usb_audio_mic_buf2, __this->host_mic.usb_audio_remain_len);
            if (!rlen) {
                __this->host_mic.usb_audio_remain_len = 0;
                usb_audio_tx_len[usb_id] = 0;
                putchar('c');
                usb_h_ep_write_async(usb_id, ep, ep_max_packet_size, as_t->ep, NULL, ep_max_packet_size, USB_ENDPOINT_XFER_ISOC, 1);
                os_sem_set(&__this->host_mic.sem, 0);
                os_sem_post(&__this->host_mic.sem);
                return;
            }
            os_sem_set(&__this->host_mic.sem, 0);
            os_sem_post(&__this->host_mic.sem);
        }
        u8 *send_buf = __this->host_mic.send_buf;
        u8 *play_buf = __this->host_mic.usb_audio_mic_buf2;
        if (channel == 2) {
            if (__this->host_mic.src_channel == 1) {
                //源数据是单声道数据,转双声道输出
                int j = 0;
                for (u32 i = 0; i < ep_max_packet_size; i += 4) {
                    //left
                    *(send_buf + i) = *(play_buf + (usb_audio_tx_len[usb_id] + j));
                    *(send_buf + i + 1) = *(play_buf + (usb_audio_tx_len[usb_id] + j + 1));
                    //right

                    *(send_buf + i + 2) = *(play_buf + (usb_audio_tx_len[usb_id] + j));
                    *(send_buf + i + 3) = *(play_buf + (usb_audio_tx_len[usb_id] + j + 1));
                    j += 2;
                }
                usb_audio_tx_len[usb_id] += j;
                usb_h_ep_write_async(usb_id, ep, ep_max_packet_size, as_t->ep, send_buf, ep_max_packet_size, USB_ENDPOINT_XFER_ISOC, 0);
            } else if (__this->host_mic.src_channel == 2) {
                //源数据是双声道数据,直接双声道输出
                usb_h_ep_write_async(usb_id, ep, ep_max_packet_size, as_t->ep, play_buf + usb_audio_tx_len[usb_id], ep_max_packet_size, USB_ENDPOINT_XFER_ISOC, 0);
                usb_audio_tx_len[usb_id] += ep_max_packet_size;
            }
        } else if (channel == 1) {
            if (__this->host_mic.src_channel == 1) {
                //源数据是单声道数据,直接单声道输出
                usb_h_ep_write_async(usb_id, ep, ep_max_packet_size, as_t->ep, play_buf + usb_audio_tx_len[usb_id], ep_max_packet_size, USB_ENDPOINT_XFER_ISOC, 0);
                usb_audio_tx_len[usb_id] += ep_max_packet_size;
            } else if (__this->host_mic.src_channel == 2) {
                //源数据是双声道数据,转单声道输出
                int j = 0;
                for (u32 i = 0; i < ep_max_packet_size; i += 2) {
                    //left
                    *(send_buf + i) = *(play_buf + (usb_audio_tx_len[usb_id] + j));
                    *(send_buf + i + 1) = *(play_buf + (usb_audio_tx_len[usb_id] + j + 1));
                    j += 4;
                }
                usb_audio_tx_len[usb_id] += j;
                usb_h_ep_write_async(usb_id, ep, ep_max_packet_size, as_t->ep, send_buf, ep_max_packet_size, USB_ENDPOINT_XFER_ISOC, 0);
            }
        }
        if (usb_audio_tx_len[usb_id] >= __this->host_mic.usb_audio_remain_len) {
            __this->host_mic.usb_audio_remain_len = 0;
            usb_audio_tx_len[usb_id] = 0;
        }
    } else {
        //audio buf null ,send null packet
        os_sem_set(&__this->host_mic.sem, 0);
        os_sem_post(&__this->host_mic.sem);
        usb_h_ep_write_async(usb_id, ep, ep_max_packet_size, as_t->ep, NULL, ep_max_packet_size, USB_ENDPOINT_XFER_ISOC, 1);
        putchar('d');
    }
#endif
}

static void audio_recorder_task(void *p)
{
    log_info(">>> Enter usb audio record task");
    struct usb_host_device *host_dev = (struct usb_host_device *)p;
    const usb_dev usb_id = host_device2id(host_dev);
    u8 *ptr = NULL;
    u32 wlen = 0;
    u32 ret = 0;
    struct audio_device_t *audio = __find_headphone_interface(host_dev);
    struct audio_streaming_t *as_t = &audio->as[__this->host_mic.Cur_AlternateSetting - 1];
    u32 ep_max_packet_size = as_t->ep_max_packet_size;
    u32 usb_audio_buf_size = ep_max_packet_size * 10; //预留10个包的缓存

    log_info("audio play ep max packet : %d\n", ep_max_packet_size);

    if (__this->host_mic.src_channel != as_t->bNrChannels && __this->host_mic.src_channel == 1) {
        usb_audio_buf_size >>= 1;
    }

    os_sem_create(&__this->host_mic.sem, 0);

    //分配双缓存
    // 一个缓存保存读卡的数据,一个用于usb发送
    if (!__this->host_mic.usb_audio_mic_buf2) {
        __this->host_mic.usb_audio_mic_buf2 = zalloc(usb_audio_buf_size);
    }
    if (!__this->host_mic.send_buf) {
        __this->host_mic.send_buf = zalloc(ep_max_packet_size);
    }
    if (!__this->host_mic.usb_audio_mic_buf) {
        __this->host_mic.usb_audio_mic_buf = zalloc(usb_audio_buf_size);
        cbuf_init(&__this->host_mic.usb_audio_record_cbuf, __this->host_mic.usb_audio_mic_buf, usb_audio_buf_size);
    }

    if (!__this->host_mic.send_buf || !__this->host_mic.usb_audio_mic_buf || !__this->host_mic.usb_audio_mic_buf2) {
        os_sem_pend(&__this->host_mic.sem, 0);
    }

    __this->host_mic.mic_state = AUDIO_MIC_START;

    usb_h_ep_write_async(usb_id, as_t->host_ep, ep_max_packet_size, as_t->ep, NULL, ep_max_packet_size, USB_ENDPOINT_XFER_ISOC, 1); //启动iso传输

    os_sem_post(__this->wait_sem);

    while (1) {
        if (__this->host_mic.Cur_AlternateSetting == 0 || __this->host_mic.mic_state != AUDIO_MIC_START) {
            os_time_dly(50);
            continue;
        }
        ptr = cbuf_write_alloc(&__this->host_mic.usb_audio_record_cbuf, &wlen);
        if (wlen) {
            /* putchar('a'); */
            if (__this->host_mic.get_buf) {
                ret = __this->host_mic.get_buf(usb_id, ptr, wlen, __this->host_mic.src_channel, __this->host_mic.sample_rate);
                if (ret != wlen) {
                    goto __task_exit;
                }
            }
            cbuf_write_updata(&__this->host_mic.usb_audio_record_cbuf, wlen);
        } else {
            log_w("usb audio record buf not enough!\n");
        }
__task_exit:
        os_sem_pend(&__this->host_mic.sem, 0);
    }
}

void usb_audio_start_recorder(const usb_dev usb_id, u8 channel, u8 bit_reso, u32 sample_rate)
{
    struct usb_host_device *host_dev = (struct usb_host_device *)host_id2device(usb_id);
    struct audio_device_t *audio = NULL;
    struct audio_streaming_t *as_t = NULL;
    u8 find_alternatesetting = 0;

    audio = __find_headphone_interface(host_dev);
    if (!audio) {
        log_error("no find headphone interface!");
        return;
    }
    for (u8 i = 0; i < ARRAY_SIZE(audio->as); i++) {
        as_t = &audio->as[i];
        if (as_t->bBitResolution == bit_reso) {
            for (u8 j = 0; j < as_t->bSamFreqType; j++) {
                if (sample_rate == 0 && as_t->tSamFreq[as_t->bSamFreqType - j - 1] && as_t->tSamFreq[as_t->bSamFreqType - j - 1] <= 48000) {
                    sample_rate = as_t->tSamFreq[as_t->bSamFreqType - j - 1];
                    find_alternatesetting = i + 1;
                    break;
                }
                if (as_t->tSamFreq[as_t->bSamFreqType - j - 1] == sample_rate) {
                    find_alternatesetting = i + 1;
                    break;
                }
            }
        }
    }
    if (!find_alternatesetting) {
        log_e("can not find Alternatesetting,please check bit_reso and sample_rate\n");
        return;
    }

    as_t = &audio->as[find_alternatesetting - 1];

    u32 ep_max_packet_size = sample_rate * 2 * as_t->bNrChannels / 1000;

    if (ep_max_packet_size > as_t->ep_max_packet_size) {
        ep_max_packet_size = as_t->ep_max_packet_size;
    } else {
        as_t->ep_max_packet_size = ep_max_packet_size;
    }

    //端点分配
    u32 host_ep = usb_get_ep_num(usb_id, USB_DIR_OUT, USB_ENDPOINT_XFER_ISOC);
    ASSERT(host_ep != -1, "ep not enough");
    ep_out_dma_buf[usb_id] = usb_h_alloc_ep_buffer(usb_id, host_ep, as_t->ep_max_packet_size + 64);  //预留64个字节防止硬件越界访问
    ASSERT(ep_out_dma_buf[usb_id] != NULL);
    as_t->host_ep = host_ep;

    if (channel == 0) {
        channel = as_t->bNrChannels;
    }

    __this->host_mic.Cur_AlternateSetting = find_alternatesetting; //选择Alternatesetting
    __this->host_mic.sample_rate = sample_rate;   //选择采样率
    __this->host_mic.src_channel = channel;

    usb_set_interface(host_dev, audio->interface_num, find_alternatesetting); //interface   Alternatesetting
    if (!as_t->sr_ctrl_dis) {
        usb_audio_sampling_frequency_control(host_dev, as_t->ep, sample_rate);//设置采样率
    }

    log_info("H2D ep: %x --> %x  interval: %d", host_ep, as_t->ep, as_t->ep_Interval);
    usb_write_txfuncaddr(usb_id, host_ep, host_dev->private_data.devnum);
    usb_h_set_ep_isr(host_dev, host_ep, usb_audio_tx_isr, host_dev);
    usb_h_ep_config(usb_id, host_ep, USB_ENDPOINT_XFER_ISOC, 1, as_t->ep_Interval, ep_out_dma_buf[usb_id], as_t->ep_max_packet_size);

    __usb_host_mic_get_all_volume(usb_id);
    usb_host_mic_set_mute(usb_id, 0);
    usb_host_mic_set_volume(usb_id, HOST_MIC_VOLUME, HOST_MIC_VOLUME);

    OS_SEM sem;
    os_sem_create(&sem, 0);
    __this->wait_sem = &sem;

    if (usb_id == 0) {
        task_create(audio_recorder_task, host_dev, "uac_record0");
    } else {
        task_create(audio_recorder_task, host_dev, "uac_record1");
    }

    os_sem_pend(&sem, 0);
    __this->wait_sem = NULL;
}

void usb_audio_stop_recorder(const usb_dev usb_id)
{
    __this->host_mic.mic_state = AUDIO_MIC_STOP;
    usb_h_set_ep_isr(NULL, 0, NULL, NULL);
    if (__this->host_mic.get_buf) {
        __this->host_mic.get_buf(usb_id, NULL, 0, 0, 0);
    }
    __this->host_mic.Cur_AlternateSetting = 0;
    if (usb_id == 0) {
        task_kill("uac_record0");
    } else {
        task_kill("uac_record1");
    }
    __this->host_mic.sample_rate = 0;
    __this->host_mic.src_channel = 0;
    if (__this->host_mic.usb_audio_mic_buf) {
        free(__this->host_mic.usb_audio_mic_buf);
        __this->host_mic.usb_audio_mic_buf = NULL;
    }
    if (__this->host_mic.usb_audio_mic_buf2) {
        free(__this->host_mic.usb_audio_mic_buf2);
        __this->host_mic.usb_audio_mic_buf2 = NULL;
    }
    if (__this->host_mic.send_buf) {
        free(__this->host_mic.send_buf);
        __this->host_mic.send_buf = NULL;
    }
    if (ep_out_dma_buf[usb_id]) {
        usb_h_free_ep_buffer(usb_id, ep_out_dma_buf[usb_id]);
        ep_out_dma_buf[usb_id] = NULL;
    }
}

void usb_audio_pause_recorder(const usb_dev usb_id)
{
    __this->host_mic.mic_state = AUDIO_MIC_PAUSE;
}

void usb_audio_resume_recorder(const usb_dev usb_id)
{
    const struct usb_host_device *host_dev = host_id2device(usb_id);
    struct audio_device_t *audio = __find_headphone_interface(host_dev);
    struct audio_streaming_t *as_t = &audio->as[__this->host_mic.Cur_AlternateSetting - 1];

    if (__this->host_mic.usb_audio_mic_buf) {
        __this->host_mic.usb_audio_remain_len = 0;
        usb_audio_tx_len[usb_id] = 0;
        cbuf_clear(&__this->host_mic.usb_audio_record_cbuf);
        __this->host_mic.mic_state = AUDIO_MIC_START;
        usb_h_ep_write_async(usb_id, as_t->host_ep, as_t->ep_max_packet_size, as_t->ep, NULL, as_t->ep_max_packet_size, USB_ENDPOINT_XFER_ISOC, 1); //重新启动传输
    }
}

/******************************************************************************************************/

static u16 host_speaker_vol_convert(u16 v, u16 min, u16 max, u8 dir)
{
    u16 retval = 0;
    s16 reg1;
    s16 reg2;

    if (dir == 0) {
        if (v > 100) {
            v = 100;
        }
        reg1 = my_abs((s16)max - (s16)min);
        retval = (s16)min + (s16)(reg1 * v / 100);
    } else {
        if ((s16)v > (s16)max) {
            v = max;
        }
        if ((s16)v < (s16)min) {
            v = min;
        }
        reg1 = my_abs((s16)max - (s16)min);
        reg2 = my_abs((s16)v - (s16)min);
        retval = reg2 * 100 / reg1;
    }

    return retval;
}

static s32 __usb_host_speaker_get_mute(const usb_dev usb_id)
{
    struct usb_host_device *host_dev = (struct usb_host_device *)host_id2device(usb_id);
    struct audio_device_t *audio = __find_control_interface(host_dev);
    if (!audio) {
        log_error("no find control interface!");
        return -1;
    }

    s32 ret = 0;
    u8 mute = 0;

    if (__this->host_spk.mute_en) {
        mute = 0;
        ret = usb_audio_mute_control_get_cur(host_dev, __this->host_spk.f_u_id, 0, &mute, audio->interface_num);
        retval_check(ret, 0);
        if (mute) {
            __this->host_spk.mute = 1;
            log_debug("host spk spk is mute");
        } else {
            __this->host_spk.mute = 0;
            log_debug("host spk spk is unmute");
        }
    }
    if (__this->host_spk.mute_l_en) {
        mute = 0;
        ret = usb_audio_mute_control_get_cur(host_dev, __this->host_spk.f_u_id, 1, &mute, audio->interface_num);
        retval_check(ret, 0);
        if (mute) {
            __this->host_spk.mute_l = 1;
            log_debug("host spk spk_l is mute");
        } else {
            __this->host_spk.mute_l = 0;
            log_debug("host spk spk_l is unmute");
        }
    }
    if (__this->host_spk.mute_r_en) {
        mute = 0;
        ret = usb_audio_mute_control_get_cur(host_dev, __this->host_spk.f_u_id, 2, &mute, audio->interface_num);
        retval_check(ret, 0);
        if (mute) {
            __this->host_spk.mute_r = 1;
            log_debug("host spk spk_r is mute");
        } else {
            __this->host_spk.mute_r = 0;
            log_debug("host spk spk_r is unmute");
        }
    }

__exit_fail:
    return ret;
}

static s32 __usb_host_speaker_set_mute(const usb_dev usb_id)
{
    struct usb_host_device *host_dev = (struct usb_host_device *)host_id2device(usb_id);
    struct audio_device_t *audio = __find_control_interface(host_dev);
    if (!audio) {
        log_error("no find control interface!");
        return -1;
    }

    s32 ret = 0;
    u8 mute = 0;

    if (__this->host_spk.mute_en) {
        mute = __this->host_spk.mute;
        ret = usb_audio_mute_control_set_cur(host_dev, __this->host_spk.f_u_id, 0, mute, audio->interface_num);
        retval_check(ret, 0);
        log_debug("host spk set mute to %d", mute);
    }
    if (__this->host_spk.mute_l_en) {
        mute = __this->host_spk.mute_l;
        ret = usb_audio_mute_control_set_cur(host_dev, __this->host_spk.f_u_id, 1, mute, audio->interface_num);
        retval_check(ret, 0);
        log_debug("host spk set mute_l to %d", mute);
    }
    if (__this->host_spk.mute_r_en) {
        mute = __this->host_spk.mute_r;
        ret = usb_audio_mute_control_set_cur(host_dev, __this->host_spk.f_u_id, 2, mute, audio->interface_num);
        retval_check(ret, 0);
        log_debug("host spk set mute_r to %d", mute);
    }

__exit_fail:
    return ret;
}

static s32 __usb_host_speaker_get_all_volume(const usb_dev usb_id)
{
    struct usb_host_device *host_dev = (struct usb_host_device *)host_id2device(usb_id);
    struct audio_device_t *audio = __find_control_interface(host_dev);
    if (!audio) {
        log_error("no find control interface!");
        return -1;
    }

    s32 ret = 0;

    if (__this->host_spk.vol_en) {
        ret = usb_audio_volume_control_get_cur(host_dev, __this->host_spk.f_u_id, 0, &__this->host_spk.vol_cur, audio->interface_num);
        //GET CUR
        ret = usb_audio_volume_control_get_cur(host_dev, __this->host_spk.f_u_id, 0, &__this->host_spk.vol_cur, audio->interface_num);
        retval_check(ret, 0);
        //GET MIN
        ret = usb_audio_volume_control_get_min(host_dev, __this->host_spk.f_u_id, 0, &__this->host_spk.vol_min, audio->interface_num);
        retval_check(ret, 0);
        //GET MAX
        ret = usb_audio_volume_control_get_max(host_dev, __this->host_spk.f_u_id, 0, &__this->host_spk.vol_max, audio->interface_num);
        retval_check(ret, 0);
        //GET RES
        ret = usb_audio_volume_control_get_res(host_dev, __this->host_spk.f_u_id, 0, &__this->host_spk.vol_res, audio->interface_num);
        retval_check(ret, 0);

        log_debug("host spk vol min %04x, vol max %04x, vol res %04x, vol cur %04x",
                  __this->host_spk.vol_min,
                  __this->host_spk.vol_max,
                  __this->host_spk.vol_res,
                  __this->host_spk.vol_cur);
    }
    if (__this->host_spk.vol_l_en) {
        //GET CUR
        ret = usb_audio_volume_control_get_cur(host_dev, __this->host_spk.f_u_id, 1, &__this->host_spk.vol_l_cur, audio->interface_num);
        retval_check(ret, 0);
        //GET MIN
        ret = usb_audio_volume_control_get_min(host_dev, __this->host_spk.f_u_id, 1, &__this->host_spk.vol_l_min, audio->interface_num);
        retval_check(ret, 0);
        //GET MAX
        ret = usb_audio_volume_control_get_max(host_dev, __this->host_spk.f_u_id, 1, &__this->host_spk.vol_l_max, audio->interface_num);
        retval_check(ret, 0);
        //GET RES
        ret = usb_audio_volume_control_get_res(host_dev, __this->host_spk.f_u_id, 1, &__this->host_spk.vol_l_res, audio->interface_num);
        retval_check(ret, 0);

        log_debug("host spk voll min %04x, voll max %04x, voll res %04x, voll cur %04x",
                  __this->host_spk.vol_l_min,
                  __this->host_spk.vol_l_max,
                  __this->host_spk.vol_l_res,
                  __this->host_spk.vol_l_cur);
    }
    if (__this->host_spk.vol_r_en) {
        //GET CUR
        ret = usb_audio_volume_control_get_cur(host_dev, __this->host_spk.f_u_id, 2, &__this->host_spk.vol_r_cur, audio->interface_num);
        retval_check(ret, 0);
        //GET MIN
        ret = usb_audio_volume_control_get_min(host_dev, __this->host_spk.f_u_id, 2, &__this->host_spk.vol_r_min, audio->interface_num);
        retval_check(ret, 0);
        //GET MAX
        ret = usb_audio_volume_control_get_max(host_dev, __this->host_spk.f_u_id, 2, &__this->host_spk.vol_r_max, audio->interface_num);
        retval_check(ret, 0);
        //GET RES
        ret = usb_audio_volume_control_get_res(host_dev, __this->host_spk.f_u_id, 2, &__this->host_spk.vol_r_res, audio->interface_num);
        retval_check(ret, 0);

        log_debug("host spk volr min %04x, volr max %04x, volr res %04x, volr cur %04x",
                  __this->host_spk.vol_r_min,
                  __this->host_spk.vol_r_max,
                  __this->host_spk.vol_r_res,
                  __this->host_spk.vol_r_cur);
    }

__exit_fail:
    return ret;
}

static s32 __usb_host_speaker_get_cur_volume(const usb_dev usb_id)
{
    struct usb_host_device *host_dev = (struct usb_host_device *)host_id2device(usb_id);
    struct audio_device_t *audio = __find_control_interface(host_dev);
    if (!audio) {
        log_error("no find control interface!");
        return -1;
    }

    s32 ret = 0;

    if (__this->host_spk.vol_en) {
        //GET CUR
        ret = usb_audio_volume_control_get_cur(host_dev, __this->host_spk.f_u_id, 0, &__this->host_spk.vol_cur, audio->interface_num);
        retval_check(ret, 0);

        log_debug("host spk get vol_cur %04x", __this->host_spk.vol_cur);
    }
    if (__this->host_spk.vol_l_en) {
        //GET CUR
        ret = usb_audio_volume_control_get_cur(host_dev, __this->host_spk.f_u_id, 1, &__this->host_spk.vol_l_cur, audio->interface_num);
        retval_check(ret, 0);

        log_debug("host spk get vol_l_cur %04x", __this->host_spk.vol_l_cur);
    }
    if (__this->host_spk.vol_r_en) {
        //GET CUR
        ret = usb_audio_volume_control_get_cur(host_dev, __this->host_spk.f_u_id, 2, &__this->host_spk.vol_r_cur, audio->interface_num);
        retval_check(ret, 0);

        log_debug("host spk get vol_r_cur %04x", __this->host_spk.vol_r_cur);
    }

__exit_fail:
    return ret;
}

static s32 __usb_host_speaker_set_cur_volume(const usb_dev usb_id)
{
    struct usb_host_device *host_dev = (struct usb_host_device *)host_id2device(usb_id);
    struct audio_device_t *audio = __find_control_interface(host_dev);
    if (!audio) {
        log_error("no find control interface!");
        return -1;
    }

    s32 ret = 0;

    if (__this->host_spk.vol_en) {
        //SET CUR
        ret = usb_audio_volume_control_set_cur(host_dev, __this->host_spk.f_u_id, 0, __this->host_spk.vol_cur, audio->interface_num);
        retval_check(ret, 0);

        log_debug("host spk set vol to 0x%04x", __this->host_spk.vol_cur);
    }
    if (__this->host_spk.vol_l_en) {
        //SET CUR
        ret = usb_audio_volume_control_set_cur(host_dev, __this->host_spk.f_u_id, 1, __this->host_spk.vol_l_cur, audio->interface_num);
        retval_check(ret, 0);
        log_debug("host spk set vol_l to 0x%04x", __this->host_spk.vol_l_cur);
    }
    if (__this->host_spk.vol_r_en) {
        //SET CUR
        ret = usb_audio_volume_control_set_cur(host_dev, __this->host_spk.f_u_id, 2, __this->host_spk.vol_r_cur, audio->interface_num);
        retval_check(ret, 0);

        log_debug("host spk set vol_r to 0x%04x", __this->host_spk.vol_r_cur);
    }

__exit_fail:
    return ret;
}

int usb_host_speaker_set_volume(const usb_dev usb_id, u16 vol)
{
    if (__this->host_spk.vol_en) {
        __this->host_spk.vol_cur = host_speaker_vol_convert(vol, __this->host_spk.vol_min, __this->host_spk.vol_max, 0);
    }
    if (__this->host_spk.vol_l_en) {
        __this->host_spk.vol_l_cur = host_speaker_vol_convert(vol, __this->host_spk.vol_l_min, __this->host_spk.vol_l_max, 0);
    }
    if (__this->host_spk.vol_r_en) {
        __this->host_spk.vol_r_cur = host_speaker_vol_convert(vol, __this->host_spk.vol_r_min, __this->host_spk.vol_r_max, 0);
    }
    return __usb_host_speaker_set_cur_volume(usb_id);
}

int usb_host_speaker_set_mute(const usb_dev usb_id, u8 mute)
{
    mute = !!mute;

    log_debug("host spk set mute %d", mute);

    if (__this->host_spk.mute_en) {
        __this->host_spk.mute = mute;
    }
    if (__this->host_spk.mute_l_en) {
        __this->host_spk.mute_l = mute;
    }
    if (__this->host_spk.mute_r_en) {
        __this->host_spk.mute_r = mute;
    }
    return __usb_host_speaker_set_mute(usb_id);
}

static s32 __usb_slave_mic_get_agc(const usb_dev usb_id)
{
    struct usb_host_device *host_dev = (struct usb_host_device *)host_id2device(usb_id);
    struct audio_device_t *audio = __find_control_interface(host_dev);
    if (!audio) {
        log_error("no find control interface!");
        return -1;
    }

    int ret = 0;
    u8 agc;

    if (__this->host_spk.agc_en) {
        ret = usb_audio_agc_control_get_cur(host_dev, __this->host_spk.f_u_id, 0, &agc, audio->interface_num);
        retval_check(ret, 0);
        __this->host_spk.agc = agc;
    }
    if (__this->host_spk.agc_l_en) {
        ret = usb_audio_agc_control_get_cur(host_dev, __this->host_spk.f_u_id, 1, &agc, audio->interface_num);
        retval_check(ret, 0);
        __this->host_spk.agc_l = agc;
    }
    if (__this->host_spk.agc_r_en) {
        ret = usb_audio_agc_control_get_cur(host_dev, __this->host_spk.f_u_id, 2, &agc, audio->interface_num);
        retval_check(ret, 0);
        __this->host_spk.agc_r = agc;
    }

__exit_fail:
    return ret;
}

static s32 __usb_slave_mic_set_agc(const usb_dev usb_id)
{
    struct usb_host_device *host_dev = (struct usb_host_device *)host_id2device(usb_id);
    struct audio_device_t *audio = __find_control_interface(host_dev);
    if (!audio) {
        log_error("no find control interface!");
        return -1;
    }

    int ret = 0;
    u8 agc;

    if (__this->host_spk.agc_en) {
        agc = __this->host_spk.agc;
        ret = usb_audio_agc_control_set_cur(host_dev, __this->host_spk.f_u_id, 0, agc, audio->interface_num);
        retval_check(ret, 0);
    }
    if (__this->host_spk.agc_l_en) {
        agc = __this->host_spk.agc_l;
        ret = usb_audio_agc_control_set_cur(host_dev, __this->host_spk.f_u_id, 1, agc, audio->interface_num);
        retval_check(ret, 0);
    }
    if (__this->host_spk.agc_r_en) {
        agc = __this->host_spk.agc_r;
        ret = usb_audio_agc_control_set_cur(host_dev, __this->host_spk.f_u_id, 2, agc, audio->interface_num);
        retval_check(ret, 0);
    }

__exit_fail:
    return ret;
}

static u32 write_file_len[USB_MAX_HW_NUM];

static void usb_audio_rx_isr(struct usb_host_device *host_dev, u32 ep)
{
    u8 *ptr = NULL;
    u32 rlen, wlen = 0;
    usb_dev usb_id = host_device2id(host_dev);
    struct audio_device_t *audio = NULL;
    struct audio_streaming_t *as_t = NULL;

    if (__this->host_spk.spk_state != AUDIO_SPK_START) {
        return;
    }

    audio = __find_microphone_interface(host_dev);
    if (!audio) {
        log_error("no find microphone interface!");
        return;
    }
    as_t = &audio->as[__this->host_spk.Cur_AlternateSetting - 1];

    int rx_len = usb_h_ep_read_async(usb_id, ep, as_t->ep, __this->host_spk.buffer, as_t->ep_max_packet_size, USB_ENDPOINT_XFER_ISOC, 0);
    /* printf("rx len : %d %d\n", rx_len, ep); */
    if (rx_len > 0) {
        cbuf_write(&__this->host_spk.usb_audio_spk_cbuf, __this->host_spk.buffer, rx_len);
    }
    cbuf_write_alloc(&__this->host_spk.usb_audio_spk_cbuf, &wlen);

    if (wlen == 0) {
        if (write_file_len[usb_id]) {
            putchar('e');
        }
        /* [> printf("R:%d  W:%d\n", rx_len,wlen); <] */
        cbuf_read_alloc(&__this->host_spk.usb_audio_spk_cbuf, &rlen);
        cbuf_read(&__this->host_spk.usb_audio_spk_cbuf, __this->host_spk.usb_audio_spk_buf2, rlen);
        write_file_len[usb_id] = rlen;

        if (usb_id == 0) {
            os_taskq_post_msg("uac_play0", 2, 0x01, rlen);
        } else {
            os_taskq_post_msg("uac_play1", 2, 0x01, rlen);
        }
    }

    usb_h_ep_read_async(usb_id, ep, as_t->ep, NULL, 0, USB_ENDPOINT_XFER_ISOC, 1); //触发下一个接收中断
}

static void audio_player_task(void *p)
{
    log_info(">>> Enter usb audio player task");
    struct usb_host_device *host_dev = (struct usb_host_device *)p;
    const usb_dev usb_id = host_device2id(host_dev);
    u8 *ptr = NULL;
    u32 rlen = 0;
    u32 ret = 0;
    int msg[4];
    struct audio_device_t *audio = __find_microphone_interface(host_dev);
    struct audio_streaming_t *as_t = &audio->as[__this->host_spk.Cur_AlternateSetting - 1];
    u32 ep_max_packet_size = as_t->ep_max_packet_size;
    u32 usb_audio_buf_size = ep_max_packet_size * 10;

    log_info("ep max packet : %d\n", ep_max_packet_size);

    //分配双缓存
    // 一个缓存写卡的数据,一个用于usb接收
    if (!__this->host_spk.usb_audio_spk_buf) {
        __this->host_spk.usb_audio_spk_buf = zalloc(usb_audio_buf_size);
        cbuf_init(&__this->host_spk.usb_audio_spk_cbuf, __this->host_spk.usb_audio_spk_buf, usb_audio_buf_size);
    }
    if (!__this->host_spk.usb_audio_spk_buf2) {
        __this->host_spk.usb_audio_spk_buf2 = zalloc(usb_audio_buf_size);
    }
    if (!__this->host_spk.buffer) {
        __this->host_spk.buffer = malloc(as_t->ep_max_packet_size);
    }

    if (!__this->host_spk.usb_audio_spk_buf || !__this->host_spk.usb_audio_spk_buf2) {
        os_taskq_pend(NULL, msg, ARRAY_SIZE(msg));
    }

    __this->host_spk.spk_state = AUDIO_SPK_START;

    __this->host_spk.put_buf(usb_id, NULL, 0, __this->host_spk.src_channel == as_t->bNrChannels ? __this->host_spk.src_channel : __this->host_spk.src_channel | BIT(7), __this->host_spk.sample_rate);

    usb_h_ep_read_async(usb_id, as_t->host_ep, as_t->ep, NULL, 0, USB_ENDPOINT_XFER_ISOC, 1); //启动iso

    os_sem_post(__this->wait_sem);

    while (1) {
        ret = os_taskq_pend(NULL, msg, ARRAY_SIZE(msg));
        if (ret == OS_TASKQ) {
            switch (msg[1]) {
            case 0x01:
                ptr = __this->host_spk.usb_audio_spk_buf2;
                rlen = msg[2];
                /* putchar('b'); */
                if (__this->host_spk.put_buf) {
                    __this->host_spk.put_buf(usb_id, ptr, rlen, __this->host_spk.src_channel, __this->host_spk.sample_rate);
                }
                write_file_len[usb_id] = 0;
                break;
            }
        }
    }
}

void usb_audio_start_player(const usb_dev usb_id, u8 channel, u8 bit_reso, u32 sample_rate)
{
    struct usb_host_device *host_dev = (struct usb_host_device *)host_id2device(usb_id);
    struct audio_device_t *audio = NULL;
    struct audio_streaming_t *as_t = NULL;
    u8 find_alternatesetting = 0;

    audio = __find_microphone_interface(host_dev);
    if (!audio) {
        log_error("no find host_spk interface!");
        return;
    }
    for (u8 i = 0; i < ARRAY_SIZE(audio->as); i++) {
        as_t = &audio->as[i];
        if (as_t->bBitResolution == bit_reso) {
            for (u8 j = 0; j < as_t->bSamFreqType; j++) {
                if (sample_rate == 0 && as_t->tSamFreq[as_t->bSamFreqType - j - 1] && as_t->tSamFreq[as_t->bSamFreqType - j - 1] <= 48000) {
                    sample_rate = as_t->tSamFreq[as_t->bSamFreqType - j - 1];
                    find_alternatesetting = i + 1;
                    break;
                }
                if (as_t->tSamFreq[as_t->bSamFreqType - j - 1] == sample_rate) {
                    find_alternatesetting = i + 1;
                    break;
                }
            }
        }
    }
    if (!find_alternatesetting) {
        log_e("can not find Alternatesetting,please check bit_reso and sample_rate\n");
        return;
    }

    as_t = &audio->as[find_alternatesetting - 1];

    u32 ep_max_packet_size = sample_rate * 2 * as_t->bNrChannels / 1000;

    if (ep_max_packet_size > as_t->ep_max_packet_size) {
        ep_max_packet_size = as_t->ep_max_packet_size;
    } else {
        as_t->ep_max_packet_size = ep_max_packet_size;
    }

    //端点分配
    u32 host_ep = usb_get_ep_num(usb_id, USB_DIR_IN, USB_ENDPOINT_XFER_ISOC);
    ASSERT(host_ep != -1, "ep not enough");
    host_ep = host_ep | USB_DIR_IN;
    ep_in_dma_buf[usb_id] = usb_h_alloc_ep_buffer(usb_id, host_ep, as_t->ep_max_packet_size + 64);  //预留64个字节防止硬件越界访问
    ASSERT(ep_in_dma_buf[usb_id] != NULL);
    as_t->host_ep = host_ep & 0x0f;

    if (channel == 0) {
        channel = as_t->bNrChannels;
    }

    __this->host_spk.Cur_AlternateSetting = find_alternatesetting; //选择Alternatesetting
    __this->host_spk.sample_rate = sample_rate;   //选择采样率
    __this->host_spk.src_channel = channel;

    usb_set_interface(host_dev, audio->interface_num, find_alternatesetting); //interface 1  Alternatesetting 1
    if (!as_t->sr_ctrl_dis) {
        usb_audio_sampling_frequency_control(host_dev, as_t->ep | USB_DIR_IN, sample_rate);//设置采样率
    }

    log_info("D2H ep: %x --> %x  interval: %d", as_t->ep, host_ep, as_t->ep_Interval);
    usb_write_rxfuncaddr(usb_id, as_t->host_ep, host_dev->private_data.devnum);
    usb_h_set_ep_isr(host_dev, host_ep, usb_audio_rx_isr, host_dev);
    usb_h_ep_config(usb_id, host_ep, USB_ENDPOINT_XFER_ISOC, 1, as_t->ep_Interval, ep_in_dma_buf[usb_id], as_t->ep_max_packet_size);

    __usb_host_speaker_get_all_volume(usb_id);
    usb_host_speaker_set_mute(usb_id, 0);
    usb_host_speaker_set_volume(usb_id, HOST_SPK_VOLUME);

    /* user_host_speaker_control(host_dev); */ //与AC53通信开启dac

    OS_SEM sem;
    os_sem_create(&sem, 0);
    __this->wait_sem = &sem;

    if (usb_id == 0) {
        task_create(audio_player_task, host_dev, "uac_play0");
    } else {
        task_create(audio_player_task, host_dev, "uac_play1");
    }

    os_sem_pend(&sem, 0);
    __this->wait_sem = NULL;
}

void usb_audio_stop_player(const usb_dev usb_id)
{
    __this->host_spk.spk_state = AUDIO_SPK_STOP;
    usb_h_set_ep_isr(NULL, 0, NULL, NULL);
    if (usb_id == 0) {
        task_kill("uac_play0");
    } else {
        task_kill("uac_play1");
    }
    if (__this->host_spk.put_buf) {
        __this->host_spk.put_buf(usb_id, NULL, 0, 0, 0);
    }
    __this->host_spk.Cur_AlternateSetting = 0;
    __this->host_spk.sample_rate = 0;
    if (__this->host_spk.usb_audio_spk_buf) {
        free(__this->host_spk.usb_audio_spk_buf);
        __this->host_spk.usb_audio_spk_buf = NULL;
    }
    if (__this->host_spk.usb_audio_spk_buf2) {
        free(__this->host_spk.usb_audio_spk_buf2);
        __this->host_spk.usb_audio_spk_buf2 = NULL;
    }
    if (__this->host_spk.buffer) {
        free(__this->host_spk.buffer);
        __this->host_spk.buffer = NULL;
    }
    if (ep_in_dma_buf[usb_id]) {
        usb_h_free_ep_buffer(usb_id, ep_in_dma_buf[usb_id]);
        ep_in_dma_buf[usb_id] = NULL;
    }
}

void usb_audio_pause_player(const usb_dev usb_id)
{
    __this->host_spk.spk_state = AUDIO_SPK_PAUSE;
}

void usb_audio_resume_player(const usb_dev usb_id)
{
    const struct usb_host_device *host_dev = host_id2device(usb_id);
    struct audio_device_t *audio = __find_microphone_interface(host_dev);
    struct audio_streaming_t *as_t = &audio->as[__this->host_spk.Cur_AlternateSetting - 1];

    if (__this->host_spk.usb_audio_spk_buf) {
        cbuf_clear(&__this->host_spk.usb_audio_spk_cbuf);
        write_file_len[usb_id] = 0;
        __this->host_spk.spk_state = AUDIO_SPK_START;
        usb_h_ep_read_async(usb_id, as_t->host_ep, as_t->ep, NULL, 0, USB_ENDPOINT_XFER_ISOC, 1); //重新启动接收
    }
}

void usb_audio_start_process(const usb_dev usb_id)
{
    usb_audio_start_recorder(usb_id, HOST_MIC_CHANNEL, HOST_MIC_AUDIO_RES, HOST_MIC_AUDIO_RATE); //开启主机的host_spk
    usb_audio_start_player(usb_id, HOST_SPK_CHANNEL, HOST_SPK_AUDIO_RES, HOST_SPK_AUDIO_RATE); //开启主机的speaker
}

void usb_audio_stop_process(const usb_dev usb_id)
{
    usb_audio_stop_recorder(usb_id);
    usb_audio_stop_player(usb_id);
}

void usb_host_audio_init(const usb_dev usb_id, int (*put_buf)(const usb_dev usb_id, void *ptr, u32 len, u8 channel, u32 sample_rate), int (*get_buf)(const usb_dev usb_id, void *ptr, u32 len, u8 channel, u32 sample_rate))
{
    memset(__this, 0, sizeof(struct usb_audio_info));
    __this->host_mic.get_buf = get_buf;
    __this->host_spk.put_buf = put_buf;
}

void usb_host_audio_exit(const usb_dev usb_id)
{
    __this->host_mic.get_buf = NULL;
    __this->host_spk.put_buf = NULL;
}

#endif
