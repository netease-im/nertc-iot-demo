#ifdef SQLITE_OS_AWTK

#define AWTK_MAX_PATHNAME 256

#define O_RDONLY 0x0000 /* open for reading only */
#define O_WRONLY 0x0001 /* open for writing only */
#define O_RDWR 0x0002   /* open for reading and writing */
#define O_EXCL 0

#ifndef O_CREAT
#define O_CREAT 0x00080
#endif /*O_CREAT*/

/*
** Define various macros that are missing from some systems.
*/
#ifndef O_LARGEFILE
#define O_LARGEFILE 0
#endif
#ifdef SQLITE_DISABLE_LFS
#undef O_LARGEFILE
#define O_LARGEFILE 0
#endif
#ifndef O_NOFOLLOW
#define O_NOFOLLOW 0
#endif
#ifndef O_BINARY
#define O_BINARY 0
#endif

#ifndef EINTR
#define EINTR 4 /* Interrupted system call */
#endif

#ifndef ENOLCK
#define ENOLCK 46 /* No record locks available */
#endif

#ifndef EACCES
#define EACCES 13 /* Permission denied */
#endif

#ifndef EPERM
#define EPERM 1 /* Operation not permitted */
#endif

#ifndef ETIMEDOUT
#define ETIMEDOUT 145 /* Connection timed out */
#endif

#ifndef ENOTCONN
#define ENOTCONN 134 /* Transport endpoint is not connected */
#endif

#ifndef ENOENT
#define ENOENT 2 /* No such file or directory */
#endif           /*ENOENT*/

#ifndef EISDIR
#define EISDIR 21 /* Is a directory */
#endif            /*EISDIR*/

static int _Access(const char *pathname, int mode)
{
    if (file_exist(pathname)) {
        return 0;
    } else {
        return -1;
    }
}

#define _AWTK_LOG_ERROR(a, b, c) _awtk_log_error_at_line(a, b, c, __LINE__)

static int _awtk_log_error_at_line(int errcode,       /* SQLite error code */
                                   const char *zFunc, /* Name of OS function that failed */
                                   const char *zPath, /* File path associated with error */
                                   int iLine          /* Source line number where error occurred */
                                  )
{
    char *zErr;         /* Message from strerror() or equivalent */
    int iErrno = errno; /* Saved syscall error number */

    /* If this is not a threadsafe build (SQLITE_THREADSAFE==0), then use
      ** the strerror() function to obtain the human-readable error message
      ** equivalent to errno. Otherwise, use strerror_r().
      */
#if SQLITE_THREADSAFE && defined(HAVE_STRERROR_R)
    char aErr[80];
    memset(aErr, 0, sizeof(aErr));
    zErr = aErr;

    /* If STRERROR_R_CHAR_P (set by autoconf scripts) or __USE_GNU is defined,
      ** assume that the system provides the GNU version of strerror_r() that
      ** returns a pointer to a buffer containing the error message. That pointer
      ** may point to aErr[], or it may point to some static storage somewhere.
      ** Otherwise, assume that the system provides the POSIX version of
      ** strerror_r(), which always writes an error message into aErr[].
      **
      ** If the code incorrectly assumes that it is the POSIX version that is
      ** available, the error message will often be an empty string. Not a
      ** huge problem. Incorrectly concluding that the GNU version is available
      ** could lead to a segfault though.
      */
#if defined(STRERROR_R_CHAR_P) || defined(__USE_GNU)
    zErr =
#endif
        strerror_r(iErrno, aErr, sizeof(aErr) - 1);

#elif SQLITE_THREADSAFE
    /* This is a threadsafe build, but strerror_r() is not available. */
    zErr = "";
#else
    /* Non-threadsafe build, use strerror(). */
    zErr = strerror(iErrno);
#endif

    if (zPath == 0) {
        zPath = "";
    }

    sqlite3_log(errcode, "os_awtk.c:%d: (%d) %s(%s) - %s", iLine, iErrno, zFunc, zPath, zErr);

    return errcode;
}

typedef struct {
    sqlite3_io_methods const *pMethod;
    sqlite3_vfs *pvfs;
    fs_file_t *fd;
    int eFileLock;
    int szChunk;
    tk_semaphore_t *sem;
} AWTK_SQLITE_FILE_T;

static const char *_awtk_temp_file_dir(void)
{
    const char *azDirs[] = {
        0, "/sql",
        "/sql/tmp"
        "/tmp",
        0 /* List terminator */
    };
    unsigned int i;
    const char *zDir = 0;
    azDirs[0] = sqlite3_temp_directory;

    for (i = 0; i < sizeof(azDirs) / sizeof(azDirs[0]); zDir = azDirs[i++]) {
        if (zDir == 0) {
            continue;
        }
        if (fs_dir_exist(os_fs(), zDir)) {
            break;
        }
    }

    return zDir;
}

/*
** Create a temporary file name in zBuf.  zBuf must be allocated
** by the calling process and must be big enough to hold at least
** pVfs->mxPathname bytes.
*/
static int _awtk_get_temp_name(int nBuf, char *zBuf)
{
    const unsigned char zChars[] =
        "abcdefghijklmnopqrstuvwxyz"
        "ABCDEFGHIJKLMNOPQRSTUVWXYZ"
        "0123456789";
    unsigned int i, j;
    const char *zDir;

    zDir = _awtk_temp_file_dir();

    if (zDir == 0) {
        zDir = ".";
    }

    /* Check that the output buffer is large enough for the temporary file
      ** name. If it is not, return SQLITE_ERROR.
      */
    if ((strlen(zDir) + strlen(SQLITE_TEMP_FILE_PREFIX) + 18) >= (size_t)nBuf) {
        return SQLITE_ERROR;
    }

    do {
        sqlite3_snprintf(nBuf - 18, zBuf, "%s/" SQLITE_TEMP_FILE_PREFIX, zDir);
        j = (int)strlen(zBuf);
        sqlite3_randomness(15, &zBuf[j]);

        for (i = 0; i < 15; i++, j++) {
            zBuf[j] = (char)zChars[((unsigned char)zBuf[j]) % (sizeof(zChars) - 1)];
        }

        zBuf[j] = 0;
        zBuf[j + 1] = 0;
    } while (_Access(zBuf, 0) == 0);

    return SQLITE_OK;
}

#include "awtk_io_methods.h"

/*
** Invoke open().  Do so multiple times, until it either succeeds or
** fails for some reason other than EINTR.
**
** If the file creation mode "m" is 0 then set it to the default for
** SQLite.  The default is SQLITE_DEFAULT_FILE_PERMISSIONS (normally
** 0644) as modified by the system umask.  If m is not 0, then
** make the file creation mode be exactly m ignoring the umask.
**
** The m parameter will be non-zero only when creating -wal, -journal,
** and -shm files.  We want those files to have *exactly* the same
** permissions as their original database, unadulterated by the umask.
** In that way, if a database file is -rw-rw-rw or -rw-rw-r-, and a
** transaction crashes and leaves behind hot journals, then any
** process that is able to write to the database will also be able to
** recover the hot journals.
*/
const char *to_str_mode(const char *file_path, int flags, int m)
{
    /*TODO*/
    if (flags & O_RDONLY) {
        return "rb";
    } else if (flags & O_RDWR) {
        if (flags & O_CREAT) {
            if (file_exist(file_path)) {
                return "rb+";
            } else {
                return "wb+";
            }
        } else {
            return "rb+";
        }
    }
    return "rb+";
}

static fs_file_t *_awtk_fs_open(const char *file_path, int f, int m)
{
    const char *mode = to_str_mode(file_path, f, m);
    fs_file_t *file = fs_open_file(os_fs(), file_path, mode);
    if (file == NULL) {
        log_debug("open %s with mode(%s) failed\n", file_path, mode);
    }
    return file;
}

static int _awtk_vfs_open(sqlite3_vfs *pvfs, const char *file_path, sqlite3_file *file_id,
                          int flags, int *pOutFlags)
{
    AWTK_SQLITE_FILE_T *p;
    int rc = SQLITE_OK; /* Function Return Code */
    int openFlags = 0;
    int openMode = 0;

    int isExclusive = (flags & SQLITE_OPEN_EXCLUSIVE);
    int isDelete = (flags & SQLITE_OPEN_DELETEONCLOSE);
    int isCreate = (flags & SQLITE_OPEN_CREATE);
    int isReadonly = (flags & SQLITE_OPEN_READONLY);
    int isReadWrite = (flags & SQLITE_OPEN_READWRITE);

    /* If argument zPath is a NULL pointer, this function is required to open
      ** a temporary file. Use this buffer to store the file name in.
      */
    char zTmpname[AWTK_MAX_PATHNAME + 2];

    p = (AWTK_SQLITE_FILE_T *)file_id;

    /* Check the following statements are true:
      **
      **   (a) Exactly one of the READWRITE and READONLY flags must be set, and
      **   (b) if CREATE is set, then READWRITE must also be set, and
      **   (c) if EXCLUSIVE is set, then CREATE must also be set.
      **   (d) if DELETEONCLOSE is set, then CREATE must also be set.
      */
    assert((isReadonly == 0 || isReadWrite == 0) && (isReadWrite || isReadonly));
    assert(isCreate == 0 || isReadWrite);
    assert(isExclusive == 0 || isCreate);
    assert(isDelete == 0 || isCreate);

    /* The main DB, main journal, WAL file and master journal are never
      ** automatically deleted. Nor are they ever temporary files.  */
    assert((!isDelete && file_path) || eType != SQLITE_OPEN_MAIN_DB);
    assert((!isDelete && file_path) || eType != SQLITE_OPEN_MAIN_JOURNAL);
    assert((!isDelete && file_path) || eType != SQLITE_OPEN_MASTER_JOURNAL);
    assert((!isDelete && file_path) || eType != SQLITE_OPEN_WAL);

    /* Assert that the upper layer has set one of the "file-type" flags. */
    assert(eType == SQLITE_OPEN_MAIN_DB || eType == SQLITE_OPEN_TEMP_DB ||
           eType == SQLITE_OPEN_MAIN_JOURNAL || eType == SQLITE_OPEN_TEMP_JOURNAL ||
           eType == SQLITE_OPEN_SUBJOURNAL || eType == SQLITE_OPEN_MASTER_JOURNAL ||
           eType == SQLITE_OPEN_TRANSIENT_DB || eType == SQLITE_OPEN_WAL);

    /* Database filenames are double-zero terminated if they are not
      ** URIs with parameters.  Hence, they can always be passed into
      ** sqlite3_uri_parameter(). */
    assert((eType != SQLITE_OPEN_MAIN_DB) || (flags & SQLITE_OPEN_URI) ||
           file_path[strlen(file_path) + 1] == 0);

    memset(p, 0, sizeof(AWTK_SQLITE_FILE_T));
    if (!file_path) {
        rc = _awtk_get_temp_name(AWTK_MAX_PATHNAME + 2, zTmpname);
        if (rc != SQLITE_OK) {
            return rc;
        }
        file_path = zTmpname;

        /* Generated temporary filenames are always double-zero terminated
            ** for use by sqlite3_uri_parameter(). */
        assert(file_path[strlen(file_path) + 1] == 0);
    }

    /* Determine the value of the flags parameter passed to POSIX function
      ** open(). These must be calculated even if open() is not called, as
      ** they may be stored as part of the file handle and used by the
      ** 'conch file' locking functions later on.  */
    if (isReadonly) {
        openFlags |= O_RDONLY;
    }
    if (isReadWrite) {
        openFlags |= O_RDWR;
    }
    if (isCreate) {
        openFlags |= O_CREAT;
    }
    if (isExclusive) {
        openFlags |= (O_EXCL | O_NOFOLLOW);
    }
    openFlags |= (O_LARGEFILE | O_BINARY);

    fs_file_t *fd = _awtk_fs_open(file_path, openFlags, openMode);

    if (fd == NULL && (errno != -EISDIR) && isReadWrite && !isExclusive) {
        /* Failed to open the file for read/write access. Try read-only. */
        flags &= ~(SQLITE_OPEN_READWRITE | SQLITE_OPEN_CREATE);
        openFlags &= ~(O_RDWR | O_CREAT);
        flags |= SQLITE_OPEN_READONLY;
        openFlags |= O_RDONLY;
        isReadonly = 1;
        fd = _awtk_fs_open(file_path, openFlags, openMode);
    }

    if (fd == NULL) {
        rc = _AWTK_LOG_ERROR(SQLITE_CANTOPEN_BKPT, "open", file_path);
        return rc;
    }

    if (pOutFlags) {
        *pOutFlags = flags;
    }

    if (isDelete) {
        fs_remove_file(os_fs(), file_path);
    }

    p->fd = fd;
    p->pMethod = &_awtk_io_method;
    p->eFileLock = NO_LOCK;
    p->szChunk = 0;
    p->pvfs = pvfs;
    p->sem = tk_semaphore_create(1, "vfssem");

    return rc;
}

int _awtk_vfs_delete(sqlite3_vfs *pvfs, const char *file_path, int syncDir)
{
    int rc = SQLITE_OK;

    if (fs_remove_file(os_fs(), file_path) == (-1)) {
        if (errno == -ENOENT) {
            rc = SQLITE_IOERR_DELETE_NOENT;
        } else {
            rc = _AWTK_LOG_ERROR(SQLITE_IOERR_DELETE, "unlink", file_path);
        }

        return rc;
    }

    // sync dir: open dir -> fsync -> close
    if ((syncDir & 1) != 0) {
        int ii;
        fs_file_t *fd = NULL;
        char zDirname[AWTK_MAX_PATHNAME + 1];

        sqlite3_snprintf(AWTK_MAX_PATHNAME, zDirname, "%s", file_path);
        for (ii = (int)strlen(zDirname); ii > 1 && zDirname[ii] != '/'; ii--)
            ;

        if (ii > 0) {
            zDirname[ii] = '\0';
            fd = _awtk_fs_open(zDirname, O_RDONLY | O_BINARY, 0);
        }

        if (fd != NULL) {
            if (fs_file_sync(fd)) {
                rc = _AWTK_LOG_ERROR(SQLITE_IOERR_DIR_FSYNC, "fsync", file_path);
            }

            fs_file_close(fd);
        }

        rc = SQLITE_OK;
    }

    return rc;
}

static int _awtk_vfs_access(sqlite3_vfs *pvfs, const char *file_path, int flags, int *pResOut)
{
    int amode = 0;

#ifndef F_OK
#define F_OK 0
#endif
#ifndef R_OK
#define R_OK 4
#endif
#ifndef W_OK
#define W_OK 2
#endif

    switch (flags) {
    case SQLITE_ACCESS_EXISTS:
        amode = F_OK;
        break;

    case SQLITE_ACCESS_READWRITE:
        amode = W_OK | R_OK;
        break;

    case SQLITE_ACCESS_READ:
        amode = R_OK;
        break;

    default:
        _AWTK_LOG_ERROR(flags, "access", file_path);
        return -1;
    }

    *pResOut = (_Access(file_path, amode) == 0);

    if (flags == SQLITE_ACCESS_EXISTS && *pResOut) {
        fs_stat_info_t buf;

        if (0 == fs_stat(os_fs(), file_path, &buf) && (buf.size == 0)) {
            *pResOut = 0;
        }
    }

    return SQLITE_OK;
}

static int _awtk_vfs_fullpathname(sqlite3_vfs *pvfs, const char *file_path, int nOut, char *zOut)
{
    if (path_normalize(file_path, zOut, nOut) == RET_OK) {
        return SQLITE_OK;
    }
    return SQLITE_FAIL;
}

static int _awtk_vfs_randomness(sqlite3_vfs *pvfs, int nByte, char *zOut)
{
    assert((size_t)nByte >= (sizeof(time_t) + sizeof(int)));

    memset(zOut, 0, nByte);
    {
        int i;
        char tick8, tick16;
        uint64_t now = time_now_ms();

        tick8 = (char)now;
        tick16 = (char)(now >> 8);

        for (i = 0; i < nByte; i++) {
            zOut[i] = (char)(i ^ tick8 ^ tick16);
            tick8 = zOut[i];
            tick16 = ~(tick8 ^ tick16);
        }
    }

    return nByte;
}

static int _awtk_vfs_sleep(sqlite3_vfs *pvfs, int microseconds)
{
    int millisecond = (microseconds + 999) / 1000;

    sleep_ms(microseconds);

    return millisecond * 1000;
}

static int _awtk_vfs_current_time_int64(sqlite3_vfs *, sqlite3_int64 *);
static int _awtk_vfs_current_time(sqlite3_vfs *pvfs, double *pnow)
{
    sqlite3_int64 i = 0;
    int rc;

    rc = _awtk_vfs_current_time_int64(0, &i);

    *pnow = i / 86400000.0;

    return rc;
}

static int _awtk_vfs_get_last_error(sqlite3_vfs *pvfs, int nBuf, char *zBuf)
{
    return 0;
}

static int _awtk_vfs_current_time_int64(sqlite3_vfs *pvfs, sqlite3_int64 *pnow)
{
    static const sqlite3_int64 awtkEpoch = 24405875 * (sqlite3_int64)8640000;
    int rc = SQLITE_OK;
    /*TODO*/
    *pnow = awtkEpoch + time_now_ms();
    return rc;
}

static int _awtk_vfs_set_system_call(sqlite3_vfs *pvfs, const char *file_path,
                                     sqlite3_syscall_ptr pfn)
{
    return SQLITE_NOTFOUND;
}

static sqlite3_syscall_ptr _awtk_vfs_get_system_call(sqlite3_vfs *pvfs, const char *file_path)
{
    return 0;
}

static const char *_awtk_vfs_next_system_call(sqlite3_vfs *pvfs, const char *file_path)
{
    return 0;
}

/*
** Initialize and deinitialize the operating system interface.
*/
SQLITE_API int sqlite3_os_init(void)
{
    static sqlite3_vfs _awtk_vfs = {
        3,                            /* iVersion */
        sizeof(AWTK_SQLITE_FILE_T),   /* szOsFile */
        AWTK_MAX_PATHNAME,            /* mxPathname */
        0,                            /* pNext */
        "awtk",                       /* zName */
        0,                            /* pAppData */
        _awtk_vfs_open,               /* xOpen */
        _awtk_vfs_delete,             /* xDelete */
        _awtk_vfs_access,             /* xAccess */
        _awtk_vfs_fullpathname,       /* xFullPathname */
        0,                            /* xDlOpen */
        0,                            /* xDlError */
        0,                            /* xDlSym */
        0,                            /* xDlClose */
        _awtk_vfs_randomness,         /* xRandomness */
        _awtk_vfs_sleep,              /* xSleep */
        _awtk_vfs_current_time,       /* xCurrentTime */
        _awtk_vfs_get_last_error,     /* xGetLastError */
        _awtk_vfs_current_time_int64, /* xCurrentTimeInt64 */
        _awtk_vfs_set_system_call,    /* xSetSystemCall */
        _awtk_vfs_get_system_call,    /* xGetSystemCall */
        _awtk_vfs_next_system_call,   /* xNextSystemCall */
    };

    sqlite3_vfs_register(&_awtk_vfs, 1);

    sqlite3MemSetDefault();

    return SQLITE_OK;
}

SQLITE_API int sqlite3_os_end(void)
{
    return SQLITE_OK;
}

#endif /* SQLITE_OS_AWTK */
