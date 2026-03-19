#include "jltar.h"
#include "fs/fs.h"
#include "stdlib.h"

JLFILE *jltar_fopen(const char *path, const char *filename, const char *mode)
{
    char hd[16];
    int ret;
    FILE *fd = NULL;

    JLFILE *fp = (JLFILE *)malloc(sizeof(JLFILE));
    if (NULL == fp) {
        jltar_info("%s : malloc fail!\n", __FUNCTION__);
        return NULL;
    }

    fp->fd = fopen(path, mode);
    if (NULL == fp->fd) {
        jltar_info("%s : open err!\n", __FUNCTION__);
        free(fp);
        return NULL;
    }

    ret = fread(&fp->headInfo, 1, sizeof(struct jltar_head), fp->fd);
    if (ret != sizeof(struct jltar_head)) {
        jltar_info("%s : read head err!\n", __FUNCTION__);
        fclose(fp->fd);
        free(fp);
        return NULL;
    }

    if (strncmp(fp->headInfo.fileName, JLTAR_NAME, strlen(JLTAR_NAME))) {
        jltar_info("%s : file err!\n", __FUNCTION__);
        fclose(fp->fd);
        free(fp);
        return NULL;
    }

    char hex_s[FILE_INDEX_NUM + 3];
    int level_1 = 0;
    int file_exist = 0;
    sprintf(hex_s, "0x%c%c%c%c", filename[0], filename[1], filename[2], filename[3]);

    fseek(fp->fd, JLTAR_HEADER +  strtol(hex_s, NULL, 16) * 4, SEEK_SET);
    fread(&level_1, sizeof(int), 1, fp->fd);
    if (-1 == level_1) {
        jltar_info("%s : get level_1 err!\n", __FUNCTION__);
        fclose(fp->fd);
        free(fp);
        return NULL;
    }

    //读取二级索引
    jltar_info("addr: 0X%X\n", JLTAR_HEADER + JLTAR_LEVEL1 + level_1);
    int n = 0;
    do {
        fseek(fp->fd, JLTAR_HEADER + JLTAR_LEVEL1 + level_1 + (sizeof(struct jltar_finfo)*n), SEEK_SET);
        fread(&fp->fileInfo, sizeof(struct jltar_finfo), 1, fp->fd);
        if (memcmp(fp->fileInfo.filename, filename, 32) == 0) {
            file_exist = 1;
            jltar_info(" n: %d\n filename: %s\n filesize: %d\n fileindex: %d\n", fp->fileInfo.n, fp->fileInfo.filename, fp->fileInfo.filesize, fp->fileInfo.fileindex);
            jltar_info("file exist.\n");
            break;
        }
        n++;
    } while (n < fp->headInfo.fileCnt);

    if (0 == file_exist) {
        jltar_info("file not exist.\n");
        fclose(fp->fd);
        free(fp);
        return NULL;
    }

    file_exist = 0;
    //跳到文件数据处
    int level_2 = (sizeof(struct jltar_finfo) * fp->headInfo.fileCnt);
    fseek(fp->fd, JLTAR_HEADER + JLTAR_LEVEL1 + level_2 + fp->fileInfo.fileindex, SEEK_SET);

    return fp;
}

int jltar_fread(void *buf, u32 size, u32 count, JLFILE *file)
{
    return fread(buf, size, count, file->fd);
}

int jltar_close(JLFILE *file)
{
    if (NULL == file) {
        return -1;
    }

    fclose(file->fd);
    free(file);
    return 0;
}

int jltar_fwrite(void *buf, u32 size, u32 count, JLFILE *file)
{
    return fwrite(buf, size, count, file->fd);
}

int jltar_fseek(JLFILE *file, int offset, int mode)
{
    int position = JLTAR_HEADER + JLTAR_LEVEL1 + file->fileInfo.fileindex + (sizeof(struct jltar_finfo) * file->headInfo.fileCnt);

    switch (mode) {
    case SEEK_SET:
        jltar_info("jltar_fseek : SEEK_SET");
        if (offset > file->fileInfo.filesize) {
            printf("The offset exceeds the file size");
            return -1;
        }
        fseek(file->fd, position + offset, SEEK_SET);
        break;
    case SEEK_END:
        jltar_info("jltar_fseek : SEEK_END");
        fseek(file->fd, position + file->fileInfo.filesize, SEEK_SET);
        break;
    case SEEK_CUR:
        jltar_info("jltar_fseek : SEEK_CUR");
        fseek(file->fd, offset, SEEK_CUR);
        break;
    default:
        jltar_info("unknow mode: %d\n", mode);
        return -1;
    }

    return 0;
}

int jltar_flen(JLFILE *file)
{
    if (NULL == file) {
        return -1;
    }

    return file->fileInfo.filesize;
}

