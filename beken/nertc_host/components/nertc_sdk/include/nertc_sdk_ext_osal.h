#ifndef __NERTC_SDK_EXT_OSAL_H__
#define __NERTC_SDK_EXT_OSAL_H__

#include <stdbool.h>
#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif

typedef void* nertc_ext_thread_handle_t;
typedef void* nertc_ext_timer_handle_t;
typedef void* nertc_ext_mutex_handle_t;
typedef void* nertc_ext_cond_handle_t;

typedef void (*nertc_ext_thread_entry_func)(void* user_data);
typedef void (*nertc_ext_timer_callback_func)(void* user_data);

typedef nertc_ext_thread_handle_t (*nertc_ext_thread_create_func)(const char* name,
                                                                  int stack_size,
                                                                  int priority,
                                                                  bool prefer_external_memory,
                                                                  nertc_ext_thread_entry_func entry,
                                                                  void* user_data);
typedef void (*nertc_ext_thread_destroy_func)(nertc_ext_thread_handle_t handle);
typedef bool (*nertc_ext_thread_is_current_func)(nertc_ext_thread_handle_t handle);
typedef void (*nertc_ext_thread_wait_func)(nertc_ext_thread_handle_t handle, uint32_t timeout_ms);
typedef void (*nertc_ext_thread_notify_func)(nertc_ext_thread_handle_t handle);

typedef nertc_ext_timer_handle_t (*nertc_ext_timer_create_func)(const char* name,
                                                                int interval_ms,
                                                                nertc_ext_timer_callback_func callback,
                                                                void* user_data);
typedef bool (*nertc_ext_timer_start_func)(nertc_ext_timer_handle_t handle);
typedef bool (*nertc_ext_timer_stop_func)(nertc_ext_timer_handle_t handle);
typedef void (*nertc_ext_timer_destroy_func)(nertc_ext_timer_handle_t handle);

typedef void (*nertc_ext_sleep_ms_func)(int ms);
typedef int64_t (*nertc_ext_get_timestamp_ms_func)(void);
typedef void (*nertc_ext_log_write_func)(int level, const char* tag, const char* message);

typedef nertc_ext_mutex_handle_t (*nertc_ext_mutex_create_func)(void);
typedef void (*nertc_ext_mutex_destroy_func)(nertc_ext_mutex_handle_t handle);
typedef bool (*nertc_ext_mutex_lock_func)(nertc_ext_mutex_handle_t handle);
typedef void (*nertc_ext_mutex_unlock_func)(nertc_ext_mutex_handle_t handle);

typedef nertc_ext_cond_handle_t (*nertc_ext_cond_create_func)(void);
typedef void (*nertc_ext_cond_destroy_func)(nertc_ext_cond_handle_t handle);
typedef void (*nertc_ext_cond_wait_func)(nertc_ext_cond_handle_t cond_handle,
                                         nertc_ext_mutex_handle_t mutex_handle,
                                         uint32_t timeout_ms);
typedef void (*nertc_ext_cond_notify_one_func)(nertc_ext_cond_handle_t handle);
typedef void (*nertc_ext_cond_notify_all_func)(nertc_ext_cond_handle_t handle);

typedef struct {
  nertc_ext_thread_create_func create_thread;
  nertc_ext_thread_destroy_func destroy_thread;
  nertc_ext_thread_is_current_func thread_is_current;
  nertc_ext_thread_wait_func thread_wait;
  nertc_ext_thread_notify_func thread_notify;

  nertc_ext_timer_create_func create_timer;
  nertc_ext_timer_start_func start_timer;
  nertc_ext_timer_stop_func stop_timer;
  nertc_ext_timer_destroy_func destroy_timer;

  nertc_ext_sleep_ms_func sleep_ms;
  nertc_ext_get_timestamp_ms_func get_timestamp_ms;
  nertc_ext_log_write_func log_write;

  nertc_ext_mutex_create_func create_mutex;
  nertc_ext_mutex_destroy_func destroy_mutex;
  nertc_ext_mutex_lock_func lock_mutex;
  nertc_ext_mutex_unlock_func unlock_mutex;

  nertc_ext_cond_create_func create_cond;
  nertc_ext_cond_destroy_func destroy_cond;
  nertc_ext_cond_wait_func wait_cond;
  nertc_ext_cond_notify_one_func notify_one_cond;
  nertc_ext_cond_notify_all_func notify_all_cond;
} nertc_sdk_ext_osal_handle_t;

#ifdef __cplusplus
}
#endif

#endif  // __NERTC_SDK_EXT_OSAL_H__
