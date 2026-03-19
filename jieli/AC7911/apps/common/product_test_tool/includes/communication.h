#ifndef _PRODUCT_COMM_H_
#define _PRODUCT_COMM_H_


typedef struct {
    s8(*init)(void);
    s8(*online)(void);
    s8(*read)(u8 *, u32);
    s8(*write)(u8 *, u32);
} prod_comm_type;


prod_comm_type *comm_ops(void);
int host_data_read(u8 *data, u32 len);
int host_data_write(u8 *data, u32 len);
u8 product_info_check(void);
void product_net_client_init(void);
u8 get_product_id(void);


#endif


