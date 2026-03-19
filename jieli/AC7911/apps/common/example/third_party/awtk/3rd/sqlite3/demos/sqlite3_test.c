#include "sqlite3.h"
#include "tkc/types_def.h"

static int dump_table(void *data, int argc, char **argv, char **azColName)
{
    int i;
    log_info("%s: ", (const char *)data);

    for (i = 0; i < argc; i++) {
        log_info("%s = %s\n", azColName[i], argv[i] ? argv[i] : "NULL");
    }

    log_info("\n");
    return 0;
}

void sqlite3_exec_sql(sqlite3 *db, const char *sql)
{
    char *zErrMsg = NULL;
    int rc = sqlite3_exec(db, sql, dump_table, (void *)"dump", &zErrMsg);
    log_info("###################################################\n");
    if (rc != SQLITE_OK) {
        log_info("SQL error: %s:%s\n", zErrMsg, sql);
        sqlite3_free(zErrMsg);
    } else {
        log_info("Operation done successfully:%s\n", sql);
    }
    log_info("###################################################\n\n");
}

static const char *sql_select = "SELECT * from COMPANY;";
static const char *sql_insert = "INSERT INTO COMPANY (ID,NAME,AGE,ADDRESS,SALARY) VALUES (1234,'abc', 32, 'California', 20000.00 ); ";
static const char *sql_delete = "DELETE from COMPANY where NAME='abc';";

int sqlite3_demo(const char *db_filename)
{
    int rc = 0;
    sqlite3 *db = NULL;

    sqlite3_initialize();

    rc = sqlite3_open(db_filename, &db);
    if (rc) {
        log_warn("Can't open database: %s\n", sqlite3_errmsg(db));
    } else {
        log_warn("Opened database successfully:%s\n", db_filename);

        sqlite3_exec_sql(db, sql_insert);
        sqlite3_exec_sql(db, sql_select);
        sqlite3_exec_sql(db, sql_delete);
        sqlite3_exec_sql(db, sql_select);
    }
    sqlite3_close(db);

    return 0;
}
