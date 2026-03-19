#include "os_jl.h"
#include "character_coding.h"

#define SQLITE_JL_MAX_PATH_CHARS   (256)

#ifndef SQLITE_DEMOVFS_BUFFERSZ
# define SQLITE_DEMOVFS_BUFFERSZ (8192*2)
#endif

struct vfs_file {
    sqlite3_file base;          /*** Must be first ***/
    FILE *h;

    char *aBuffer;                  /* Pointer to malloc'd buffer */
    int nBuffer;                    /* Valid bytes of data in zBuffer */
    sqlite3_int64 iBufferOfst;      /* Offset in file of zBuffer[0] */
};

static int demoDirectWrite(
    struct vfs_file *p,                    /* File handle */
    const void *zBuf,               /* Buffer containing data to write */
    int iAmt,                       /* Size of data to write in bytes */
    sqlite_int64 iOfst              /* File offset to write to */
)
{
    off_t ofst;                     /* Return value from lseek() */
    size_t nWrite;                  /* Return value from write() */

    if (iOfst > flen(p->h)) {
        return SQLITE_IOERR_WRITE;
    }

    ofst = fseek(p->h, iOfst, SEEK_SET);
    if (ofst != 0) {
        return SQLITE_IOERR_WRITE;
    }

    nWrite = fwrite((void *)zBuf, 1, iAmt, p->h);
    if (nWrite != iAmt) {
        return SQLITE_IOERR_WRITE;
    }

    return SQLITE_OK;
}

static int demoFlushBuffer(struct vfs_file *p)
{
    int rc = SQLITE_OK;
    if (p->nBuffer) {
        rc = demoDirectWrite(p, p->aBuffer, p->nBuffer, p->iBufferOfst);
        p->nBuffer = 0;
    }
    return rc;
}

static int jlClose(sqlite3_file *id)
{
    int rc;
    struct vfs_file *p = (struct vfs_file *)id;

    rc = demoFlushBuffer(p);
    sqlite3_free(p->aBuffer);
    fseek(p->h, 0, SEEK_END);
    fclose(p->h);
    return rc;
}

static int jlRead(
    sqlite3_file *id,          /* File to read from */
    void *pBuf,                /* Write content into this buffer */
    int amt,                   /* Number of bytes to read */
    sqlite3_int64 offset       /* Begin reading at this offset */
)
{
    struct vfs_file *pFile = (struct vfs_file *)id; /* file handle */
    int rc;

    /* printf("jlRead len:%d off:%lld \n",amt,offset); */

    if (!pFile->h) {
        return SQLITE_IOERR_READ;
    }

    rc = demoFlushBuffer(pFile);
    if (rc != SQLITE_OK) {
        return rc;
    }

    if (offset > flen(pFile->h)) {
        return SQLITE_IOERR_SHORT_READ;
    }

    if (fseek(pFile->h, offset, SEEK_SET) != 0) {
        return SQLITE_IOERR_READ;
    }

    int ret = fread(pBuf, 1, amt, pFile->h);
    if (ret == amt) {
        return SQLITE_OK;
    } else if (ret >= 0) {
        if (ret < amt) {
            memset(&((char *)pBuf)[ret], 0, amt - ret);
        }
        return SQLITE_IOERR_SHORT_READ;
    }

    return SQLITE_IOERR_READ;
}

static int jlWrite(
    sqlite3_file *id,
    const void *zBuf,
    int iAmt,
    sqlite_int64 iOfst
)
{
    struct vfs_file *p = (struct vfs_file *)id; /* file handle */

    /* printf("jlWrite len:%d off:%lld \n",iAmt,iOfst); */

    if (p->aBuffer) {
        char *z = (char *)zBuf;       /* Pointer to remaining data to write */
        int n = iAmt;                 /* Number of bytes at z */
        sqlite3_int64 i = iOfst;      /* File offset to write to */

        while (n > 0) {
            int nCopy;                  /* Number of bytes to copy into buffer */

            /* If the buffer is full, or if this data is not being written directly
            ** following the data already buffered, flush the buffer. Flushing
            ** the buffer is a no-op if it is empty.
            */
            if (p->nBuffer == SQLITE_DEMOVFS_BUFFERSZ || p->iBufferOfst + p->nBuffer != i) {
                int rc = demoFlushBuffer(p);
                if (rc != SQLITE_OK) {
                    return rc;
                }
            }
            p->iBufferOfst = i - p->nBuffer;

            /* Copy as much data as possible into the buffer. */
            nCopy = SQLITE_DEMOVFS_BUFFERSZ - p->nBuffer;
            if (nCopy > n) {
                nCopy = n;
            }
            memcpy(&p->aBuffer[p->nBuffer], z, nCopy);
            p->nBuffer += nCopy;

            n -= nCopy;
            i += nCopy;
            z += nCopy;
        }
    } else {
        return demoDirectWrite(p, zBuf, iAmt, iOfst);
    }

    return SQLITE_OK;
}

static int jlDeviceCharacteristics(sqlite3_file *id)
{
    return 0;
}

static int jlFileControl(sqlite3_file *id, int op, void *pArg)
{
    return SQLITE_NOTFOUND;
}
static int jlUnlock(sqlite3_file *id, int locktype)
{
    return SQLITE_OK;
}
static int jlLock(sqlite3_file *id, int locktype)
{
    return SQLITE_OK;
}

static int jlFileSize(sqlite3_file *id, sqlite3_int64 *pSize)
{
    struct vfs_file *p = (struct vfs_file *)id;
    int rc;

    rc = demoFlushBuffer(p);
    if (rc != SQLITE_OK) {
        return rc;
    }

    int ilen = flen(p->h);

    /* printf("jlFileSize :%d \n",ilen); */

    if (ilen < 0) {
        return SQLITE_IOERR_FSTAT;
    }

    *pSize = ilen;

    return SQLITE_OK;
}

static int jlTruncate(sqlite3_file *id, sqlite3_int64 nByte)
{
    struct vfs_file *pFile = (struct vfs_file *)id;
    ftruncate(pFile->h, nByte);
    return SQLITE_OK;
}
static int jlCheckReservedLock(sqlite3_file *id, int *pResOut)
{
    *pResOut = 0 ;
    return SQLITE_OK;
}
static int jlSync(sqlite3_file *id, int flags)
{
    struct vfs_file *p = (struct vfs_file *)id;

    int rc;
    rc = demoFlushBuffer(p);
    if (rc != SQLITE_OK) {
        return rc;
    }

    return SQLITE_OK;
}

static int jlSectorSize(sqlite3_file *id)
{
    return 0;
}

static const sqlite3_io_methods jlIoMethod = {
    1,                              /* iVersion */
    jlClose,                       /* xClose */
    jlRead,                        /* xRead */
    jlWrite,                       /* xWrite */
    jlTruncate,                    /* xTruncate */
    jlSync,                        /* xSync */
    jlFileSize,                    /* xFileSize */
    jlLock,                        /* xLock */
    jlUnlock,                      /* xUnlock */
    jlCheckReservedLock,           /* xCheckReservedLock */
    jlFileControl,                 /* xFileControl */
    jlSectorSize,                  /* xSectorSize */
    jlDeviceCharacteristics,       /* xDeviceCharacteristics */
};

static int jlAccess(
    sqlite3_vfs *pVfs,         /* Not used on win32 */
    const char *zFilename,     /* Name of file to check */
    int flags,                 /* Type of test to make on this file */
    int *pResOut               /* OUT: Result */
)
{
    FILE *fd = NULL;

    fd = fopen(zFilename, "r");

    if (fd == NULL) {
        //no exist
        *pResOut = 0;
    } else {
        int attr;
        fget_attr(fd, &attr);
        if (flags == SQLITE_ACCESS_READWRITE) {
            if (attr & F_ATTR_RW) {
                *pResOut = 1;
            } else {
                *pResOut = 0;
            }
        } else if (flags == SQLITE_ACCESS_READ) {
            if (attr & F_ATTR_RO) {
                *pResOut = 1;
            } else {
                *pResOut = 0;
            }
        } else {
            *pResOut = 1;
        }

        fclose(fd);
    }

    // printf("access name:%s flag:%d res:%d \n", zFilename, flags, *pResOut);

    return SQLITE_OK;
}

static int jlOpen(
    sqlite3_vfs *pVfs,        /* Used to get maximum path length and AppData */
    const char *zName,        /* Name of the file (UTF-8) */
    sqlite3_file *id,         /* Write the SQLite file handle here */
    int flags,                /* Open mode flags */
    int *pOutFlags            /* Status return flags */
)
{
    FILE *h = NULL;
    char *aBuf = 0;
    struct vfs_file *pFile = (struct vfs_file *)id;
    const char *open_name;
    //char long_name[256];/*<-------------文件路径名太长的时候这里出事了 大约大于62的时候就出错了*/
    char *long_name;

    long_name = (char *)malloc(strlen(zName) * 2 + 8);
    memset(long_name, 0, strlen(zName) * 2 + 8);
    memset(pFile, 0, sizeof(struct vfs_file));

    if (flags & SQLITE_OPEN_MAIN_JOURNAL) {
        aBuf = (char *)sqlite3_malloc(SQLITE_DEMOVFS_BUFFERSZ);
        if (!aBuf) {
            free(long_name);
            return SQLITE_NOMEM;
        }
    }

    const char *filename = strrchr(zName, '/');
    if (filename) {
        filename++;
    } else {
        filename = zName;
    }

    if (strlen(filename) > 12) {
        //long_file_name_encode(zName, long_name, ARRAY_SIZE(long_name));
        long_file_name_encode(zName, (u8 *)long_name, strlen(zName) * 2 + 8);
        open_name = long_name;
    } else {
        open_name = zName;
    }

    h = fopen(open_name, "r+");
    if (h == NULL) {
        h = fopen(open_name, "w+");
    }
    if (h == NULL) {
        printf("create the file:%s %d fail\n", zName, fget_err_code(open_name));
        free(long_name);
        return SQLITE_ERROR;
    }

    if (pOutFlags) {
        *pOutFlags = flags;
    }

    pFile->aBuffer = aBuf;
    pFile->base.pMethods = &jlIoMethod;
    pFile->h = h;

    free(long_name);
    return SQLITE_OK;
}

static int jlDelete(
    sqlite3_vfs *pVfs,          /* Not used on win32 */
    const char *zFilename,      /* Name of file to delete */
    int syncDir                 /* Not used on win32 */
)
{
    const char *del_name;
    //char long_name[256];/*<-------------文件名太长的时候这里出事了*/
    char *long_name;


    const char *filename = strrchr(zFilename, '/');
    long_name = (char *)malloc(strlen(zFilename) * 2 + 8);
    memset(long_name, 0, strlen(zFilename) * 2 + 8);
    if (filename) {
        filename++;
    } else {
        filename = zFilename;
    }

    if (strlen(filename) > 12) {
        //long_file_name_encode(zFilename, long_name, ARRAY_SIZE(long_name));
        long_file_name_encode(zFilename, (u8 *)long_name, strlen(zFilename) * 2 + 8);
        del_name = long_name;
    } else {
        del_name = zFilename;
    }


    if (fdelete_by_name(del_name) == 0) {
        free(long_name);
        return SQLITE_OK; /* Deleted OK. */
    } else {
        free(long_name);
        printf("delete %s fail.", zFilename);
        return SQLITE_ERROR;
    }
}

static int jlFullPathname(
    sqlite3_vfs *pVfs,            /* Pointer to vfs object */
    const char *zRelative,        /* Possibly relative input path */
    int nFull,                    /* Size of output buffer in bytes */
    char *zFull                   /* Output buffer */
)
{
    int nByte = nFull;
    if (pVfs->mxPathname < nByte) {
        nByte = pVfs->mxPathname;
    }
    memcpy(zFull, zRelative, nByte);

    return SQLITE_OK;
}

static void *jlDlOpen(sqlite3_vfs *pVfs, const char *zPath)
{
    return 0;
}

static void jlDlError(sqlite3_vfs *pVfs, int nByte, char *zErrMsg)
{
    sqlite3_snprintf(nByte, zErrMsg, "Loadable extensions are not supported");
    zErrMsg[nByte - 1] = '\0';
}

static void (*jlDlSym(sqlite3_vfs *pVfs, void *pH, const char *z))(void)
{
    return 0;
}

static void jlDlClose(sqlite3_vfs *pVfs, void *pHandle)
{
    return;
}

/*
** Parameter zByte points to a buffer nByte bytes in size. Populate this
** buffer with pseudo-random data.
*/
static int jlRandomness(sqlite3_vfs *pVfs, int nByte, char *zByte)
{
    return SQLITE_OK;
}

/*
** Sleep for at least nMicro microseconds. Return the (approximate) number
** of microseconds slept for.
*/
static int jlSleep(sqlite3_vfs *pVfs, int nMicro)
{
    os_time_dly(nMicro / 10);
    return nMicro;
}

static int jlCurrentTime(sqlite3_vfs *pVfs, double *pTime)
{
    int t = timer_get_ms();
    *pTime = t / 86400.0 + 2440587.5;
    return SQLITE_OK;
}

/*
** Initialize and deinitialize the operating system interface.
*/
int rtos_vfs_init(void)
{
    static sqlite3_vfs jlVfs = {
        1,                     /* iVersion */
        sizeof(struct vfs_file),       /* szOsFile */
        SQLITE_JL_MAX_PATH_CHARS, /* mxPathname */
        0,                     /* pNext */
        "JLOS",               /* zName */
        0,                    /* pAppData */
        jlOpen,               /* xOpen */
        jlDelete,             /* xDelete */
        jlAccess,             /* xAccess */
        jlFullPathname,       /* xFullPathname */
        jlDlOpen,                   /* xDlOpen */
        jlDlError,                  /* xDlError */
        jlDlSym,                    /* xDlSym */
        jlDlClose,                  /* xDlClose */
        jlRandomness,               /* xRandomness */
        jlSleep,                    /* xSleep */
        jlCurrentTime,              /* xCurrentTime */
    };

    sqlite3_vfs_register(&jlVfs, 1);

    return SQLITE_OK;
}

int rtos_vfs_end(void)
{

    return SQLITE_OK;
}
