#include "tkc/platform.h"

extern int sqlite3_demo(const char *db_filename);

int sqlite3_test_main(int argc, char *argv[])
{
    const char *db_filename = "data/test.db";
    platform_prepare();

    return sqlite3_demo(db_filename);
}
