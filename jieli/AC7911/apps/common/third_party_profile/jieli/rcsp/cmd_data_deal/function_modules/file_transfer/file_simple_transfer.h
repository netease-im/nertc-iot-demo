#ifndef __FILE_SIMPLE_TRANSFER_H__
#define __FILE_SIMPLE_TRANSFER_H__

#include "typedef.h"
#include "app_config.h"

typedef struct file_simple_transfer_opt {
    // 返回长度(成功)/0(失败)      根据当前文件类型获取id表长度(传入 文件类型)，该接口返回的长度是 [id(2byte) + file_size(没有文件就是0，但都是占2个byte的位置)] * n
    int (*get_id_table_len)(u8 file_type);
    // 成功返回长度/失败0   根据类型获取文件id列表(传入 文件类型)，table_data用于存放返回数据，返回的数据格式：[id(2byte) + file_size(没有文件就是0，但都是占2个byte的位置)] * n
    int (*get_id_table)(u8 file_type, u8 *table_data, u16 data_len);
    // 成功返回数据长度/失败0   读取文件(文件id，数据偏移，数据，数据长度)
    int (*read_file_by_id)(u8 file_type, u16 id, u32 data_offset, u8 *data, u16 data_len);
    // 成功返回0/失败非0   新增文件(文件id，数据偏移，数据，数据长度)
    int (*insert_file_by_id)(u8 file_type, u16 *id, u32 data_offset, u8 *data, u16 data_len, u16 file_total_size);
    // 成功返回0/失败非0   更新文件(文件id，数据偏移，数据，数据长度)
    int (*update_file_by_id)(u8 file_type, u16 id, u32 data_offset, u8 *data, u16 data_len, u16 file_total_size);
    // 成功返回0/失败非0   删除文件(文件id)
    int (*delete_file_by_id)(u8 file_type, u16 id);
} simple_trans_opt;

void file_simple_transfer_for_small_file(void *priv, u8 OpCode_SN, u8 *data, u16 len);
int register_file_simple_transfer_interface(simple_trans_opt *file_simple_transfer_interface);

#endif
