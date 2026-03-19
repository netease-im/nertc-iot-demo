#include "fs/fs.h"
#define __FILE_defined

#include "tkc/types_def.h"
#include "tkc/fs.h"
#include "tkc/mem.h"
#include "tkc/utils.h"


static int32_t fs_os_file_read(fs_file_t *file, void *buffer, uint32_t size)    //printf(" %s \r\n",__FUNCTION__);
{
    FILE *fp = (FILE *)(file->data);

    return (int32_t)fread(buffer, 1, size, fp);
}

static int32_t fs_os_file_write(fs_file_t *file, const void *buffer, uint32_t size)
{
    /*printf(" %s \r\n", __FUNCTION__);*/
    FILE *fp = (FILE *)(file->data);

    return fwrite(buffer, 1, size, fp);
}

static int32_t fs_os_file_printf(fs_file_t *file, const char *const format_str, va_list vl)
{
    assert(0);
//  FILE* fp = (FILE*)(file->data);
//  return vfprintf(fp, format_str, vl);
}

static ret_t fs_os_file_seek(fs_file_t *file, int32_t offset)    //printf(" %s \r\n",__FUNCTION__);
{
    FILE *fp = (FILE *)(file->data);

    return fseek(fp, offset, SEEK_SET) == 0 ? RET_OK : RET_FAIL;
}

static int64_t fs_os_file_tell(fs_file_t *file)
{
    printf(" %s \r\n", __FUNCTION__);
    FILE *fp = (FILE *)(file->data);

    return ftell(fp);
}

static int64_t fs_os_file_size(fs_file_t *file)
{
    printf(" %s \r\n", __FUNCTION__);

    FILE *fp = (FILE *)(file->data);

    struct vfs_attr fattr;
    return_value_if_fail(fget_attrs(fp, &fattr) == 0, -1);

    return fattr.fsize;
}

static ret_t fs_os_file_stat(fs_file_t *file, fs_stat_info_t *fst)
{
    printf(" %s \r\n", __FUNCTION__);
    FILE *fp = (FILE *)(file->data);

    struct vfs_attr fattr;
    return_value_if_fail(fget_attrs(fp, &fattr) == 0, RET_FAIL);

    memset(fst, 0, sizeof(*fst));

    fst->size = fattr.fsize;
    fst->is_dir = fattr.attr & F_ATTR_DIR;
    fst->is_reg_file = fattr.attr & F_ATTR_ARC;

    return RET_OK;
}

static ret_t fs_os_file_sync(fs_file_t *file)
{
    (void)file;
    return f_flush_wbuf(CONFIG_ROOT_PATH) == 0 ? RET_OK : RET_FAIL;
}

static ret_t fs_os_file_truncate(fs_file_t *file, int32_t size)
{
    FILE *fp = (FILE *)(file->data);
    assert(0);
//  return ftruncate(fileno(fp), size) == 0 ? RET_OK : RET_FAIL;
}

static bool_t fs_os_file_eof(fs_file_t *file)
{
    FILE *fp = (FILE *)(file->data);
    assert(0);
//  return feof(fp) != 0;
}

static ret_t fs_os_file_close(fs_file_t *file)
{
    FILE *fp = (FILE *)(file->data);
    fclose(fp);
    TKMEM_FREE(file);

    return RET_OK;
}

static const fs_file_vtable_t s_file_vtable = {.read = fs_os_file_read,
                                               .write = fs_os_file_write,
                                               .printf = fs_os_file_printf,
                                               .seek = fs_os_file_seek,
                                               .tell = fs_os_file_tell,
                                               .size = fs_os_file_size,
                                               .stat = fs_os_file_stat,
                                               .sync = fs_os_file_sync,
                                               .truncate = fs_os_file_truncate,
                                               .eof = fs_os_file_eof,
                                               .close = fs_os_file_close
                                              };

static const fs_dir_vtable_t s_dir_vtable = {.rewind = NULL,
                                             .read = NULL,
                                             .close = NULL,
                                            };

static fs_file_t *fs_os_open_file(fs_t *fs, const char *name, const char *mode)
{
    (void)fs;
    return_value_if_fail(name != NULL && mode != NULL, NULL);
#ifdef CONFIG_JLFAT_ENABLE
    char path[128];
    path[long_file_name_encode(name, (u8 *)path, sizeof(path))] = '\0';
    FILE *file = fopen(path, mode);
#else
    FILE *file = fopen(name, mode);
#endif
    if (file == NULL) {
        printf("%s, open [%s][%s] fail! \r\n", __FUNCTION__, name, mode);
        return NULL;
    }

    fs_file_t *f = TKMEM_ALLOC(sizeof(fs_file_t));
    if (f != NULL) {
        f->vt = &s_file_vtable;
        f->data = file;
    } else {
        fclose(file);
    }
    return f;
}

static ret_t fs_os_remove_file(fs_t *fs, const char *name)
{
    assert(0);
}

static bool_t fs_os_file_exist(fs_t *fs, const char *name)
{
    return_value_if_fail(name != NULL, FALSE);
#ifdef CONFIG_JLFAT_ENABLE
    char path[128];
    path[long_file_name_encode(name, (u8 *)path, sizeof(path))] = '\0';
    FILE  *file = fopen(path, "r");
#else
    FILE  *file = fopen(name, "r");
#endif
    if (file) {
        fclose(file);
        return TRUE;
    }
    /*printf("%s, open [%s] fail! \r\n", __FUNCTION__, name);*/
    return FALSE;
}

static ret_t fs_os_file_rename(fs_t *fs, const char *name, const char *new_name)
{
    assert(0);
}

static fs_dir_t *fs_os_open_dir(fs_t *fs, const char *name)
{
    assert(0);
    (void)fs;
    return_value_if_fail(name != NULL, NULL);
#ifdef CONFIG_JLFAT_ENABLE
    char path[128];
    path[long_file_name_encode(name, (u8 *)path, sizeof(path))] = '\0';
    FILE *file = fopen(path, "r");
#else
    FILE *file = fopen(name, "r");
#endif
    if (file == NULL) {
        printf("%s, open [%s][%s] fail! \r\n", __FUNCTION__, name);
        return NULL;
    }

    fs_dir_t *dir = TKMEM_ALLOC(sizeof(fs_dir_t));
    if (dir != NULL) {
        dir->vt = &s_dir_vtable;
        dir->data = file;
    } else {
        fclose(file);
    }
    return dir;
}

static ret_t fs_os_remove_dir(fs_t *fs, const char *name)
{
    assert(0);
}

static ret_t fs_os_change_dir(fs_t *fs, const char *name)
{
    assert(0);
}

static ret_t fs_os_create_dir(fs_t *fs, const char *name)
{
    //只写了创建根目录下的目录
    return fmk_dir(CONFIG_ROOT_PATH, strrchr(name, '/'), 0) == 0 ? RET_OK : RET_FAIL;;
}

static bool_t fs_os_dir_exist(fs_t *fs, const char *name)
{
    return_value_if_fail(name != NULL, FALSE);
#ifdef CONFIG_JLFAT_ENABLE
    char path[128];
    path[long_file_name_encode(name, (u8 *)path, sizeof(path))] = '\0';
    FILE  *file = fopen(path, "r");
#else
    FILE  *file = fopen(name, "r");
#endif

    if (file) {
        fclose(file);
        return TRUE;
    }
    printf("%s, open [%s] fail! \r\n", __FUNCTION__, name);
    return FALSE;
}

static ret_t fs_os_dir_rename(fs_t *fs, const char *name, const char *new_name)
{
    assert(0);
}

static int32_t fs_os_get_file_size(fs_t *fs, const char *name)  //printf("%s, ,%s\r\n",__FUNCTION__,name);
{
    return_value_if_fail(fs && name, -1);
#ifdef CONFIG_JLFAT_ENABLE
    char path[128];
    path[long_file_name_encode(name, (u8 *)path, sizeof(path))] = '\0';
    FILE  *file = fopen(path, "r");
#else
    FILE  *file = fopen(name, "r");
#endif
    return_value_if_fail(file, -1);

    struct vfs_attr fattr;
    return_value_if_fail(fget_attrs(file, &fattr) == 0, -1);
    fclose(file);

    return fattr.fsize;
}

static ret_t fs_os_get_disk_info(fs_t *fs, const char *volume, int32_t *free_kb,
                                 int32_t *total_kb)
{
    assert(0);
    (void)fs;
    return_value_if_fail(free_kb && total_kb, RET_BAD_PARAMS);
    return_value_if_fail(!"fs_os_get_disk_info not supported yet", RET_BAD_PARAMS);

    *free_kb = 0;
    *total_kb = 0;
    return RET_NOT_IMPL;
}
static ret_t fs_os_get_cwd(fs_t *fs, char path[MAX_PATH + 1])
{
    (void)fs;
    tk_strcpy(path, CONFIG_ROOT_PATH);
    return RET_OK;
}

static ret_t fs_os_get_exe(fs_t *fs, char path[MAX_PATH + 1])  // printf("%s, ,%s\r\n",__FUNCTION__,path);
{
    (void)fs;
    tk_strcpy(path, CONFIG_ROOT_PATH);
    return RET_OK;
}
static ret_t fs_os_get_user_storage_path(fs_t *fs, char path[MAX_PATH + 1])  // printf("%s, ,%s\r\n",__FUNCTION__,path);
{
    (void)fs;
    tk_strcpy(path, CONFIG_ROOT_PATH);
    return RET_OK;
}
static ret_t fs_os_get_temp_path(fs_t *fs, char path[MAX_PATH + 1])  // printf("%s, ,%s\r\n",__FUNCTION__,path);
{
    (void)fs;
    tk_strcpy(path, CONFIG_ROOT_PATH);
    return RET_OK;
}
static ret_t fs_os_stat(fs_t *fs, const char *name, fs_stat_info_t *fst)
{
    /* printf("%s, ,%s\r\n", __FUNCTION__, name); */

    return_value_if_fail(fs && fst && name, RET_FAIL);

#ifdef CONFIG_JLFAT_ENABLE
    char path[128];
    path[long_file_name_encode(name, (u8 *)path, sizeof(path))] = '\0';
    FILE  *file = fopen(path, "r");
#else
    FILE  *file = fopen(name, "r");
#endif

    return_value_if_fail(file, RET_FAIL);

    struct vfs_attr fattr;
    return_value_if_fail(fget_attrs(file, &fattr) == 0, RET_FAIL);

    memset(fst, 0, sizeof(*fst));

    fst->size = fattr.fsize;
    fst->is_dir = fattr.attr & F_ATTR_DIR;
    fst->is_reg_file = fattr.attr & F_ATTR_ARC;
    fclose(file);

    return RET_OK;
}

static const fs_t s_os_fs = {.open_file = fs_os_open_file,
                             .remove_file = fs_os_remove_file,
                             .file_exist = fs_os_file_exist,
                             .file_rename = fs_os_file_rename,

                             .open_dir = fs_os_open_dir,
                             .remove_dir = fs_os_remove_dir,
                             .create_dir = fs_os_create_dir,
                             .change_dir = fs_os_change_dir,
                             .dir_exist = fs_os_dir_exist,
                             .dir_rename = fs_os_dir_rename,

                             .get_file_size = fs_os_get_file_size,
                             .get_disk_info = fs_os_get_disk_info,
                             .get_cwd = fs_os_get_cwd,
                             .get_exe = fs_os_get_exe,
                             .get_user_storage_path = fs_os_get_user_storage_path,
                             .get_temp_path = fs_os_get_temp_path,
                             .stat = fs_os_stat
                            };

fs_t *os_fs(void)
{
    return (fs_t *)&s_os_fs;
}

