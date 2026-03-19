#ifndef __RCSP_NFC_DATA_FUNC_H__
#define __RCSP_NFC_DATA_FUNC_H__

#include "typedef.h"
#include "app_config.h"

int JL_rcsp_nfc_data_funciton(void *priv, u8 OpCode, u8 OpCode_SN, u8 *data, u16 len);
int nfc_id_default_notify(u32 devHeadler, u16 nfc_id);
void nfc_file_trans_back_end(void *priv, u32 handler, u8 op, int result, int param);

#endif
