#include <string.h>
#include "printf.h"
#include "sqlite3.h"


int sqlite3_key(sqlite3 *db, const void *key, int len);
sqlite3 *db;
char *errMsg;

void TestCreateTable()
{
    int rc = sqlite3_exec(db, "CREATE TABLE students(number varchar(10), name varchar(10),sex varchar(6), age varchar(6));", NULL, NULL, NULL);
    if (rc) {
        int errcode = sqlite3_errcode(db);
        printf("Create tbl fail:%s %i\n", sqlite3_errmsg(db), errcode);
        if (errcode == 1) //table exist.
            ;
    }
}
void insertRec()
{
    int rc = sqlite3_exec(db, "INSERT INTO students VALUES('00001', 'Mary', 'female', 15);\
                           INSERT INTO students VALUES('00002', 'John', 'male', 16);\
                           INSERT INTO students VALUES('00003', 'Mike', 'male', 15);\
                           INSERT INTO students VALUES('00004', 'Kevin', 'male', 17);\
                           INSERT INTO students VALUES('00005', 'Alice', 'female', 14);\
                           INSERT INTO students VALUES('00006', 'Susan', 'female', 16);\
                           INSERT INTO students VALUES('00007', 'Christina', 'female', 15);\
                           INSERT INTO students VALUES('00008', 'Brian', 'male', 16);\
                           INSERT INTO students VALUES('00009', 'Dennis', 'male', 14);\
                           INSERT INTO students VALUES('00010', '中山', 'female', 18);",
                          NULL, NULL, &errMsg);
    if (rc) {
        printf("insert record:%s\n", sqlite3_errmsg(db));
    }
}
int ResCB(void *data, int n_columns, char **column_values, char **column_names)
{
    int i;

    for (i = 0; i < n_columns; ++i) {
        printf("%10s:%10s\n", column_names[i], column_values[i]);
        if ((i % 4) == 3) {
            printf("\n");
        }
    }

    return 0;
}
void SearchRC()
{
    char *sql = "SELECT students.* FROM students WHERE sex='female';";
    sqlite3_stmt *stmt;

    sqlite3_prepare_v2(db, sql, -1, &stmt, NULL);
    while (sqlite3_step(stmt) == SQLITE_ROW) {
        const unsigned char *number = sqlite3_column_text(stmt, 0);
        const unsigned char *name   = sqlite3_column_text(stmt, 1);
        const unsigned char *sex    = sqlite3_column_text(stmt, 2);
        const unsigned char *age    = sqlite3_column_text(stmt, 3);
        printf("number:%10s name:%10s sex:%10s age:%10s\n", number, name, sex, age);
    }
    sqlite3_finalize(stmt);
}
void SearchRC2()
{
    /* char *sql = "SELECT OfficialWord.* FROM OfficialWord;"; */
    char *sql = "SELECT students.* FROM students WHERE sex='female';";
    sqlite3_stmt *stmt;

    sqlite3_prepare_v2(db, sql, -1, &stmt, NULL);
    while (sqlite3_step(stmt) == SQLITE_ROW) {
        const unsigned char *number = sqlite3_column_text(stmt, 0);
        const unsigned char *name   = sqlite3_column_text(stmt, 1);
        const unsigned char *sex    = sqlite3_column_text(stmt, 2);
        const unsigned char *age    = sqlite3_column_text(stmt, 3);
        printf("number:%10s name:%10s sex:%10s age:%10s\n", number, name, sex, age);
    }
    sqlite3_finalize(stmt);
}
void SearchRC3()
{
    char *sql = "SELECT name FROM sqlite_master WHERE type='pragma';";// "PRAGMA cipher_hmac_algorithm;";
    sqlite3_stmt *stmt;

    sqlite3_prepare_v2(db, sql, -1, &stmt, NULL);
    while (sqlite3_step(stmt) == SQLITE_ROW) {
        const unsigned char *number = sqlite3_column_text(stmt, 0);
        const unsigned char *name   = sqlite3_column_text(stmt, 1);
        const unsigned char *sex    = sqlite3_column_text(stmt, 2);
        const unsigned char *age    = sqlite3_column_text(stmt, 3);
        printf("number:%10s name:%10s sex:%10s age:%10s\n", number, name, sex, age);
    }
    sqlite3_finalize(stmt);
}

void listTable()
{
    char *sql = "SELECT name FROM sqlite_master WHERE type='table'";
    sqlite3_stmt *stmt;

    sqlite3_prepare_v2(db, sql, -1, &stmt, NULL);
    while (sqlite3_step(stmt) == SQLITE_ROW) {
        const unsigned char *number = sqlite3_column_text(stmt, 0);
        const unsigned char *name   = sqlite3_column_text(stmt, 1);
        printf("number:%10s name:%10s \n", number, name);
    }
    sqlite3_finalize(stmt);
}
int sqlite_test_main()
{
    int i, rc;
    char *zErrMsg = 0;
#if 0
    /* rc = sqlite3_open("d:\\example.db", &db); */
    rc = sqlite3_open("storage/sd0/C/1112221.db", &db);
    if (rc) {
        printf("open db fail.:%s\n", sqlite3_errmsg(db));
        return -1;
    } else {
        printf("Open db success\n");
    }
    rc = sqlite3_key(db, "abcd1234", 8);
    listTable();
    TestCreateTable();
    rc = sqlite3_exec(db, "delete from students;", NULL, NULL, &errMsg);
    if (rc) {
        printf("exec db fail.:%s\n", sqlite3_errmsg(db));
    }
    insertRec();
    rc = sqlite3_exec(db, "SELECT students.* FROM students WHERE sex='female';", ResCB, NULL, &errMsg);
    rc = sqlite3_exec(db, "delete from students where name='Mary';", NULL, NULL, &errMsg);
    printf("\nDelete Mary res:%i %s\n", rc, errMsg);
    SearchRC();
    rc = sqlite3_exec(db, "update students set age = age+1;", NULL, NULL, &errMsg);
    printf("\nupdate age:%i %s\n", rc, errMsg);
    SearchRC();
    rc = sqlite3_exec(db, "SELECT students.* FROM students WHERE age>15;", ResCB, NULL, &errMsg);

    /*
    密钥：abcd1234
    PRAGMA cipher_page_size = 4096;
    PRAGMA kdf_iter = 5120;
    PRAGMA cipher_hmac_algorithm = 'HMAC_SHA256';
    PRAGMA cipher_kdf_algorithm = 'PBKDF2_HMAC_SHA256';
    */
    /* rc=sqlite3_open("d://dbe4.db", &db); */
#else
    /* rc = sqlite3_open("storage/sd0/C/dbe4.db", &db); */
    rc = sqlite3_open("storage/sd0/C/1112221.db", &db);
    if (rc) {
        printf("open db fail.:%s\n", sqlite3_errmsg(db));
        return -1;
    } else {
        printf("Open db success\n");
    }

    rc = sqlite3_key(db, "abcd1234", 8);

    if (rc) {
        printf("key fail.:%s\n", sqlite3_errmsg(db));
        return -1;
    }

    rc = sqlite3_exec(db, "PRAGMA legacy_page_size = 4096;", NULL, NULL, &zErrMsg);
    if (rc != SQLITE_OK) {
        printf("SQL错误: %s\n", zErrMsg);
        sqlite3_free(zErrMsg);
    }
    rc = sqlite3_exec(db, "PRAGMA kdf_iter = 5120;", NULL, NULL, &zErrMsg);
    if (rc != SQLITE_OK) {
        printf("SQL错误: %s\n", zErrMsg);
        sqlite3_free(zErrMsg);
    }
    rc = sqlite3_exec(db, "PRAGMA cipher_hmac_algorithm = 'HMAC_SHA256';", NULL, NULL, &zErrMsg);
    if (rc != SQLITE_OK) {
        printf("SQL错误: %s\n", zErrMsg);
        sqlite3_free(zErrMsg);
    }
    rc = sqlite3_exec(db, "PRAGMA cipher_kdf_algorithm = 'PBKDF2_HMAC_SHA256';", NULL, NULL, &zErrMsg);
    if (rc != SQLITE_OK) {
        printf("SQL错误: %s\n", zErrMsg);
        sqlite3_free(zErrMsg);
    }

    listTable();
    SearchRC2();
    /* rc = sqlite3_exec(db, "SELECT OfficialWord.* FROM OfficialWord;", ResCB, NULL, &errMsg); */
    rc = sqlite3_exec(db, "SELECT students.* FROM students WHERE sex='female';", ResCB, NULL, &errMsg);
    if (rc) {
        printf("Search fail.:%s\n", sqlite3_errmsg(db));
        sqlite3_close(db);
        return -1;
    }
#endif

    sqlite3_close(db);
    return 0;
}

void execute_sql(sqlite3 *db, const char *sql, const char *success_msg)
{
    char *err_msg = NULL;
    int rc = sqlite3_exec(db, sql, NULL, NULL, &err_msg);
    if (rc != SQLITE_OK) {
        printf("SQL error: %s rc:%d \n", err_msg, rc);
        sqlite3_free(err_msg);
    } else if (success_msg) {
        printf("%s\n", success_msg);
    }
}

void print_query_results(sqlite3 *db, const char *sql)
{
    sqlite3_stmt *stmt;
    if (sqlite3_prepare_v2(db, sql, -1, &stmt, NULL) != SQLITE_OK) {
        printf("Failed to prepare statement: %s\n", sqlite3_errmsg(db));
        return;
    }

    while (sqlite3_step(stmt) == SQLITE_ROW) {
        printf("id = %d ", sqlite3_column_int(stmt, 0));
        printf("name = %s ", sqlite3_column_text(stmt, 1));
        printf("age = %d ", sqlite3_column_int(stmt, 2));
        printf("sex = %s\n\n", sqlite3_column_text(stmt, 3));
    }

    sqlite3_finalize(stmt);
}

int sqlite_test(void)
{
    /* sqlite3 *db; */
    int rc = sqlite3_open("storage/sd0/C/7example1.db", &db);

    if (rc) {
        printf("Can't open database: %s\n", sqlite3_errmsg(db));
        return 1;
    } else {
        printf("Opened database successfully\n");
    }

// 设置密码
    const char *key = "my_secure_key";
    rc = sqlite3_key(db, key, strlen(key));
    if (rc != SQLITE_OK) {
        printf("Failed to set encryption key: %s\n", sqlite3_errmsg(db));
        return 1;
    }

    // 创建表
    const char *create_table_sql =
        "CREATE TABLE IF NOT EXISTS person ("
        "id INTEGER PRIMARY KEY AUTOINCREMENT, "
        "name TEXT NOT NULL, "
        "age INTEGER NOT NULL, "
        "sex TEXT NOT NULL);";
    execute_sql(db, create_table_sql, "Table created successfully");

    // 插入数据
    const char *insert_data_sql =
        "INSERT INTO person (name, age, sex) "
        "SELECT 'Alice', 21, 'female' WHERE NOT EXISTS ("
        "SELECT 1 FROM person WHERE name = 'Alice' AND age = 21 AND sex = 'female');";
    execute_sql(db, insert_data_sql, "Data inserted successfully");

    execute_sql(db,
                "INSERT INTO person (name, age, sex) "
                "SELECT 'Bob', 22, 'male' WHERE NOT EXISTS ("
                "SELECT 1 FROM person WHERE name = 'Bob' AND age = 22 AND sex = 'male');",
                "Data inserted successfully");

    execute_sql(db,
                "INSERT INTO person (name, age, sex) "
                "SELECT 'Charlie', 19, 'male' WHERE NOT EXISTS ("
                "SELECT 1 FROM person WHERE name = 'Charlie' AND age = 19 AND sex = 'male');",
                "Data inserted successfully");

    // 查询数据
    printf("Query results:\n");
    print_query_results(db, "SELECT * FROM person;");

    // 更新数据
    const char *update_data_sql =
        "UPDATE person SET age = age + 1 WHERE name = 'Alice';";
    execute_sql(db, update_data_sql, "Data updated successfully");

    // 删除数据
    const char *delete_data_sql =
        "DELETE FROM person WHERE name = 'Bob';";
    execute_sql(db, delete_data_sql, "Data deleted successfully");

    // 查询更新和删除后的数据
    printf("Query results after update and delete:\n");
    print_query_results(db, "SELECT * FROM person;");

    // 关闭数据库
    sqlite3_close(db);
    printf("Database closed successfully\n");
    return 0;
}

