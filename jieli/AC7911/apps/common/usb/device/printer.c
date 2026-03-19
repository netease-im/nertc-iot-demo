#include "usb/device/usb_stack.h"
#include "usb/usb_config.h"
#include "usb/device/cdc.h"
#include "app_config.h"
#include "os/os_api.h"
#include "printer.h"  //need redefine __u8, __u16, __u32

#define LOG_TAG_CONST       USB
#define LOG_TAG             "[Printer]"
#define LOG_ERROR_ENABLE
#define LOG_DEBUG_ENABLE
#define LOG_INFO_ENABLE
/* #define LOG_DUMP_ENABLE */
#define LOG_CLI_ENABLE
#include "debug.h"

#if TCFG_USB_SLAVE_PRINTER_ENABLE

static const u8 printer_desc[] = {
///Interface
    USB_DT_INTERFACE_SIZE,   //Length
    USB_DT_INTERFACE,   //DescriptorType:Inerface
    0x00,   //InterfaceNum:0
    0x00,   //AlternateSetting:0
    0x02,   //NumEndpoint:0
    USB_CLASS_PRINTER,   //InterfaceClass:Printer
    0x01,   //Printers
    0x02,   //InterfaceProtocol
    0x04,   //Interface String
///Endpoint IN
    USB_DT_ENDPOINT_SIZE,
    USB_DT_ENDPOINT,
    USB_DIR_IN | PTR_BULK_EP_IN,
    USB_ENDPOINT_XFER_BULK,
    LOBYTE(MAXP_SIZE_BULKIN), HIBYTE(MAXP_SIZE_BULKIN),
    0x01,
///Endpoint OUT
    USB_DT_ENDPOINT_SIZE,
    USB_DT_ENDPOINT,
    PTR_BULK_EP_OUT,
    USB_ENDPOINT_XFER_BULK,
    LOBYTE(MAXP_SIZE_BULKOUT), HIBYTE(MAXP_SIZE_BULKOUT),
    0x01,
};


struct usb_printer_handle {
    u8 *ep_out_dmabuffer;
    u8 *ep_in_dmabuffer;
    void (*wakeup_handle)(struct usb_device_t *usb_device);
    void (*reset_wakeup_handle)(struct usb_device_t *usb_device, u32 itf_num);

};

struct usb_printer_handle *printer_handle = NULL;
static OS_SEM wake_sem;


static void printer_wakeup_handler(struct usb_device_t *usb_device)
{
    os_sem_post(&wake_sem);
}


static void printer_wakeup(struct usb_device_t *usb_device, u32 ep)
{
#if 0 //for test
    u32 len = 0;
    u8 read_ep[64];
    const usb_dev usb_id = usb_device2id(usb_device);

    ep &= 0x0f;
    len = usb_g_intr_read(usb_id, ep, read_ep, sizeof(read_ep), 0);
    printf("recv_len = %d, ep = %d\n", len, ep);
    put_buf(read_ep, len);
#else
    if ((printer_handle) && (printer_handle->wakeup_handle)) {
        printer_handle->wakeup_handle(usb_device);
    }
#endif
}


static void reset_wakeup(struct usb_device_t *usb_device, u32 itf_num)
{
    const usb_dev usb_id = usb_device2id(usb_device);
    if (printer_handle && printer_handle->reset_wakeup_handle) {
        printer_handle->reset_wakeup_handle(usb_device, itf_num);
    }
}


static void printer_endpoint_init(struct usb_device_t *usb_device, u32 itf)
{
    const usb_dev usb_id = usb_device2id(usb_device);

    usb_g_ep_config(usb_id, PTR_BULK_EP_IN | USB_DIR_IN, USB_ENDPOINT_XFER_BULK, 0, printer_handle->ep_out_dmabuffer, MAXP_SIZE_BULKIN);

    usb_g_ep_config(usb_id, PTR_BULK_EP_OUT, USB_ENDPOINT_XFER_BULK, 1, printer_handle->ep_out_dmabuffer, MAXP_SIZE_BULKOUT);
    usb_g_set_intr_hander(usb_id, PTR_BULK_EP_OUT, printer_wakeup);

    usb_enable_ep(usb_id, PTR_BULK_EP_IN);
    usb_enable_ep(usb_id, PTR_BULK_EP_OUT);
}


static const u8 device_id[] = {
    /* 0x00, 0x0D, 0x50, 0x72, 0x69, 0x6E, 0x74, 0x65, 0x72, 0x20, 0x56, 0x30, 0x2E, 0x31, 0x00 */
    0x00, 0x50, 0x4d, 0x46, 0x47, 0x3a, 0x50, 0x6f, 0x73, 0x74, 0x65, 0x4b, 0x50, 0x72, 0x69, 0x6e,
    0x74, 0x65, 0x72, 0x73, 0x3b, 0x4d, 0x44, 0x4c, 0x3a, 0x50, 0x4f, 0x53, 0x54, 0x45, 0x4b, 0x20,
    0x51, 0x38, 0x2f, 0x32, 0x30, 0x30, 0x3b, 0x43, 0x4d, 0x44, 0x3a, 0x50, 0x50, 0x4c, 0x49, 0x3b,
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
};


static u32 printer_itf_hander(struct usb_device_t *usb_device, struct usb_ctrlrequest *setup)
{
    u32 tx_len;
    u8 *tx_payload = usb_get_setup_buffer(usb_device);
    u32 bRequestType = setup->bRequestType & USB_TYPE_MASK;
    log_info("%s() %x %x", __func__, bRequestType, setup->bRequest);
    switch (bRequestType) {
    case USB_TYPE_STANDARD:
        switch (setup->bRequest) {
        case USB_REQ_GET_STATUS:
            usb_set_setup_phase(usb_device, USB_EP0_STAGE_SETUP);
            break;
        case USB_REQ_GET_INTERFACE:
            tx_len = 1;
            u8 i = 0;
            usb_set_data_payload(usb_device, setup, &i, tx_len);
            break;
        case USB_REQ_SET_INTERFACE:
            usb_set_setup_phase(usb_device, USB_EP0_STAGE_SETUP);
            break;
        default:
            break;
        }
        break;
    case USB_TYPE_CLASS:
        switch (setup->bRequest) {
        case 0:
            printer_endpoint_init(usb_device, -1);
            tx_len = sizeof(device_id);
            memcpy(tx_payload, device_id, sizeof(device_id));
            usb_set_data_payload(usb_device, setup, tx_payload, tx_len);
            break;
        default:
            break;
        }
        break;
    default:
        break;
    }
    return 0;
}


u32 printer_desc_config(const usb_dev usb_id, u8 *ptr, u32 *cur_itf_num)
{
    log_debug("%s() %d", __func__, *cur_itf_num);
    memcpy(ptr, printer_desc, sizeof(printer_desc));
    ptr[2] = *cur_itf_num;
    if (usb_set_interface_hander(usb_id, *cur_itf_num, printer_itf_hander) != *cur_itf_num) {

    }
    if (usb_set_reset_hander(usb_id, *cur_itf_num, reset_wakeup) != *cur_itf_num) {

    }
    *cur_itf_num = *cur_itf_num + 1;
    return sizeof(printer_desc);
}


u32 printer_register(const usb_dev usb_id)
{
    if (printer_handle == NULL) {
        printer_handle = (struct usb_printer_handle *)zalloc(sizeof(struct usb_printer_handle));
        if (printer_handle == NULL) {
            log_error("printer_register err");
            return -1;
        }
        log_info("printer_handle = %x", printer_handle);
        os_sem_create(&wake_sem, 0);

        printer_handle->ep_out_dmabuffer = usb_alloc_ep_dmabuffer(usb_id, PTR_BULK_EP_OUT, MAXP_SIZE_BULKIN + MAXP_SIZE_BULKOUT);
        (usb_id, PTR_BULK_EP_OUT, MAXP_SIZE_BULKIN + MAXP_SIZE_BULKOUT);
        printer_handle->wakeup_handle = printer_wakeup_handler;
    }
    return 0;
}


u32 printer_release()
{
    if (printer_handle) {
        os_sem_del(&wake_sem, 0);
        free(printer_handle);
        printer_handle = NULL;
    }
    return 0;
}


#if 0

#include "event/device_event.h"

static u8 printer_exit;
static usb_dev printer_id;

static void printer_data_handler_task(void *priv)
{
    u32 len;
    u8 buffer[64];

    for (;;) {
        os_sem_pend(&wake_sem, 0);
        if (printer_exit) {
            return;
        }
        memset(buffer, 0, sizeof(buffer));
        len = usb_g_bulk_read(printer_id, PTR_BULK_EP_OUT, buffer, sizeof(buffer), 0);
        printf("%s, recv_len = %d\n", __func__, len);

        put_buf(buffer, len);
        usb_g_bulk_write(printer_id, PTR_BULK_EP_IN, buffer, len);
    }
}



static int product_device_event_handler(struct sys_event *e)
{
    int pid;
    struct device_event *event = (struct device_event *)e->payload;
    const char *usb_msg = (const char *)event->value;

    if (e->from == DEVICE_EVENT_FROM_OTG) {
        if (usb_msg[0] == 's') {
            if (event->event == DEVICE_EVENT_IN) {
                printf("usb %c online", usb_msg[2]);
                printer_id = usb_msg[2] - '0';
                printer_exit = 0;
                thread_fork("printer_data_handler_task", 30, 1024, 0, &pid, printer_data_handler_task, NULL);
            } else if (event->event == DEVICE_EVENT_OUT) {
                printf("usb %c offline", usb_msg[2]);
                printer_exit = 1;
                os_sem_post(&wake_sem);
                thread_kill(&pid, KILL_WAIT);
            }
        }
    }
    return 0;
}


SYS_EVENT_STATIC_HANDLER_REGISTER(product_device_event) = {
    .event_type     = SYS_DEVICE_EVENT,
    .prob_handler   = product_device_event_handler,
    .post_handler   = NULL,
};

#endif

#endif


