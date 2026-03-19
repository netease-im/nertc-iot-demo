#include "rcsp_config.h"
#include "rcsp_event.h"
#include "file_transfer.h"

///>>>>>>>>>>>设备接收到APP下发不需要回复数据
#if (RCSP_MODE)
void data_recieve_no_respone(void *priv, u8 CMD_OpCode, u8 *data, u16 len)
{
//    printf("data_recieve_no_respone %x\n", CMD_OpCode);
    switch (CMD_OpCode) {
    case JL_OPCODE_FILE_TRANSFER:
        file_transfer_download_doing(data, len);
        break;
    }
}
#endif//RCSP_MODE

