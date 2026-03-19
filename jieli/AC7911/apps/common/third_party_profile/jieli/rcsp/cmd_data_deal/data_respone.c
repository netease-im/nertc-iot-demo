#include "rcsp_config.h"
#include "rcsp_event.h"

////>>>>>>>>>>>>>>>设备接收到上报数据的回复
#if (RCSP_MODE)
void data_respone(void *priv, u8 status, u8 CMD_OpCode, u8 *data, u16 len)
{
    printf("data_respone, %x\n", CMD_OpCode);
}
#endif//RCSP_MODE

