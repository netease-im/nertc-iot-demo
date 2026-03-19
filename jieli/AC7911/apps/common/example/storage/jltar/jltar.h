#ifndef JLTAR_H__
#define JLTAR_H__

#include "fs/fs.h"

#if 1
#define jltar_info(x, ...)    printf( x " ", ## __VA_ARGS__)
#else
#define jltar_info(...)
#endif

#define JLTAR_HEADER (0x10) //头部信息
#define JLTAR_LEVEL1 (0x10000 * 4)//一级索引总大小
#define JLTAR_NAME "JL01"
#define FILE_INDEX_NUM 4 //截取文件名前面4个字符作为索引值

struct jltar_head {
    char fileName[4];
    int fileCnt;
    int fileSize;
    int ts;
};

struct jltar_finfo {
    int n;
    char filename[52];
    int filesize;
    int fileindex;
};

typedef struct JLFILE_ {
    FILE *fd;
    struct jltar_finfo fileInfo;
    struct jltar_head headInfo;
} JLFILE;

JLFILE *jltar_fopen(const char *path, const char *filename, const char *mode);
int jltar_fread(void *buf, u32 size, u32 count, JLFILE *file);
int jltar_close(JLFILE *file);
int jltar_flen(JLFILE *file);

#endif



