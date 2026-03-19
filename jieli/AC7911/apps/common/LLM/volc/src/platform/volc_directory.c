#include "volc_directory.h"
#include "app_config.h"
#include <sys/stat.h>

#include <errno.h>
#include "fs/fs.h"
#include "os/os_api.h"
#include "volc_errno.h"
#include "volc_type.h"
#include "volc_memory.h"
#include "volc_fileio.h"


char *get_basename(const char *path)
{
    if (!path || *path == '\0') {
        return NULL;
    }

    char *tmp = strdup(path);
    if (!tmp) {
        return NULL;
    }

    // 去掉末尾所有 '/'
    size_t len = strlen(tmp);
    while (len > 0 && tmp[len - 1] == '/') {
        tmp[--len] = '\0';
    }

    // 处理全 '/' 路径（如 "/" 或 "////"）
    if (len == 0) {
        free(tmp);
        return strdup("/");
    }

    // 查找最后一个 '/'
    char *last_slash = strrchr(tmp, '/');
    char *basename;

    if (last_slash) {
        // 直接取最后一个 '/' 后的部分
        basename = strdup(last_slash + 1);
    } else {
        // 无 '/'，路径本身即为名称
        basename = strdup(tmp);
    }

    free(tmp);
    return basename;
}

char *get_modified_basename(const char *path)
{
    if (!path || *path == '\0') {
        return NULL;
    }

    char *tmp = strdup(path);
    if (!tmp) {
        return NULL;
    }

    // 去掉末尾的 '/'
    size_t len = strlen(tmp);
    while (len > 0 && tmp[len - 1] == '/') {
        tmp[--len] = '\0';
    }

    if (len == 0) {
        free(tmp);
        return strdup("/"); // 根目录特殊处理
    }

    char *last_slash = strrchr(tmp, '/');
    char *result;

    if (last_slash) {
        // 返回包括 '/' 的名称（如 "/c"）。
        // 若路径以 '/' 分割，需要确保 tmp 中还有前面的内容
        if (last_slash == tmp) { // 路径为 "/xxx"
            result = strdup(last_slash);
        } else {
            result = strdup(last_slash);
        }
    } else {
        // 无 '/'，返回原路径
        result = strdup(tmp);
    }

    free(tmp);
    return result;
}



void normalize_path(char *path)
{
    if (!path || *path == '\0') {
        return;
    }

    char *read_ptr = path;    // 读指针
    char *write_ptr = path;   // 写指针

    while (*read_ptr != '\0') {
        // 将当前字符复制到写指针位置
        *write_ptr = *read_ptr++;

        // 如果当前字符是 '/'，则跳过后续连续 '/'
        if (*write_ptr == '/') {
            while (*read_ptr == '/') {
                read_ptr++;
            }
        }
        write_ptr++;  // 移动写指针
    }

    *write_ptr = '\0';  // 结束字符串
}

int mkdir(const char *path, mode_t mode)
{
    volc_debug("%s, %s\n", __FUNCTION__, path);
    char *newPath = replacePrefix(path, "./", CONFIG_ROOT_PATH);
    normalize_path(newPath);
    volc_debug("new path: %s\n", newPath);
    free(newPath);

    return 0;
}

int remove(const char *path)
{
    volc_debug("%s, %s\n", __FUNCTION__, path);
    char *newPath = replacePrefix(path, "./", CONFIG_ROOT_PATH);
    normalize_path(newPath);
    volc_debug("new path: %s\n", newPath);
    free(newPath);

    return 0;
}

static uint32_t _volc_remove_file_dir(uint64_t data, volc_dir_entry_type_e type, char *path, char *name)
{
    volc_debug("%s, %s type:%d \n", __FUNCTION__, path, type);
    char *newPath = replacePrefix(path, "./", CONFIG_ROOT_PATH);
    normalize_path(newPath);
    volc_debug("new path: %s\n", newPath);
    VOLC_UNUSED_PARAM(data);
    VOLC_UNUSED_PARAM(name);
    uint32_t ret = VOLC_STATUS_SUCCESS;

    switch (type) {
    case VOLC_DIR_ENTRY_TYPE_DIRECTORY:
        VOLC_CHK(0 == fdelete_dir(newPath), VOLC_STATUS_REMOVE_DIRECTORY_FAILED);
        break;
    case VOLC_DIR_ENTRY_TYPE_FILE:
        FILE *fp = fopen(newPath, "r+");
        VOLC_CHK(0 == fdelete(fp), VOLC_STATUS_REMOVE_FILE_FAILED);
        break;
    case VOLC_DIR_ENTRY_TYPE_LINK:
//            VOLC_CHK(0 == unlink(newPath), VOLC_STATUS_REMOVE_LINK_FAILED);
        break;
    default:
        VOLC_CHK(0, VOLC_STATUS_UNKNOWN_DIR_ENTRY_TYPE);
    }

err_out_label:
    free(newPath);
    return ret;
}

static uint32_t _volc_get_file_dir_size(uint64_t data, volc_dir_entry_type_e type, char *path, char *name)
{
    volc_debug("%s, %s\n", __FUNCTION__, path);

    char *newPath = replacePrefix(path, "./", CONFIG_ROOT_PATH);
    normalize_path(newPath);
    volc_debug("new path: %s\n", newPath);

    VOLC_UNUSED_PARAM(name);
    uint64_t size = 0;
    uint64_t *p_size = (uint64_t *) data;
    uint32_t ret = VOLC_STATUS_SUCCESS;

    switch (type) {
    case VOLC_DIR_ENTRY_TYPE_DIRECTORY:
        break;
    case VOLC_DIR_ENTRY_TYPE_FILE:
        VOLC_CHK_STATUS(volc_file_get_length(newPath, &size));
        break;
    case VOLC_DIR_ENTRY_TYPE_LINK:
        break;
    default:
        VOLC_CHK(0, VOLC_STATUS_UNKNOWN_DIR_ENTRY_TYPE);
    }

    *p_size += size;

err_out_label:
    free(newPath);
    return ret;
}

uint32_t volc_traverse_directory(const char *path, uint64_t data, bool iterate, volc_directory_entry_callback entry)
{
    volc_debug("%s, %s\n", __FUNCTION__, path);
    char *newPath = replacePrefix(path, "./", CONFIG_ROOT_PATH);
    normalize_path(newPath);
    volc_debug("new path: %s\n", newPath);
    free(newPath);
#if 0
    uint32_t ret = VOLC_SUCCESS;
    char temp_file_name[VOLC_MAX_PATH_LEN];
    uint32_t path_len = 0;

    uint32_t dir_path_len;
    DIR *dir = NULL;
    struct dirent *dir_ent = NULL;
    struct stat entry_stat;

    VOLC_CHK(path != NULL && entry != NULL && path[0] != '\0', VOLC_STATUS_INVALID_ARG);

    // Ensure we don't get a very long paths. Need at least a separator and a null terminator.
    path_len = (uint32_t) strlen(path);
    VOLC_CHK(path_len + 2 < VOLC_MAX_PATH_LEN, VOLC_STATUS_PATH_TOO_LONG);

    // Ensure the path is appended with the separator
    strcpy(temp_file_name, path);

    if (temp_file_name[path_len - 1] != VOLC_FPATHSEPARATOR) {
        temp_file_name[path_len] = VOLC_FPATHSEPARATOR;
        path_len++;
        temp_file_name[path_len] = '\0';
    }

    dir = opendir(temp_file_name);

    // Need to make a distinction between various types of failures
    if (dir == NULL) {
        switch (errno) {
        case EACCES:
            VOLC_CHK(0, VOLC_STATUS_DIRECTORY_ACCESS_DENIED);
        case ENOENT:
            VOLC_CHK(0, VOLC_STATUS_DIRECTORY_MISSING_PATH);
        default:
            VOLC_CHK(0, VOLC_STATUS_DIRECTORY_OPEN_FAILED);
        }
    }

    while (NULL != (dir_ent = readdir(dir))) {
        if ((0 == strcmp(dir_ent->d_name, ".")) || (0 == strcmp(dir_ent->d_name, ".."))) {
            continue;
        }

        // Prepare the path
        temp_file_name[path_len] = '\0';

        // Check if it's a directory, link, file or unknown
        strncat(temp_file_name, dir_ent->d_name, VOLC_MAX_PATH_LEN - path_len);
        VOLC_CHK(0 == stat(temp_file_name, &entry_stat), VOLC_STATUS_DIRECTORY_ENTRY_STAT_ERROR);

        if (S_ISREG(entry_stat.st_mode)) {
            VOLC_CHK_STATUS(entry(data, VOLC_DIR_ENTRY_TYPE_FILE, temp_file_name, dir_ent->d_name));
        } else if (S_ISLNK(entry_stat.st_mode)) {
            VOLC_CHK_STATUS(entry(data, VOLC_DIR_ENTRY_TYPE_LINK, temp_file_name, dir_ent->d_name));
        } else if (S_ISDIR(entry_stat.st_mode)) {
            // Append the path separator and null terminate
            dir_path_len = strlen(temp_file_name);
            VOLC_CHK(dir_path_len + 2 < VOLC_MAX_PATH_LEN, VOLC_STATUS_PATH_TOO_LONG);

            // Iterate into sub-directories if specified
            if (iterate) {
                temp_file_name[dir_path_len] = VOLC_FPATHSEPARATOR;
                temp_file_name[dir_path_len + 1] = '\0';

                // Recurse into the directory
                VOLC_CHK_STATUS(volc_traverse_directory(temp_file_name, data, iterate, entry));
            }

            // Remove the path separator
            temp_file_name[dir_path_len] = '\0';
            VOLC_CHK_STATUS(entry(data, VOLC_DIR_ENTRY_TYPE_DIRECTORY, temp_file_name, dir_ent->d_name));
        } else {
            // We treat this as unknown
            VOLC_CHK_STATUS(entry(data, VOLC_DIR_ENTRY_TYPE_UNKNOWN, temp_file_name, dir_ent->d_name));
        }
    }

err_out_label:

    if (dir != NULL) {
        closedir(dir);
        dir = NULL;
    }
    return ret;
#endif // 0
}

uint32_t volc_remove_directory(const char *path)
{
    volc_debug("%s, %s\n", __FUNCTION__, path);

    char *newPath = replacePrefix(path, "./", CONFIG_ROOT_PATH);
    normalize_path(newPath);
    volc_debug("new path: %s\n", newPath);

    uint32_t ret = VOLC_STATUS_SUCCESS;

    VOLC_CHK(newPath != NULL && path[0] != '\0', VOLC_STATUS_INVALID_ARG);
//
//    VOLC_CHK_STATUS(volc_traverse_directory(path, 0, 1, _volc_remove_file_dir));

    // Finally, remove the directory when it's empty
    VOLC_CHK(0 == fdelete_dir(newPath), VOLC_STATUS_REMOVE_DIRECTORY_FAILED);

err_out_label:
    free(newPath);
    return ret;
}

uint32_t volc_create_directory(const char *path)
{
    volc_debug("%s, %s\n", __FUNCTION__, path);

//    char path_[128];

    char *newPath = replacePrefix(path, "./", CONFIG_ROOT_PATH);
    normalize_path(newPath);
    volc_debug("new path: %s\n", newPath);
    char *parent = get_parent_directory(newPath);
    if (parent) {
//        printf("原路径: %s 父目录: %s\n", newPath, parent); // 输出：父目录: storage/sd0/C
//        free(parent);
    }
//    char* p = strrchr(newPath,'/');
//    if(p == NULL) {
//        return -1;
//    }
    char *dirname = get_modified_basename(newPath);
//    size_t len = p - path_;
//    char* parant_path = volc_calloc(1,len + 1);
//    strncpy(parant_path,path_,len);
//    char firname[64];
//    strncpy(firname, p, sizeof(path_)-len);
    volc_debug("parent %s, dirname:%s", parent, dirname);
    int ret = fmk_dir(parent, dirname, 0);
    printf("ret:%d", ret);
//    if (ret == 0) {
    VOLC_SAFE_MEMFREE(parent);
//        volc_debug("fmk dir succ!");
    VOLC_SAFE_MEMFREE(newPath);
    VOLC_SAFE_MEMFREE(dirname);
    return 0;
//    } else {
//        VOLC_SAFE_MEMFREE(parent);
//        volc_debug("fmk dir error!");
//        VOLC_SAFE_MEMFREE(newPath);
//        VOLC_SAFE_MEMFREE(dirname);
//        return -1;
//    }

}

uint32_t volc_get_directory_size(const char *path, uint64_t *p_size)
{
    volc_debug("%s, %s\n", __FUNCTION__, path);
    char *newPath = replacePrefix(path, "./", CONFIG_ROOT_PATH);
    normalize_path(newPath);
    volc_debug("new path: %s\n", newPath);

    uint32_t ret = VOLC_STATUS_SUCCESS;
    uint64_t size = 0;

    VOLC_CHK(p_size != NULL && path != NULL && path[0] != '\0', VOLC_STATUS_INVALID_ARG);

//    VOLC_CHK_STATUS(volc_traverse_directory(path, (uint64_t) &size, 1, _volc_get_file_dir_size));
    *p_size = flen_dir(newPath);
//    *p_size = size;

err_out_label:
    free(newPath);
    return ret;
}
