#ifndef __FILE_BLUK_TRANS_PREPARE_H__
#define __FILE_BLUK_TRANS_PREPARE_H__

#include "typedef.h"
#include "app_config.h"

void file_bluk_trans_prepare(void *priv, u8 OpCode_SN, u8 *data, u16 len);
void file_bluk_trans_init(void (*end_callback)(void));
void file_bluk_trans_close(u8 dire);

#endif
