#ifndef _SQLITE_CONFIG_AWTK_H_
#define _SQLITE_CONFIG_AWTK_H_
/*
* SQLite compile macro
*/
#ifndef SQLITE_MINIMUM_FILE_DESCRIPTOR
#define SQLITE_MINIMUM_FILE_DESCRIPTOR 0
#endif

#define SQLITE_OMIT_LOAD_EXTENSION 0

#define SQLITE_OMIT_WAL 1

#define SQLITE_OMIT_AUTOINIT 1

#ifndef SQLITE_AWTK_NO_WIDE
#define SQLITE_AWTK_NO_WIDE 1
#endif

#ifndef SQLITE_TEMP_STORE
#define SQLITE_TEMP_STORE 1
#endif

#ifndef SQLITE_THREADSAFE
#define SQLITE_THREADSAFE 1
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

#ifndef SQLITE_OS_AWTK
#define SQLITE_OS_AWTK 1
#endif

#endif
