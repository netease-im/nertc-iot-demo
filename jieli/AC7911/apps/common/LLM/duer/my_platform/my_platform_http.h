#ifndef  _MY_PLATFORM_HTTP_H_
#define  _MY_PLATFORM_HTTP_H_

#include "http/http_cli.h"
#include  "system/includes.h"
#include "my_platform_common.h"

extern int my_http_get_request(char *url, char **response);
extern int my_http_post_request(char *url, char **response);
extern int my_http_get_url_data_size(char *url);


#endif

