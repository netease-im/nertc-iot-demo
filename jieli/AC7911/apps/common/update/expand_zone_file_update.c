#include "app_config.h"
#include "res_config.h"
#include "asm/sfc_norflash_api.h"
#include "fs/fs.h"
#include "init.h"

#define READ_BUF_SIZE SDFILE_SECTOR_SIZE

int check_expand_zone_packres_file_crc(const char *path)
{
    u32 file_addr, file_len, file_offset, read_len;
    u16 file_crc, header_crc, calculate;

    FILE *fp = fopen(path, "r");
    if (fp == NULL) {
        return -1;
    }

    struct vfs_attr file_attr;
    fget_attrs(fp, &file_attr);
    file_addr = file_attr.sclust;

    printf("%s addr is 0x%x, size is %x", path, file_addr, flen(fp));

    fclose(fp);

    u8 *data = (u8 *)malloc(READ_BUF_SIZE);
    if (!data) {
        return -1;
    }

    sdfile_reserve_zone_read(data, file_addr, READ_BUF_SIZE, 0);

    header_crc = data[0] | ((u16)data[1] << 8);
    file_crc = data[2] | ((u16)data[3] << 8);
    file_offset = data[4] | ((u32)data[5] << 8) | ((u32)data[6] << 16) | ((u32)data[7] << 24);
    file_len = data[8] | ((u32)data[9] << 8) | ((u32)data[10] << 16) | ((u32)data[11] << 24);

    printf("file crc 0x0x%x, header_crc 0x%x, file_len 0x%x", file_crc, header_crc, file_len);

    if (file_offset != 0x20 || file_len <= file_offset) {
        goto __err;
    }

    calculate = CRC16(data + 2, file_offset - 2);
    if (header_crc != calculate) {
        printf("header_crc calculate err 0x%x, 0x%x", header_crc, calculate);
        goto __err;
    }

    read_len = READ_BUF_SIZE - file_offset > file_len - file_offset ? file_len - file_offset : READ_BUF_SIZE - file_offset;
    calculate = CRC16(data + file_offset, read_len);
    file_offset += read_len;

    while (file_offset < file_len) {
        sdfile_reserve_zone_read(data, file_addr + file_offset, READ_BUF_SIZE, 0);
        read_len = READ_BUF_SIZE > file_len - file_offset ? file_len - file_offset : READ_BUF_SIZE;
        calculate = CRC16_with_initval(data, read_len, calculate);
        file_offset += read_len;
    }

    if (file_crc != calculate) {
        printf("file_crc calculate err 0x%x, 0x%x", file_crc, calculate);
        goto __err;
    }

    free(data);

    return 0;

__err:
    free(data);

    return -1;
}

u32 expand_zone_packres_file_update_start(const char *path)
{
    u32 file_addr;

    FILE *fp = fopen(path, "r");
    if (fp == NULL) {
        return 0;
    }

    struct vfs_attr file_attr;
    fget_attrs(fp, &file_attr);
    file_addr = file_attr.sclust;
    fclose(fp);

    return file_addr;
}

u32 expand_zone_packres_file_update_write(u32 addr, void *buf, u32 len)
{
    u32 offset = 0;

    ASSERT((addr & (SDFILE_SECTOR_SIZE - 1)) == 0 && (len & (SDFILE_SECTOR_SIZE - 1)) == 0);

    while (offset < len) {
        sdfile_reserve_zone_erase(addr + offset, SDFILE_SECTOR_SIZE, 0);
        sdfile_reserve_zone_write(buf + offset, addr + offset, SDFILE_SECTOR_SIZE, 0);
        offset += SDFILE_SECTOR_SIZE;
    }

    return offset;
}

//测试时需要修改isd_config.ini文件把UIPACKRES_LEN设置成AUPACKRES_LEN同一个值
static int expand_zone_packres_file_test(void)
{
    u32 au_update_addr, ui_update_addr;
    void *data = malloc(SDFILE_SECTOR_SIZE);

    check_expand_zone_packres_file_crc("mnt/sdfile/EXT_RESERVED/aupackres");
    check_expand_zone_packres_file_crc("mnt/sdfile/EXT_RESERVED/uipackres");

    au_update_addr = expand_zone_packres_file_update_start("mnt/sdfile/EXT_RESERVED/aupackres");
    ui_update_addr = expand_zone_packres_file_update_start("mnt/sdfile/EXT_RESERVED/uipackres");

    for (u32 offset = 0; offset < 0x141000; offset += SDFILE_SECTOR_SIZE) {
        sdfile_reserve_zone_read(data, au_update_addr + offset, SDFILE_SECTOR_SIZE, 0);
        //把UI资源擦写成提示音资源
        expand_zone_packres_file_update_write(ui_update_addr + offset, data, SDFILE_SECTOR_SIZE);
    }

    free(data);

    //升级完成后重新校验文件完整性
    check_expand_zone_packres_file_crc("mnt/sdfile/EXT_RESERVED/aupackres");
    check_expand_zone_packres_file_crc("mnt/sdfile/EXT_RESERVED/uipackres");

    return 0;
}
/* module_initcall(expand_zone_packres_file_test); */
