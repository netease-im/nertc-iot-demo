#include "volc_fileio.h"

#include <sys/stat.h>
#include <errno.h>
#include <unistd.h>
#include "fs/fs.h"
#include "volc_type.h"
#include "app_config.h"

extern int long_file_name_encode(const char *input, u8 *output, u32 output_buf_len);

char *replacePrefix(const char *src, const char *prefix, const char *newPrefix)
{
    size_t prefixLen = strlen(prefix);
    size_t newPrefixLen = strlen(newPrefix);

    // 检查是否以指定前缀开头
    if (strncmp(src, prefix, prefixLen) != 0) {
        return strdup(src); // 若前缀不匹配，返回原字符串副本
    }

    // 拼接新字符串
    size_t totalLen = newPrefixLen + strlen(src + prefixLen) + 1;
    char *result = malloc(totalLen);
    if (!result) {
        return NULL;
    }

    memcpy(result, newPrefix, newPrefixLen);
    strcpy(result + newPrefixLen, src + prefixLen);

    return result;
}

char *get_parent_directory(const char *path)
{
    if (!path || strlen(path) == 0) {
        return NULL;    // 处理无效输入
    }

    // 创建可修改的副本
    char *tmp = strdup(path);
    if (!tmp) {
        return NULL;
    }

    // 去除末尾的 '/'
    size_t len = strlen(tmp);
    while (len > 0 && tmp[len - 1] == '/') {
        tmp[--len] = '\0';
    }

    // 查找最后一个 '/' 的位置
    char *last_slash = strrchr(tmp, '/');
    char *parent = NULL;

    if (last_slash != NULL) {
        * (last_slash + 1) = '\0'; // 截断到父目录
        // 处理根目录的特殊情况（如 "/usr" → 父目录是 "/"）
        parent = (tmp[0] == '\0') ? strdup("/") : strdup(tmp);
    } else {
        // 无父目录，返回当前目录 "."
        parent = strdup(".");
    }

    free(tmp);
    return parent;
}

uint32_t volc_file_read(const char *path, bool bin_mode, uint8_t *buffer, uint64_t *size)
{
    volc_debug("%s, %s\n", __FUNCTION__, path);

    char *newPath = replacePrefix(path, "./", CONFIG_ROOT_PATH);
    normalize_path(newPath);
    volc_debug("new path: %s\n", newPath);

    uint64_t file_len = 0;
    uint32_t ret = VOLC_STATUS_SUCCESS;
    FILE *fp = NULL;

    VOLC_CHK(newPath != NULL && size != NULL, VOLC_STATUS_NULL_ARG);

    fp = fopen(newPath, bin_mode ? "rb" : "r");

    VOLC_CHK(fp != NULL, VOLC_STATUS_OPEN_FILE_FAILED);

    // Get the size of the file
    fseek(fp, 0, SEEK_END);
    file_len = ftell(fp);

    if (buffer == NULL) {
        // requested the length - set and early return
        *size = file_len;
        VOLC_CHK(0, VOLC_STATUS_SUCCESS);
    }

    // Validate the buffer size
    VOLC_CHK(file_len <= *size, VOLC_STATUS_BUFFER_TOO_SMALL);

    // Read the file into memory buffer
    fseek(fp, 0, SEEK_SET);
    VOLC_CHK((fread(buffer, (size_t) file_len, 1, fp) == 1), VOLC_STATUS_READ_FILE_FAILED);

err_out_label:
    if (fp != NULL) {
        fclose(fp);
        fp = NULL;
    }
    free(newPath);
    return ret;
}

uint32_t volc_file_read_segment(const char *path, bool bin_mode, uint8_t *buffer, uint64_t offset, uint64_t size)
{
    volc_debug("%s, %s\n", __FUNCTION__, path);
    char *newPath = replacePrefix(path, "./", CONFIG_ROOT_PATH);
    normalize_path(newPath);
    volc_debug("new path: %s\n", newPath);

    uint64_t file_len;
    uint32_t ret = VOLC_STATUS_SUCCESS;
    FILE *fp = NULL;
    int32_t result = 0;

    VOLC_CHK(newPath != NULL && buffer != NULL && size != 0, VOLC_STATUS_NULL_ARG);

    fp = fopen(newPath, bin_mode ? "rb" : "r");

    VOLC_CHK(fp != NULL, VOLC_STATUS_OPEN_FILE_FAILED);

    // Get the size of the file
    fseek(fp, 0, SEEK_END);
    file_len = ftell(fp);

    // Check if we are trying to read past the end of the file
    VOLC_CHK(offset + size <= file_len, VOLC_STATUS_READ_FILE_FAILED);

    // Set the offset and read the file content
    result = fseek(fp, (uint32_t) offset, SEEK_SET);
    VOLC_CHK(result == 0 && (fread(buffer, (size_t) size, 1, fp) == 1), VOLC_STATUS_READ_FILE_FAILED);

err_out_label:

    if (fp != NULL) {
        fclose(fp);
        fp = NULL;
    }
    free(newPath);
    return ret;
}

uint32_t volc_file_write(const char *path, bool bin_mode, bool append, uint8_t *buffer, uint64_t size)
{
    volc_debug("%s, %s\n", __FUNCTION__, path);
    uint32_t ret = VOLC_STATUS_SUCCESS;

    VOLC_CHK(path != NULL && buffer != NULL, VOLC_STATUS_NULL_ARG);
    char *newPath = replacePrefix(path, "./", CONFIG_ROOT_PATH);
    normalize_path(newPath);
    volc_debug("new path: %s\n", newPath);

    FILE *fp = NULL;

    fp = fopen(newPath, bin_mode ? (append ? "wb" : "wb+") : (append ? "w" : "w+"));

    VOLC_CHK(fp != NULL, VOLC_STATUS_OPEN_FILE_FAILED);

    // Write the buffer to the file
    VOLC_CHK(fwrite(buffer, (size_t) size, 1, fp) == 1, VOLC_STATUS_WRITE_TO_FILE_FAILED);

err_out_label:

    if (fp != NULL) {
        fclose(fp);
        fp = NULL;
    }
    free(newPath);
    return ret;
}

uint32_t volc_file_update(const char *path, bool bin_mode, uint8_t *buffer, uint64_t offset, uint64_t size)
{
    volc_debug("%s, %s\n", __FUNCTION__, path);

    uint32_t ret = VOLC_STATUS_SUCCESS;
    VOLC_CHK(path != NULL && buffer != NULL, VOLC_STATUS_NULL_ARG);


    char *newPath = replacePrefix(path, "./", CONFIG_ROOT_PATH);
    normalize_path(newPath);
    volc_debug("new path: %s\n", newPath);

    FILE *fp = NULL;
    uint32_t i;
    uint8_t *p_cur_ptr;

    fp = fopen(newPath, bin_mode ? "rb+" : "r+");

    VOLC_CHK(fp != NULL, VOLC_STATUS_OPEN_FILE_FAILED);

    VOLC_CHK(0 == fseek(fp, (uint32_t) offset, SEEK_SET), VOLC_STATUS_INVALID_OPERATION);

    for (i = 0, p_cur_ptr = buffer + offset; i < size; i++, p_cur_ptr++) {
        VOLC_CHK(-1 != fputc(*p_cur_ptr, fp), VOLC_STATUS_WRITE_TO_FILE_FAILED);
    }

err_out_label:

    if (fp != NULL) {
        fclose(fp);
        fp = NULL;
    }
    free(newPath);
    return ret;
}

uint32_t volc_file_get_length(const char *path, uint64_t *p_length)
{
    volc_debug("%s, %s\n", __FUNCTION__, path);
    char *newPath = replacePrefix(path, "./", CONFIG_ROOT_PATH);
    normalize_path(newPath);
    volc_debug("new path: %s\n", newPath);
    uint32_t ret = VOLC_STATUS_SUCCESS;
    VOLC_CHK_STATUS(volc_file_read(newPath, 1, NULL, p_length));
err_out_label:
    free(newPath);
    return ret;
}

uint32_t volc_file_set_length(const char *path, uint64_t length)
{
    volc_debug("%s, %s\n", __FUNCTION__, path);
    char *newPath = replacePrefix(path, "./", CONFIG_ROOT_PATH);
    normalize_path(newPath);
    volc_debug("new path: %s\n", newPath);
    uint32_t ret = VOLC_STATUS_SUCCESS;
    int32_t ret_val, err_code, file_desc;

    VOLC_CHK(path != NULL, VOLC_STATUS_NULL_ARG);

    VOLC_UNUSED_PARAM(file_desc);

    ret_val = truncate(newPath, length);

    err_code = errno;

    if (ret_val == -1) {
        switch (err_code) {
        case EACCES:
            ret = VOLC_STATUS_DIRECTORY_ACCESS_DENIED;
            break;

        case ENOENT:
            ret = VOLC_STATUS_DIRECTORY_MISSING_PATH;
            break;

        case EINVAL:
            ret = VOLC_STATUS_INVALID_ARG_LEN;
            break;

        case EISDIR:
        case EBADF:
            ret = VOLC_STATUS_INVALID_ARG;
            break;

        case ENOSPC:
            ret = VOLC_STATUS_NOT_ENOUGH_MEMORY;
            break;

        default:
            ret = VOLC_STATUS_INVALID_OPERATION;
        }
    }

err_out_label:
    free(newPath);
    return ret;
}

uint32_t volc_file_exists(const char *path, bool *p_exists)
{
    volc_debug("%s, %s\n", __FUNCTION__, path);
    char *newPath = replacePrefix(path, "./", CONFIG_ROOT_PATH);
    normalize_path(newPath);
    volc_debug("new path: %s\n", newPath);

    if (path == NULL || p_exists == NULL) {
        return VOLC_STATUS_NULL_ARG;
    }
    int path_len = strlen(newPath);
    char *p = strrchr(newPath, '/');
    if (p == NULL) {
        return -1;
    }
    int len = p - newPath;
    volc_debug("path_len:%d, len:%d", path_len, len);
    if ((path_len - 1) == len) {
        int exist = fdir_exist(newPath);
        if (exist) {
            *p_exists = 1;
            volc_debug("fdir exist");
        } else {
            *p_exists = 0;
            volc_debug("fdir not exist");
        }
    } else {
        FILE *fp = fopen(newPath, "r");
        if (fp) {
            *p_exists = 1;
            volc_debug("file exist");
            fclose(fp);
        } else {
            *p_exists = 0;
            volc_debug("file not exist");
        }
    }

//    if (path == NULL || p_exists == NULL) {
//        return VOLC_STATUS_NULL_ARG;
//    }
//
//    struct stat st;
//    int32_t result = stat(path, &st);
//    *p_exists = (result == 0);
    free(newPath);
    return VOLC_STATUS_SUCCESS;
}

uint32_t volc_file_create(const char *path, uint64_t size)
{
    volc_debug("%s, %s\n", __FUNCTION__, path);
    char *newPath = replacePrefix(path, "./", CONFIG_ROOT_PATH);
    normalize_path(newPath);
    volc_debug("new path: %s\n", newPath);
    uint32_t ret = VOLC_STATUS_SUCCESS;
    FILE *fp = NULL;
    char path_file[125];
    VOLC_CHK(path != NULL, VOLC_STATUS_NULL_ARG);
    long_file_name_encode(newPath, path_file, sizeof(path_file));

    fp = fopen(path_file, "wb+");
    VOLC_CHK(fp != NULL, VOLC_STATUS_OPEN_FILE_FAILED);

    if (size != 0) {
        VOLC_CHK(0 == fseek(fp, (uint32_t)(size - 1), SEEK_SET), VOLC_STATUS_INVALID_OPERATION);
        VOLC_CHK(0 == fputc(0, fp), VOLC_STATUS_INVALID_OPERATION);
    }

err_out_label:

    if (fp != NULL) {
        fclose(fp);
        fp = NULL;
    }
    free(newPath);
    return ret;
}

uint32_t volc_file_delete(const char *path)
{
    volc_debug("%s, %s\n", __FUNCTION__, path);
    char *newPath = replacePrefix(path, "./", CONFIG_ROOT_PATH);
    normalize_path(newPath);
    volc_debug("new path: %s\n", newPath);
    uint32_t ret = VOLC_STATUS_SUCCESS;
//    unlink(path);
    fdelete(newPath);
err_out_label:
    free(newPath);
    return ret;
}
