#ifndef _DUER_JSON_REQUEST_H_
#define _DUER_JSON_REQUEST_H_

#include "duer_common.h"
#include "my_platform_common.h"


extern char *duer_start_frame(const char *appid, const char *appkey, const char *client_id, const char *pid,
                              const char *cuid, const char *format, int support_dcs, int sample,
                              const char *user_id, const char *access_token, int support_tts,
                              int support_text2dcs, const char *dialog_request_id, int access_rc,
                              const char *rc_version);

extern char *build_finish_frame() ;


extern char *duer_text_info_frame(const char *CUID,
                                  const char *current_dialog_request_id,
                                  const char *text_query);

#endif
