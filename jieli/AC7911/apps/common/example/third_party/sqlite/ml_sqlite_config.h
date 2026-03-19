#ifndef _ML_SQLITE_CONFIG_H_
#define _ML_SQLITE_CONFIG_H_

/*
* SQLite compile macro
*/
#ifndef SQLITE_MINIMUM_FILE_DESCRIPTOR
#define SQLITE_MINIMUM_FILE_DESCRIPTOR  0
#endif

#define SQLITE_OMIT_LOAD_EXTENSION 1

#define SQLITE_OMIT_WAL 1

//#define SQLITE_OMIT_AUTOINIT 1

#ifndef SQLITE_RTTHREAD_NO_WIDE
#define SQLITE_RTTHREAD_NO_WIDE 1
#endif

#ifndef SQLITE_TEMP_STORE
#define SQLITE_TEMP_STORE 1
#endif

#ifndef SQLITE_THREADSAFE
//#define SQLITE_THREADSAFE 1
#define SQLITE_THREADSAFE 0
#endif

#ifndef HAVE_READLINE
#define HAVE_READLINE 0
#endif

#ifndef NDEBUG
#define NDEBUG
#endif

#ifndef SQLITE_OS_OTHER
#define SQLITE_OS_OTHER 1
#endif

#define SQLITE_MUTEX_RTTHREAD 1
#define SQLITE_OS_RTTHREAD 1
//efine SQLITE_MUTEX_PTHREADS
#define SQLCIPHER_OMIT_LOG 1

/*
#define BUILD_sqlite -DNDEBUG
//#define SQLITE_OMIT_LOAD_EXTENSION           1
#define SQLITE_DQS                           0
//#define SQLITE_OS_OTHER                      1
#define SQLITE_NO_SYNC                       1
#define SQLITE_TEMP_STORE                    1
//#define SQLITE_DISABLE_LFS                   1
#define SQLITE_DISABLE_DIRSYNC               1
#define SQLITE_SECURE_DELETE                 0
#define SQLITE_DEFAULT_LOOKASIDE        512,64
#define YYSTACKDEPTH                        20
#define SQLITE_SMALL_STACK                   1
#define SQLITE_SORTER_PMASZ                  4
#define SQLITE_DEFAULT_CACHE_SIZE           -1
#define SQLITE_DEFAULT_MEMSTATUS             0
#define SQLITE_DEFAULT_MMAP_SIZE             0
#define SQLITE_CORE                          1
//#define SQLITE_THREADSAFE                    0
#define SQLITE_MUTEX_APPDEF                  1
#define SQLITE_OMIT_WAL                      1
//#define SQLITE_DISABLE_FTS3_UNICODE          1
//#define SQLITE_DISABLE_FTS4_DEFERRED         1
#define SQLITE_LIKE_DOESNT_MATCH_BLOBS       1
#define SQLITE_DEFAULT_FOREIGN_KEYS          0
#define SQLITE_DEFAULT_LOCKING_MODE          1
#define SQLITE_DEFAULT_PAGE_SIZE           512
#define SQLITE_DEFAULT_PCACHE_INITSZ         8
#define SQLITE_MAX_DEFAULT_PAGE_SIZE     32768
#define SQLITE_POWERSAFE_OVERWRITE           1
#define SQLITE_MAX_EXPR_DEPTH                0
*/


#endif
