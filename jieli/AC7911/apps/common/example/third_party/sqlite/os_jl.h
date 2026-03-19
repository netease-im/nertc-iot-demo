/*

 2025-01-4

*/
#ifndef SQLITE_OS_JL_H
#define SQLITE_OS_JL_H

#ifndef SQLITE_OS_JL
#define SQLITE_OS_JL 1
#endif

#include "sqlite3.h"
#include "fs/fs.h"
typedef unsigned long DWORD;

typedef struct jlVfsAppData jlVfsAppData;
struct jlVfsAppData {
    const sqlite3_io_methods *pMethod; /* The file I/O methods to use. */
    void *pAppData;                    /* The extra pAppData, if any. */
    int bNoLock;                      /* Non-zero if locking is disabled. */
};

#if 0
typedef struct jlFile jlFile;
struct jlFile {
    const sqlite3_io_methods *pMethod; /*** Must be first ***/
    sqlite3_vfs *pVfs;      /* The VFS used to open this file */
#if 0
    HANDLE h;               /* Handle for accessing the file */
#else
    FILE *h;
#endif
    const char *zPath;      /* Full pathname of this file */
};
#endif

#endif /* SQLITE_OS_JL_H */
